#include "fileinput.h"
#include "common/log.h"
#include "common/config.h"
#include "common/common.h"

FileInput::FileInput()
	: _active(false)
	, _width(0)
	, _height(0)
	, _channels(0)
	, _type(CV_8UC3)
{
}

FileInput::~FileInput()
{
}

bool FileInput::open()
{
	string fileName = Config::instance()->inputFilePath();
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
	return true;
}

void FileInput::close()
{
	_active = false;
	_reader.release();
}

bool FileInput::read(MatPtr &frame)
{
	if (_reader.read(*frame))
	{
		//LOGD("Frame type: " << frame->type());
		if (frame->type() == CV_8UC4)
			cv::cvtColor(*frame, *frame, cv::COLOR_BGRA2BGR, 3);
		return true;
	}

	return false;
}
