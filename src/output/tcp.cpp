#include "tcp.h"
#include "common/log.h"
#include "common/config.h"

namespace output
{

Tcp::Tcp()
	: _active(false)
{
}

Tcp::~Tcp()
{
}

bool Tcp::open()
{
	string	gstString = "appsrc ! videoconvert ! videoscale ! video/x-raw,format=BGR";
	gstString += ",width=" + std::to_string(GetConfig()->outputWidth());
	gstString += ",height=" + std::to_string(GetConfig()->outputHeight());
	gstString += ",framerate=" + std::to_string(GetConfig()->outputFps()) + "/1";
	gstString += " ! queue ! jpegenc";
	gstString += " ! tcpserversink";
	gstString += " host=0.0.0.0";
	gstString += " port=5000";

	LOG("[Tcp] " << gstString);

	if (!_writer.open(gstString, 0, GetConfig()->outputFps(),  cv::Size(GetConfig()->outputWidth(), GetConfig()->outputHeight())))
	{
		LOGE("Can't open writer!");
		return false;
	}

	_active = true;
	return true;
}

void Tcp::close()
{
	_active = false;
	_writer.release();
}

bool Tcp::write(const MatPtr &frame)
{
	_writer.write(*frame);
	return true;
}

}
