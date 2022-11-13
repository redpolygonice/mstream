#ifndef NULLOUTPUT_H
#define NULLOUTPUT_H

#include "common/types.h"
#include "ioutput.h"

// Null output implemetation for testing
class NullOutput : public IOutput
{
private:
	std::atomic_bool _active;

public:
	NullOutput();
	virtual ~NullOutput();

public:
	static OutputPtr create() { return std::make_shared<NullOutput>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
};

#endif // NULLOUTPUT_H
