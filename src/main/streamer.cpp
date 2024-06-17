#include "streamer.h"
#include "common/common.h"
#include "common/log.h"
#include "common/config.h"
#include "input/gstcamera.h"
#include "input/camera.h"
#include "input/file.h"
#include "input/v4lcamera.h"
#include "input/libcamera.h"
#include "output/rtp.h"
#include "output/rtsp.h"
#include "output/tcp.h"
#include "output/hls.h"
#include "output/file.h"
#include "output/null.h"

#include "nn/dnn.h"
#include "nn/dnnonnx.h"
#include "nn/khadas.h"

#ifdef USE_RKNN
#include "nn/rknn.h"
#endif

#ifdef USE_TENGINE
#include "nn/tengine.h"
#include "nn/tengine8bit.h"
#include "nn/tenginetimvx.h"
#endif

Streamer::Streamer()
	: _active(false)
	, _nn(nullptr)
	, _input(nullptr)
	, _output(nullptr)
	, _readThread(nullptr)
	, _writeThread(nullptr)
	, _window(true)
	, _async(true)
	, _imgproc(false)
{
}

Streamer::~Streamer()
{
	stop();
}

bool Streamer::start()
{
	// Load settings
	if (!GetConfig()->load())
	{
		LOGE("Can't load settings!");
		return false;
	}

	// Load neural network
	_detect = GetConfig()->detect();
	if (_detect)
	{
		if (GetConfig()->nnType() == NnType::DnnDarknet)
			_nn = nn::Dnn::create(GetConfig()->nnType());
		else if (GetConfig()->nnType() == NnType::DnnCaffe)
			_nn = nn::Dnn::create(GetConfig()->nnType());
		else if (GetConfig()->nnType() == NnType::DnnTensorflow)
			_nn = nn::Dnn::create(GetConfig()->nnType());
		else if (GetConfig()->nnType() == NnType::DnnTorch)
			_nn = nn::Dnn::create(GetConfig()->nnType());
		else if (GetConfig()->nnType() == NnType::DnnONNX)
			_nn = nn::DnnOnnx::create(GetConfig()->nnType());
		else if (GetConfig()->nnType() == NnType::Khadas)
			_nn = nn::Khadas::create();
#ifdef USE_RKNN
		else if (GetConfig()->nnType() == NnType::Rknn)
			_nn = nn::Rknn::create();
#endif
#ifdef USE_TENGINE
		else if (GetConfig()->nnType() == NnType::Tengine)
			_nn = nn::Tengine::create();
		else if (GetConfig()->nnType() == NnType::Tengine8bit)
			_nn = nn::Tengine8bit::create();
		else if (GetConfig()->nnType() == NnType::TengineTimvx)
			_nn = nn::TengineTimvx::create();
#endif

		if (_nn == nullptr)
		{
			LOGE("Wrong NN type!");
			return false;
		}

		_nn->setModelWidth(GetConfig()->modelWidth());
		_nn->setModelHeight(GetConfig()->modelHeight());
		_nn->setModelChannels(GetConfig()->modelChannels());
		_nn->setNumClasses(GetConfig()->numClasses());
		_nn->setConfThreshold(GetConfig()->confThreshold());
		_nn->setNmsThreshold(GetConfig()->nmsThreshold());
		_nn->setClassesFile(GetConfig()->classesFile());

		if (!_nn->init(GetConfig()->modelPath(), GetConfig()->cfgPath()))
		{
			LOGE("Can't load NN!");
			return false;
		}
	}

	// Load image processor
	_imgproc = GetConfig()->imgproc();
	if (_imgproc)
	{
		if (!_imageProcessor.load())
		{
			LOGE("Can't load Image Processor!");
			return false;
		}
	}

	// Create input object
	if (GetConfig()->inputType() == InputType::Camera)
		_input = input::Camera::create();
	else if (GetConfig()->inputType() == InputType::GstCamera)
		_input = input::GstCamera::create();
	else if (GetConfig()->inputType() == InputType::V4lCamera)
		_input = input::V4lCamera::create();
	else if (GetConfig()->inputType() == InputType::LibCamera)
		_input = input::LibCamera::create();
	else if (GetConfig()->inputType() == InputType::File)
		_input = input::File::create();

	if (_input == nullptr)
	{
		LOGE("Can't create input object!");
		return false;
	}

	if (!_input->open())
	{
		LOGE("Can't open input object!");
		return false;
	}

	// Create output object
	if (GetConfig()->outputType() == OutputType::Null)
		_output = output::Null::create();
	else if (GetConfig()->outputType() == OutputType::Rtp)
		_output = output::Rtp::create();
	else if (GetConfig()->outputType() == OutputType::Rtsp)
		_output = output::Rtsp::create();
	else if (GetConfig()->outputType() == OutputType::Tcp)
		_output = output::Tcp::create();
	else if (GetConfig()->outputType() == OutputType::Hls)
		_output = output::Hls::create();
	else if (GetConfig()->outputType() == OutputType::File)
		_output = output::File::create();

	if (_output == nullptr)
	{
		LOGE("Can't create output object!");
		return false;
	}

	if (!_output->open())
	{
		LOGE("Can't open output!");
		return false;
	}

	_window = GetConfig()->window();
	_async = GetConfig()->async();
	_active = true;

	if (_async)
	{
		_readThread = std::make_unique<std::thread>( std::thread([this]() { read(); }));
		_writeThread = std::make_unique<std::thread>(std::thread([this]() { write(); }));
		_writeThread->join();
	}
	else
		write();

	return true;
}

void Streamer::stop()
{
	if (!_active)
		return;

	_active = false;

	if (_readThread != nullptr && _readThread->joinable())
		_readThread->join();

	if (_writeThread != nullptr && _writeThread->joinable())
		_writeThread->join();

	if (_input != nullptr)
	{
		_input->close();
		_input.reset();
	}

	if (_output != nullptr)
	{
		_output->close();
		_output.reset();
	}
}

void Streamer::read()
{
	while (_active)
	{
		MatPtr frame = std::make_shared<cv::Mat>();
		if (_input->read(frame))
			_data.push(frame);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Streamer::write()
{
	string windowName = "Streamer";
	if (_window)
		cv::namedWindow(windowName);

	LOG("Start detect ..");

	int delay = 1000 / GetConfig()->outputFps();
	if (_detect)
		delay = std::max(delay - 50, 10);

	while (_active)
	{
		if (!_output->ready())
		{
			sleepFor(10);
			continue;
		}

		MatPtr frame = nullptr;
		if (_async)
			frame = _data.pop();
		else
		{
			frame = std::make_shared<cv::Mat>();
			if (!_input->read(frame))
			{
				frame.reset();
				frame = nullptr;
			}
		}

		if (frame != nullptr)
		{
			// Process image
			if (_imgproc)
				_imageProcessor.process(frame);

			// Detect
			if (_detect)
			{
				if (_detectAsync)
				{
					detect_async(frame, [&]() {
						// Missing frames while detection is working
						while (!_detectResult)
						{
							_input->read();
							sleepFor(5);
						}
					});
				}
				else
					detect(frame);
			}

			// Show window
			if (_window)
			{
				cv::resize(*frame, *frame, cv::Size(GetConfig()->outputWidth(), GetConfig()->outputHeight()));
				cv::imshow(windowName, *frame);
			}

			// Write to output
			_output->write(frame);
		}

		if (_window)
			cv::waitKey(delay);
		else
			sleepFor(delay);
	}

	if (_window)
		cv::destroyWindow(windowName);
}

bool Streamer::detect(const MatPtr &frame)
{
	bool result = false;
	RectList rects;
	_nn->setInput(frame);
	if (_nn->detect(rects))
	{
		LOG("Detect count: " << rects.size());
		for (cv::Rect &rect : rects)
			cv::rectangle(*frame, rect, cv::Scalar(0, 0, 255), 3, 8, 0);
		result = true;
	}

	return result;
}

bool Streamer::detect_async(const MatPtr &frame, const WaitFunc &waitFunc)
{
	_detectResult = false;
	std::future<bool> future = std::async(std::launch::async, [&]() {
		bool result = detect(frame);
		_detectResult = true;
		return result;
	});

	if (waitFunc != nullptr)
		waitFunc();

	return future.get();
}
