#include "gstcamera.h"
#include "common/log.h"
#include "common/config.h"

namespace input
{

GstCamera::GstCamera()
	: _active(false)
{
}

GstCamera::~GstCamera()
{
}

bool GstCamera::open()
{
	string gstString = "v4l2src device=/dev/video";
	gstString += std::to_string(GetConfig()->cameraDev());
	gstString += " ! video/x-raw, format=" + GetConfig()->cameraFormat();
	gstString += ",width=" + std::to_string(GetConfig()->cameraWidth());
	gstString += ",height=" + std::to_string(GetConfig()->cameraHeight());
	gstString += ",framerate=" + std::to_string(GetConfig()->cameraFps()) + "/1";
	gstString += " ! videoconvert ! video/x-raw,format=BGR ! appsink drop=1";

	LOG("[GstCamera] " << gstString);

	if (!_capture.open(gstString, cv::CAP_GSTREAMER))
	{
		LOGE("Can't open capture device!");
		return false;
	}

	// _capture.set(cv::CAP_PROP_FPS, GetConfig()->cameraFps());
	// _capture.set(cv::CAP_PROP_FRAME_WIDTH, GetConfig()->cameraWidth());
	// _capture.set(cv::CAP_PROP_FRAME_HEIGHT, GetConfig()->cameraHeight());

	_active = true;
	return true;
}

void GstCamera::close()
{
	_active = false;
	_capture.release();
}

bool GstCamera::read(MatPtr &frame)
{
	if (_capture.read(*frame))
		return true;
	return false;
}

}
