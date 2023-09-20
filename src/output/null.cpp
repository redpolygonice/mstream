#include "null.h"

namespace output
{

Null::Null()
	: _active(false)
{
}

Null::~Null()
{
}

bool Null::open()
{
	return true;
}

void Null::close()
{
}

bool Null::write(const MatPtr &frame)
{
	return true;
}

}
