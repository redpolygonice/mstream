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
	string fileName = GetConfig()->outputFilePath();
	_writer.setWidth(GetConfig()->outputWidth());
	_writer.setHeight(GetConfig()->outputHeight());
	_writer.setFps(GetConfig()->outputFps());
	_writer.setBitrate(GetConfig()->outputBitrate());
	_writer.setGop(GetConfig()->outputGop());
	_writer.setCodec(GetConfig()->outputFileCodec());

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
