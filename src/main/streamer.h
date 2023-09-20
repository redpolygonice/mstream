#ifndef STREAMER_H
#define STREAMER_H

#include "common/types.h"
#include "nn/innetwork.h"
#include "input/iinput.h"
#include "output/ioutput.h"
#include "imgproc/imageprocessor.h"
#include "common/threadsafequeue.h"

class Streamer;
typedef std::shared_ptr<Streamer> StreamerPtr;

// Streamer object
class Streamer : public std::enable_shared_from_this<Streamer>
{
private:
	std::atomic_bool _active;
	nn::NNetworkPtr _nn;
	input::InputPtr _input;
	output::OutputPtr _output;
	ImageProcessor _imageProcessor;
	ThreadSafeQueue<MatPtr> _data;
	std::unique_ptr<std::thread> _readThread;
	std::unique_ptr<std::thread> _writeThread;

	bool _window;
	bool _async;
	bool _detect;
	bool _imgproc;

	static StreamerPtr _instance;

private:
	void read();
	void write();
	bool detect(const MatPtr &frame);

public:
	Streamer();
	~Streamer();

public:
	bool start();
	void stop();

	static StreamerPtr instance() { return _instance; }
};

#endif // STREAMER_H
