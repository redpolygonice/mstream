#include "rtspoutput.h"

RtspOutput::RtspOutput()
	: _active(false)
{
}

RtspOutput::~RtspOutput()
{
}

bool RtspOutput::open()
{
	return true;
}

void RtspOutput::close()
{
}

bool RtspOutput::write(const MatPtr &frame)
{
	return true;
}
