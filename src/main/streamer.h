#ifndef STREAMER_H
#define STREAMER_H

#include "common/types.h"
#include "nn/innetwork.h"
#include "input/iinput.h"
#include "output/ioutput.h"
#include "imgproc/imageprocessor.h"
#include "common/threadsafequeue.h"

using WaitFunc = std::function<void()>;

// Streamer object
class Streamer
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

	std::thread _detectThread;
	std::mutex _detectMutex;
	std::condition_variable _detectWait;
	std::atomic_bool _detectResult;

	bool _window;
	bool _async;
	bool _detect;
	bool _detectAsync = false;
	bool _imgproc;

private:
	void read();
	void write();
	bool detect(const MatPtr &frame);
	bool detect_async(const MatPtr &frame, const WaitFunc &waitFunc = nullptr);

public:
	Streamer();
	~Streamer();

public:
	bool start();
	void stop();
};

using StreamerPtr = std::shared_ptr<Streamer>;

inline StreamerPtr GetStreamer()
{
	static StreamerPtr streamer = nullptr;
	if (streamer == nullptr)
		streamer = std::make_shared<Streamer>();
	return streamer;
}

#endif // STREAMER_H
