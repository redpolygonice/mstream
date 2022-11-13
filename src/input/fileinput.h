#ifndef FILEINPUT_H
#define FILEINPUT_H

#include "common/types.h"
#include "iinput.h"

// Video file input implemetation
class FileInput : public IInput
{
private:
	std::atomic_bool _active;
	cv::VideoCapture _reader;

	int _width;
	int _height;
	int _channels;
	int _type;

public:
	FileInput();
	virtual ~FileInput();

public:
	static InputPtr create() { return std::make_shared<FileInput>(); }
	bool open() override;
	void close() override;
	bool read(MatPtr &frame) override;
};

#endif // FILEINPUT_H
