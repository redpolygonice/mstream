#ifndef IOUTPUT_H
#define IOUTPUT_H

#include "common/types.h"

// Output interface
class IOutput
{
public:
	IOutput() {}
	virtual ~IOutput() {}

public:
	virtual bool open() = 0;
	virtual void close() = 0;
	virtual bool write(const MatPtr &frame) = 0;
};

typedef std::shared_ptr<IOutput> OutputPtr;

#endif // IOUTPUT_H
