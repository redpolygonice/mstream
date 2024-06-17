#include "libcamera.h"
#include "common/log.h"
#include "common/config.h"

namespace input
{

LibCamera::LibCamera()
	: _active(false)
{
}

LibCamera::~LibCamera()
{
}

bool LibCamera::open()
{
	string gstString = "libcamerasrc";
	gstString += " ! video/x-raw,format=" + GetConfig()->cameraFormat();
	gstString += ",width=" + std::to_string(GetConfig()->cameraWidth());
	gstString += ",height=" + std::to_string(GetConfig()->cameraHeight());
	gstString += ",framerate=" + std::to_string(GetConfig()->cameraFps()) + "/1";
	gstString += " ! videoconvert ! video/x-raw,format=BGR ! appsink drop=1";

	LOG("[LibCamera] " << gstString);

	if (!_capture.open(gstString, cv::CAP_GSTREAMER))
	{
		LOGE("Can't open capture device!");
		return false;
	}

	_active = true;
	return true;
}

void LibCamera::close()
{
	_active = false;
	_capture.release();
}

bool LibCamera::read(MatPtr &frame)
{
	if (_capture.read(*frame))
		return true;
	return false;
}

}
