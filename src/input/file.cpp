#include "file.h"
#include "common/log.h"
#include "common/config.h"
#include "common/common.h"

namespace input
{

File::File()
	: _active(false)
	, _width(0)
	, _height(0)
	, _channels(0)
	, _type(CV_8UC3)
{
}

File::~File()
{
}

bool File::open()
{
	string fileName = GetConfig()->inputFilePath();
	if (!isFileExists(fileName))
	{
		LOGE("File: " << fileName << " doesn't exist!");
		return false;
	}

	if (!_reader.open(fileName))
	{
		LOGE("Can't open " << fileName);
		return false;
	}

	_width = _reader.get(cv::CAP_PROP_FRAME_WIDTH);
	_height = _reader.get(cv::CAP_PROP_FRAME_HEIGHT);
	_active = true;
	return true;
}

void File::close()
{
	_active = false;
	_reader.release();
}

bool File::read(MatPtr &frame)
{
	if (_reader.read(*frame))
	{
		if (frame->type() == CV_8UC4)
			cv::cvtColor(*frame, *frame, cv::COLOR_BGRA2BGR, 3);
		return true;
	}

	return false;
}

bool File::read()
{
	cv::Mat frame;
	_reader.read(frame);
	return true;
}

}
