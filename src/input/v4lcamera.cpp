#include "v4lcamera.h"
#include "common/log.h"
#include "common/common.h"
#include "common/config.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <setjmp.h>

namespace input
{

static void YUYV2RGB(int width, int height, unsigned char *yuyv_image, unsigned char *rgb_image)
{
	int y;
	int cr;
	int cb;

	double r;
	double g;
	double b;

	for (int i = 0, j = 0; i < width * height * 3; i+=6, j+=4) {
		//first pixel
		y = yuyv_image[j];
		cb = yuyv_image[j+1];
		cr = yuyv_image[j+3];

		r = y + (1.4065 * (cr - 128));
		g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
		b = y + (1.7790 * (cb - 128));

		//This prevents colour distortions in your rgb image
		if (r < 0) r = 0;
		else if (r > 255) r = 255;
		if (g < 0) g = 0;
		else if (g > 255) g = 255;
		if (b < 0) b = 0;
		else if (b > 255) b = 255;

		rgb_image[i] = (unsigned char)r;
		rgb_image[i+1] = (unsigned char)g;
		rgb_image[i+2] = (unsigned char)b;

		//second pixel
		y = yuyv_image[j+2];
		cb = yuyv_image[j+1];
		cr = yuyv_image[j+3];

		r = y + (1.4065 * (cr - 128));
		g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
		b = y + (1.7790 * (cb - 128));

		if (r < 0) r = 0;
		else if (r > 255) r = 255;
		if (g < 0) g = 0;
		else if (g > 255) g = 255;
		if (b < 0) b = 0;
		else if (b > 255) b = 255;

		rgb_image[i+3] = (unsigned char)r;
		rgb_image[i+4] = (unsigned char)g;
		rgb_image[i+5] = (unsigned char)b;
	}
}

static int xioctl(int fd, int request, void *arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

V4lCamera::V4lCamera()
	:_active(false)
	,_fd(-1)
	,_buffers(nullptr)
	,_bufferCount(4)
	,_buftype(V4L2_BUF_TYPE_VIDEO_CAPTURE)
	,_width(Config::instance()->cameraWidth())
	,_height(Config::instance()->cameraHeight())
	,_imageQuality(50)
	,_brightness(50)
	,_format(V4L2_PIX_FMT_YUYV)
	,_outformat(RgbFormat)
{
}

V4lCamera::~V4lCamera()
{
	close();
}

bool V4lCamera::open()
{
	if (_active)
		close();

	int dev = Config::instance()->cameraDev();
	if (dev < 0)
	{
		dev = getVideoDevice();
		if (dev < 0)
		{
			LOGE("No camera device!");
			return false;
		}
		else
		{
			LOG("Found video device #" << dev);
		}
	}

	char devPath[50];
	sprintf(devPath, "/dev/video%d", dev);

	return open(devPath);
}

void V4lCamera::close()
{
	_active = false;
	uninit();
}

bool V4lCamera::open(const string &dev)
{
	if (_active)
		close();

	_active = false;
	_fd = ::open(dev.c_str(), O_RDWR /*| O_NONBLOCK*/);
	if (_fd == -1)
	{
		perror("Open video device");
		return false;
	}

	if (!init())
		return false;

	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(req));

	req.count = _bufferCount;
	req.type = _buftype;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(_fd, VIDIOC_REQBUFS, &req))
	{
		perror("VIDIOC_REQBUFS");
		return false;
	}

	if (req.count < 2)
	{
		perror("Insufficient buffer memory");
		return false;
	}

	_buffers = (buffer*)calloc(req.count, sizeof(*_buffers));
	if (!_buffers)
	{
		perror("Out of memory");
		return false;
	}

	for (_bufferCount = 0; _bufferCount < req.count; ++_bufferCount)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));

		struct v4l2_plane planes[1];
		memset(&planes, 0, sizeof(planes));

		buf.type = _buftype;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = _bufferCount;

		if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buftype)
		{
			buf.m.planes = planes;
			buf.length = 1;
		}

		if (-1 == xioctl(_fd, VIDIOC_QUERYBUF, &buf))
		{
			perror("VIDIOC_QUERYBUF");
			return false;
		}

		if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buftype)
		{
			_buffers[_bufferCount].length = buf.m.planes[0].length;
			_buffers[_bufferCount].start = mmap(NULL, buf.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, buf.m.planes[0].m.mem_offset);
		}
		else
		{
			_buffers[_bufferCount].length = buf.length;
			_buffers[_bufferCount].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, buf.m.offset);
		}

		if (MAP_FAILED == _buffers[_bufferCount].start)
		{
			LOGE("MAP_FAILED");
			return false;
		}
	}

	for (unsigned int i = 0; i < _bufferCount; ++i)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));

		buf.type = _buftype;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buftype)
		{
			struct v4l2_plane planes[1];
			buf.m.planes = planes;
			buf.length = 1;
		}

		if (-1 == xioctl(_fd, VIDIOC_QBUF, &buf))
		{
			perror("VIDIOC_QBUF");
			return false;
		}
	}

	enum v4l2_buf_type type = _buftype;
	if (-1 == xioctl(_fd, VIDIOC_STREAMON, &type))
	{
		perror("VIDIOC_STREAMON");
		return false;
	}

	_active = true;
	return true;
}

bool V4lCamera::init()
{
	struct v4l2_capability cap;
	memset(&cap, 0, sizeof(cap));

	if (-1 == xioctl(_fd, VIDIOC_QUERYCAP, &cap))
	{
		perror("VIDIOC_QUERYCAP");
		return false;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
			!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
		perror("Device doesn't support video capture!");
		return false;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		perror("Device doesn't support streaming!");
		return false;
	}

	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
		_buftype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		_buftype = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	struct v4l2_cropcap cropcap;
	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == xioctl (_fd, VIDIOC_CROPCAP, &cropcap))
	{
		struct v4l2_crop crop;
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect;

		if (-1 == xioctl(_fd, VIDIOC_S_CROP, &crop))
			perror("VIDIOC_S_CROP");
	}

	// Get pixel format
	memset(&_fmt, 0, sizeof(_fmt));
	_fmt.type = _buftype;

	if (-1 == xioctl(_fd, VIDIOC_G_FMT, &_fmt))
	{
		perror("VIDIOC_G_FMT");
		return false;
	}

	// Check width
	if (_width > _fmt.fmt.pix.width)
	{
		LOGW("Wrong image width: " << _width);
		//_width = _fmt.fmt.pix.width;
	}

	// Check height
	if (_height > _fmt.fmt.pix.height)
	{
		LOGW("Wrong image height: " << _height);
		//_height = _fmt.fmt.pix.height;
	}

	// Set pixel format
	memset(&_fmt, 0, sizeof(_fmt));
	_fmt.type = _buftype;;
	_fmt.fmt.pix.width = _width;
	_fmt.fmt.pix.height = _height;
	_fmt.fmt.pix.pixelformat = _format;
	_fmt.fmt.pix.field = V4L2_FIELD_ANY;

	if (-1 == xioctl(_fd, VIDIOC_S_FMT, &_fmt))
	{
		perror("VIDIOC_S_FMT");
		return false;
	}

	return true;
}

void V4lCamera::uninit()
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(_fd, VIDIOC_STREAMOFF, &type))
	{
		perror("VIDIOC_STREAMOFF");
	}

	for (unsigned int i = 0; i < _bufferCount; ++i)
	{
		if (-1 == munmap(_buffers[i].start, _buffers[i].length))
		{
			perror("munmap");
		}
	}

	if (-1 == ::close(_fd))
	{
		perror("close");
	}
}

bool V4lCamera::read(MatPtr &frame)
{
	int count = 5;
	while (count-- > 0)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(_fd, &fds);

		struct timeval tv;
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		int r = select(_fd + 1, &fds, NULL, NULL, &tv);
		if (r == -1)
		{
			if (errno == EINTR)
				continue;

			perror("Select");
			return false;
		}

		if (r == 0)
		{
			perror("Select timeout");
			return false;
		}

		int error = getFrame(frame);
		if (error == 0)
			break;
		else if (error == EAGAIN)
			continue;
		else
			return false;
	}

	return true;
}

int V4lCamera::getFrame(MatPtr &frame)
{
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));

	buf.type = _buftype;
	buf.memory = V4L2_MEMORY_MMAP;

	if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buftype)
	{
		struct v4l2_plane planes[1];
		buf.m.planes = planes;
		buf.length = 1;
	}

	// Dequeue buffer
	if (-1 == xioctl(_fd, VIDIOC_DQBUF, &buf))
	{
		perror("VIDIOC_DQBUF");
		return errno;
	}

	unsigned int bytesused = 0;
	if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buftype)
		bytesused = buf.m.planes[0].bytesused;
	else
		bytesused = buf.bytesused;

	// Format data
	int imgsize = _width * _height * 3;
	uint8_t *imageData = (uint8_t *)malloc(imgsize);
	YUYV2RGB(_width, _height, (uint8_t *)_buffers[buf.index].start, imageData);

	cv::Mat data(_height, _width, CV_8UC3, imageData);
	data.assignTo(*frame);
	free(imageData);

	// Queue buffer
	if (-1 == xioctl(_fd, VIDIOC_QBUF, &buf))
	{
		perror("VIDIOC_QBUF");
		return errno;
	}

	return 0;
}

}
