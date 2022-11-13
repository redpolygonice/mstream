#ifndef OCVINPUT_H
#define OCVINPUT_H

#include "common/types.h"
#include "iinput.h"

// OpenCV input implemetation
class OcvInput : public IInput
{
private:
	std::atomic_bool _active;
	cv::VideoCapture _capture;

public:
	OcvInput();
	virtual ~OcvInput();

public:
	static InputPtr create() { return std::make_shared<OcvInput>(); }
	bool open() override;
	void close() override;
	bool read(MatPtr &frame) override;
};

#endif // OCVINPUT_H
