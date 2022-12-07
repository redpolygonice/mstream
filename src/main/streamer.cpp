#include "streamer.h"
#include "common/log.h"
#include "common/config.h"
#include "input/ocvinput.h"
#include "input/fileinput.h"
#include "output/rtpoutput.h"
#include "output/hlsoutput.h"
#include "output/fileoutput.h"
#include "output/nulloutput.h"

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

StreamerPtr Streamer::_instance = nullptr;

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
	if (!Config::instance()->load())
	{
		LOGE("Can't load settings!");
		return false;
	}

	// Load neural network
	_detect = Config::instance()->detect();
	if (_detect)
	{
		if (Config::instance()->nnType() == NnType::DnnDarknet)
			_nn = Dnn::create(Config::instance()->nnType());
		else if (Config::instance()->nnType() == NnType::DnnCaffe)
			_nn = Dnn::create(Config::instance()->nnType());
		else if (Config::instance()->nnType() == NnType::DnnTensorflow)
			_nn = Dnn::create(Config::instance()->nnType());
		else if (Config::instance()->nnType() == NnType::DnnTorch)
			_nn = Dnn::create(Config::instance()->nnType());
		else if (Config::instance()->nnType() == NnType::DnnONNX)
			_nn = DnnOnnx::create(Config::instance()->nnType());
		else if (Config::instance()->nnType() == NnType::Khadas)
			_nn = Khadas::create();
#ifdef USE_RKNN
		else if (Config::instance()->nnType() == NnType::Rknn)
			_nn = Rknn::create();
#endif
#ifdef USE_TENGINE
		else if (Config::instance()->nnType() == NnType::Tengine)
			_nn = Tengine::create();
		else if (Config::instance()->nnType() == NnType::Tengine8bit)
			_nn = Tengine8bit::create();
		else if (Config::instance()->nnType() == NnType::TengineTimvx)
			_nn = TengineTimvx::create();
#endif

		if (_nn == nullptr)
		{
			LOGE("Wrong NN type!");
			return false;
		}

		_nn->setModelWidth(Config::instance()->modelWidth());
		_nn->setModelHeight(Config::instance()->modelHeight());
		_nn->setModelChannels(Config::instance()->modelChannels());
		_nn->setNumClasses(Config::instance()->numClasses());
		_nn->setConfThreshold(Config::instance()->confThreshold());
		_nn->setNmsThreshold(Config::instance()->nmsThreshold());
		_nn->setClassesFile(Config::instance()->classesFile());

		if (!_nn->init(Config::instance()->modelPath(), Config::instance()->cfgPath()))
		{
			LOGE("Can't load NN!");
			return false;
		}
	}

	// Load image processor
	_imgproc = Config::instance()->imgproc();
	if (_imgproc)
	{
		if (!_imageProcessor.load())
		{
			LOGE("Can't load Image Processor!");
			return false;
		}
	}

	// Create input object
	if (Config::instance()->inputType() == InputType::Camera)
		_input = OcvInput::create();
	else if (Config::instance()->inputType() == InputType::File)
		_input = FileInput::create();

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
	if (Config::instance()->outputType() == OutputType::Null)
		_output = NullOutput::create();
	else if (Config::instance()->outputType() == OutputType::Rtp)
		_output = RtpOutput::create();
	else if (Config::instance()->outputType() == OutputType::Hls)
		_output = HlsOutput::create();
	else if (Config::instance()->outputType() == OutputType::File)
		_output = FileOutput::create();

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

	_window = Config::instance()->window();
	_async = Config::instance()->async();
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

	_instance = nullptr;
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

	while (_active)
	{
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
				detect(frame);

			// Show window
			if (_window)
			{
				cv::resize(*frame, *frame, cv::Size(Config::instance()->outputWidth(), Config::instance()->outputHeight()));
				cv::imshow(windowName, *frame);
			}

			// Write to output
			_output->write(frame);
		}

		if (_window)
			cv::waitKey(10);
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
