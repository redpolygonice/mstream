#include "camera.h"
#include "common/common.h"
#include "common/log.h"
#include "common/config.h"

namespace input
{

Camera::Camera()
	: _active(false)
{
}

Camera::~Camera()
{
}

bool Camera::open()
{
	int dev = Config::instance()->cameraDev();
	if (dev < 0)
	{
		dev = getVideoDevice();
		if (dev < 0)
		{
			LOGE("No camera device!");
			return false;
		}
		else
		{
			LOG("Found video device #" << dev);
		}
	}

	if (!_capture.open(dev))
	{
		LOGE("Can't open camera device!");
		return false;
	}

	_active = true;
	return true;
}

void Camera::close()
{
	_active = false;
	_capture.release();
}

bool Camera::read(MatPtr &frame)
{
	if (_capture.read(*frame))
		return true;
	return false;
}

}
