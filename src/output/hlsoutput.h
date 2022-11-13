#ifndef HLSOUTPUT_H
#define HLSOUTPUT_H

#include "common/types.h"
#include "ioutput.h"

// HLS output implemetation
class HlsOutput : public IOutput
{
private:
	std::atomic_bool _active;
	cv::VideoWriter _writer;
	std::thread _httpThread;

public:
	HlsOutput();
	virtual ~HlsOutput();

public:
	static OutputPtr create() { return std::make_shared<HlsOutput>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;

private:
	bool startHttpServer();
	void stopHttpServer();
	string getHlsDir();
};

#endif // HLSOUTPUT_H
