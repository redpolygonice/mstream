#include "ocvinput.h"
#include "common/log.h"
#include "common/config.h"

OcvInput::OcvInput()
	: _active(false)
{
}

OcvInput::~OcvInput()
{
}

bool OcvInput::open()
{
	string gstString = "v4l2src device=/dev/video";
	gstString += std::to_string(Config::instance()->cameraDev());
	gstString += " ! video/x-raw, format=" + Config::instance()->cameraFormat();
	gstString += " ! videoconvert ! video/x-raw, format=BGR ! appsink drop=1";

	if (!_capture.open(gstString, cv::CAP_GSTREAMER))
	{
		LOGE("Can't open capture device!");
		return false;
	}

	_capture.set(cv::CAP_PROP_FPS, Config::instance()->cameraFps());
	_capture.set(cv::CAP_PROP_FRAME_WIDTH, Config::instance()->cameraWidth());
	_capture.set(cv::CAP_PROP_FRAME_HEIGHT, Config::instance()->cameraHeight());

	_active = true;
	return true;
}

void OcvInput::close()
{
	_active = false;
	_capture.release();
}

bool OcvInput::read(MatPtr &frame)
{
	if (_capture.read(*frame))
		return true;
	return false;
}
