#ifndef NULLOUTPUT_H
#define NULLOUTPUT_H

#include "common/types.h"
#include "ioutput.h"

namespace output
{

// Null output implemetation for testing
class Null : public IOutput
{
private:
	std::atomic_bool _active;

public:
	Null();
	virtual ~Null();

public:
	static OutputPtr create() { return std::make_shared<Null>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
};

}

#endif // NULLOUTPUT_H
