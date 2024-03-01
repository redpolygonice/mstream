#include "rtp.h"
#include "common/log.h"
#include "common/config.h"

namespace output
{

Rtp::Rtp()
	: _active(false)
{
}

Rtp::~Rtp()
{
}

bool Rtp::open()
{
	string	gstString = "appsrc ! videoconvert ! videoscale ! video/x-raw";
	gstString += ",width=" + std::to_string(GetConfig()->outputWidth());
	gstString += ",height=" + std::to_string(GetConfig()->outputHeight());
	gstString += ",framerate=" + std::to_string(GetConfig()->outputFps()) + "/1";
	if (GetConfig()->procType() == ProcType::Rpi3)
		gstString += " ! omxh264enc control-rate=2 target-bitrate=2000000";
	else if (GetConfig()->procType() == ProcType::Rk)
		gstString += " ! mpph264enc bps=2000000";
	else
		gstString += " ! x264enc speed-preset=superfast tune=zerolatency";
	gstString += " ! rtph264pay ! queue ! udpsink";
	gstString += " host=" + GetConfig()->rtpHost();
	gstString += " port=" + std::to_string(GetConfig()->rtpPort());
	gstString += " sync=false";

	if (!_writer.open(gstString, 0, GetConfig()->outputFps(),  cv::Size(GetConfig()->outputWidth(), GetConfig()->outputHeight())))
	{
		LOGE("Can't open writer!");
		return false;
	}

	_active = true;
	return true;
}

void Rtp::close()
{
	_active = false;
	_writer.release();
}

bool Rtp::write(const MatPtr &frame)
{
	_writer.write(*frame);
	return true;
}

}
