#include "nulloutput.h"

NullOutput::NullOutput()
	: _active(false)
{
}

NullOutput::~NullOutput()
{
}

bool NullOutput::open()
{
	return true;
}

void NullOutput::close()
{
}

bool NullOutput::write(const MatPtr &frame)
{
	return true;
}
