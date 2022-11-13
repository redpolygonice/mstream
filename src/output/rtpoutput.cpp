#include "rtpoutput.h"
#include "common/log.h"
#include "common/config.h"

RtpOutput::RtpOutput()
	: _active(false)
{
}

RtpOutput::~RtpOutput()
{
}

bool RtpOutput::open()
{
	string	gstString = "appsrc ! videoconvert ! videoscale ! video/x-raw";
	gstString += ",width=" + std::to_string(Config::instance()->outputWidth());
	gstString += ",height=" + std::to_string(Config::instance()->outputHeight());
	gstString += ",framerate=" + std::to_string(Config::instance()->outputFps()) + "/1";
	if (Config::instance()->procType() == ProcType::Unknown)
		gstString += " ! x264enc ! rtph264pay ! queue ! udpsink";
	else if (Config::instance()->procType() == ProcType::Rk)
		gstString += " ! mpph264enc ! rtph264pay ! queue ! udpsink";
	gstString += " host=" + Config::instance()->rtpHost();
	gstString += " port=" + std::to_string(Config::instance()->rtpPort());
	gstString += " sync=false";

	if (!_writer.open(gstString, 0, Config::instance()->outputFps(),  cv::Size(Config::instance()->outputWidth(), Config::instance()->outputHeight())))
	{
		LOGE("Can't open writer!");
		return false;
	}

	_active = true;
	return true;
}

void RtpOutput::close()
{
	_active = false;
	_writer.release();
}

bool RtpOutput::write(const MatPtr &frame)
{
	_writer.write(*frame);
	return true;
}
