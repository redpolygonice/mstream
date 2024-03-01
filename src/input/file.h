#ifndef FILEINPUT_H
#define FILEINPUT_H

#include "common/types.h"
#include "iinput.h"

namespace input
{

// Video file input implemetation
class File : public IInput
{
private:
	std::atomic_bool _active;
	cv::VideoCapture _reader;

	int _width;
	int _height;
	int _channels;
	int _type;

public:
	File();
	virtual ~File();

public:
	static InputPtr create() { return std::make_shared<File>(); }
	bool open() override;
	void close() override;
	bool read(MatPtr &frame) override;
	bool read() override;
};

}

#endif // FILEINPUT_H
