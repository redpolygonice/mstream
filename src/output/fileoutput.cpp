#include "fileoutput.h"
#include "common/log.h"
#include "common/config.h"

FileOutput::FileOutput()
	: _active(false)
{
}

FileOutput::~FileOutput()
{
}

bool FileOutput::open()
{
	string fileName = Config::instance()->outputFilePath();
	_writer.setWidth(Config::instance()->outputWidth());
	_writer.setHeight(Config::instance()->outputHeight());
	_writer.setFps(Config::instance()->outputFps());
	_writer.setBitrate(Config::instance()->outputBitrate());
	_writer.setGop(Config::instance()->outputGop());
	_writer.setCodec(Config::instance()->outputFileCodec());

	if (!_writer.open(fileName))
	{
		LOGE("Can't open AvWriter!");
		return false;
	}

	_active = true;
	return true;
}

void FileOutput::close()
{
	_active = false;
	_writer.close();
}

bool FileOutput::write(const MatPtr &frame)
{
	_writer.writeFrame(frame->data);
	return false;
}
