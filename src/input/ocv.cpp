#include "ocv.h"
#include "common/log.h"
#include "common/config.h"

namespace input
{

Ocv::Ocv()
	: _active(false)
{
}

Ocv::~Ocv()
{
}

bool Ocv::open()
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

void Ocv::close()
{
	_active = false;
	_capture.release();
}

bool Ocv::read(MatPtr &frame)
{
	if (_capture.read(*frame))
		return true;
	return false;
}

}
