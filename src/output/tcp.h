#ifndef TCPOUTPUT_H
#define TCPOUTPUT_H

#include "common/types.h"
#include "ioutput.h"

namespace output
{

// RTP output implemetation
class Tcp : public IOutput
{
private:
	std::atomic_bool _active;
	cv::VideoWriter _writer;

public:
	Tcp();
	virtual ~Tcp();

public:
	static OutputPtr create() { return std::make_shared<Tcp>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
};

}

#endif // TCPOUTPUT_H
