#ifndef GSTCAMERA_H
#define GSTCAMERA_H

#include "common/types.h"
#include "iinput.h"

namespace input
{

// OpenCV input implemetation
class GstCamera : public IInput
{
private:
	std::atomic_bool _active;
	cv::VideoCapture _capture;

public:
	GstCamera();
	virtual ~GstCamera();

public:
	static InputPtr create() { return std::make_shared<GstCamera>(); }
	bool open() override;
	void close() override;
	bool read(MatPtr &frame) override;
};

}

#endif // GSTCAMERA_H
