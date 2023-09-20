#include "camera.h"
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
	if (!_capture.open(Config::instance()->cameraDev()))
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
