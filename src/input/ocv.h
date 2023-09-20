#ifndef OCVINPUT_H
#define OCVINPUT_H

#include "common/types.h"
#include "iinput.h"

namespace input
{

// OpenCV input implemetation
class Ocv : public IInput
{
private:
	std::atomic_bool _active;
	cv::VideoCapture _capture;

public:
	Ocv();
	virtual ~Ocv();

public:
	static InputPtr create() { return std::make_shared<Ocv>(); }
	bool open() override;
	void close() override;
	bool read(MatPtr &frame) override;
};

}

#endif // OCVINPUT_H
