#ifndef RTPOUTPUT_H
#define RTPOUTPUT_H

#include "common/types.h"
#include "ioutput.h"

// RTP output implemetation
class RtpOutput : public IOutput
{
private:
	std::atomic_bool _active;
	cv::VideoWriter _writer;

public:
	RtpOutput();
	virtual ~RtpOutput();

public:
	static OutputPtr create() { return std::make_shared<RtpOutput>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
};

#endif // RTPOUTPUT_H
