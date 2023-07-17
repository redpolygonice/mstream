#ifndef RTSPOUTPUT_H
#define RTSPOUTPUT_H

#include "common/types.h"
#include "ioutput.h"

// RTSP output implemetation for testing
class RtspOutput : public IOutput
{
private:
	std::atomic_bool _active;

public:
	RtspOutput();
	virtual ~RtspOutput();

public:
	static OutputPtr create() { return std::make_shared<RtspOutput>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
};

#endif // RTSPOUTPUT_H
