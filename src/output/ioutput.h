#ifndef IOUTPUT_H
#define IOUTPUT_H

#include "common/types.h"

namespace output
{

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
	virtual bool active() const { return false; }
	virtual bool ready() const { return true; }
};

typedef std::shared_ptr<IOutput> OutputPtr;

}

#endif // IOUTPUT_H
