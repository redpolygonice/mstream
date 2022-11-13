#ifndef IINPUT_H
#define IINPUT_H

#include "common/types.h"

// Input interface
class IInput
{
public:
	IInput() {}
	virtual ~IInput() {}

public:
	virtual bool open() = 0;
	virtual void close() = 0;
	virtual bool read(MatPtr &frame) = 0;
};

typedef std::shared_ptr<IInput> InputPtr;

#endif // IINPUT_H
