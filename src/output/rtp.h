#ifndef RTPOUTPUT_H
#define RTPOUTPUT_H

#include "common/types.h"
#include "ioutput.h"

namespace output
{

// RTP output implemetation
class Rtp : public IOutput
{
private:
	std::atomic_bool _active;
	cv::VideoWriter _writer;

public:
	Rtp();
	virtual ~Rtp();

public:
	static OutputPtr create() { return std::make_shared<Rtp>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
};

}

#endif // RTPOUTPUT_H
