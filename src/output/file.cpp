#include "file.h"
#include "common/log.h"
#include "common/config.h"

namespace output
{

File::File()
	: _active(false)
{
}

File::~File()
{
}

bool File::open()
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

void File::close()
{
	_active = false;
	_writer.close();
}

bool File::write(const MatPtr &frame)
{
	_writer.writeFrame(frame->data);
	return false;
}

}
