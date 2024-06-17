#ifndef LIBCAMERA_H
#define LIBCAMERA_H

#include "common/types.h"
#include "iinput.h"

namespace input
{

// OpenCV input implemetation
class LibCamera : public IInput
{
private:
	std::atomic_bool _active;
	cv::VideoCapture _capture;

public:
	LibCamera();
	virtual ~LibCamera();

public:
	static InputPtr create() { return std::make_shared<LibCamera>(); }
	bool open() override;
	void close() override;
	bool read(MatPtr &frame) override;
};

}

#endif // LIBCAMERA_H
