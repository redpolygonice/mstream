#ifdef USE_TENGINE
#include "tengine.h"
#include "common/log.h"
#include "common/common.h"

#include <tengine/ocl_device.h>

Tengine::Tengine()
	: INNetwork(NnType::Tengine)
	, _graph(nullptr)
	, _inputTensor(nullptr)
	, _opencl(true)
	, _width(0)
	, _height(0)
{
}

Tengine::~Tengine()
{
}

bool Tengine::init(const std::string &model, const std::string &cfg, void *params)
{
	_postProcess.setParent(shared_from_this());

	if (!isFileExists(model))
	{
		LOGE("Model or config file doesn't exist!");
		return false;
	}

	// Set runtime options
	struct options opt;
	opt.num_thread = threads;
	opt.cluster = TENGINE_CLUSTER_ALL;
	opt.precision = TENGINE_MODE_FP32;
	opt.affinity = 0;

	// Inital tengine
	if (init_tengine() != 0)
	{
		LOGE("Init tengine failed!");
		return false;
	}

	LOG("Tengine-lite library version: " << get_tengine_version());

	if (_opencl)
	{
		LOG("Trying OpenCL ..");
		context_t openclContext = create_context("ocl", 1);

		struct ocl_option oclOpt;
		memset(&oclOpt, 0, sizeof(oclOpt));

		char cacheFile[] = "test.cache";
		if (isFileExists(cacheFile))
			oclOpt.load_cache = true;
		else
			oclOpt.load_cache = false;
		oclOpt.cache_path = cacheFile;
		oclOpt.store_cache = true;

		if (set_context_device(openclContext, "OCL", (void*)&oclOpt, sizeof(oclOpt)) != 0)
		{
			LOGW("Add context device opencl failed!");
		}

		// Create graph, load tengine model xxx.tmfile
		_graph = create_graph(openclContext, "tengine", model.c_str());
		if (_graph == nullptr)
		{
			LOGW("Load OpenCL failed!");
			_graph = create_graph(nullptr, "tengine", model.c_str());
			if (_graph == nullptr)
			{
				LOGE("Create graph failed!");
				return false;
			}
		}
	}
	else
	{
		_graph = create_graph(nullptr, "tengine", model.c_str());
		if (_graph == nullptr)
		{
			LOGE("Create graph failed!");
			return false;
		}
	}

	int imgSize = _modelHeight * _modelWidth * _modelChannels;
	int dims[] = { 1, _modelChannels, _modelHeight, _modelWidth };
	_inputData.resize(imgSize);

	tensor_t inputTensor = get_graph_input_tensor(_graph, 0, 0);
	if (inputTensor == nullptr)
	{
		LOGE("Get input tensor failed!");
		return false;
	}

	if (set_tensor_shape(inputTensor, dims, 4) < 0)
	{
		LOGE("Set input tensor shape failed!");
		return false;
	}

	if (set_tensor_buffer(inputTensor, _inputData.data(), imgSize * sizeof(float)) < 0)
	{
		LOGE("Set input tensor buffer failed!");
		return false;
	}

	// Prerun graph, set work options(num_thread, cluster, precision)
	if (prerun_graph_multithread(_graph, opt) < 0)
	{
		LOGE("Prerun multithread graph failed!");
		return false;
	}

	return true;
}

bool Tengine::setInput(const MatPtr &frame)
{
	cv::Mat img;
	if (frame->channels() == 1)
		cv::cvtColor(*frame, img, cv::COLOR_GRAY2RGB);
	else
		cv::cvtColor(*frame, img, cv::COLOR_BGR2RGB);

	cv::resize(img, img, cv::Size(_modelWidth, _modelHeight));
	img.convertTo(img, CV_32FC3);
	float *imgData = (float*)img.data;

	// nhwc to nchw
	for (int h = 0; h < _modelHeight; h++)
	{
		for (int w = 0; w < _modelWidth; w++)
		{
			for (int c = 0; c < _modelChannels; c++)
			{
				int inIndex = h * _modelWidth * _modelChannels + w * _modelChannels + c;
				int outIndex = c * _modelHeight * _modelWidth + h * _modelWidth + w;
				_inputData[outIndex] = (imgData[inIndex] - mean[c]) * scale[c];
			}
		}
	}

	_width = frame->cols;
	_height = frame->rows;

	return true;
}

bool Tengine::detect(RectList &out)
{
	out.clear();

	if (run_graph(_graph, 1) < 0)
		return false;

	postProcess(out);
	return true;
}

void Tengine::postProcess(RectList &out)
{
	tensor_t p8_output = get_graph_output_tensor(_graph, 2, 0);
	tensor_t p16_output = get_graph_output_tensor(_graph, 1, 0);
	tensor_t p32_output = get_graph_output_tensor(_graph, 0, 0);

	float* p8_data = nullptr;
	float* p16_data = nullptr;
	float* p32_data = nullptr;

	if (p8_output != nullptr)
		p8_data = (float*)get_tensor_buffer(p8_output);

	if (p16_output != nullptr)
		p16_data = (float*)get_tensor_buffer(p16_output);

	if (p32_output != nullptr)
		p32_data = (float*)get_tensor_buffer(p32_output);

	std::vector<Object> proposals;
	std::vector<Object> objects8;
	std::vector<Object> objects16;
	std::vector<Object> objects32;
	std::vector<Object> objects;

	if (p32_data != nullptr)
	{
		_postProcess.generate_proposals(32, p32_data, _confThreshold, objects32);
		proposals.insert(proposals.end(), objects32.begin(), objects32.end());
	}

	if (p16_data != nullptr)
	{
		_postProcess.generate_proposals(16, p16_data, _confThreshold, objects16);
		proposals.insert(proposals.end(), objects16.begin(), objects16.end());
	}

	if (p8_data != nullptr)
	{
		_postProcess.generate_proposals(8, p8_data, _confThreshold, objects8);
		proposals.insert(proposals.end(), objects8.begin(), objects8.end());
	}

	_postProcess.sort_descent_inplace(proposals);

	std::vector<int> picked;
	_postProcess.nms_sorted_bboxes(proposals, picked, _nmsThreshold);

	float ratio_x = (float)_width / _modelWidth;
	float ratio_y = (float)_height / _modelHeight;

	int count = picked.size();
	objects.resize(count);

	for (int i = 0; i < count; i++)
	{
		objects[i] = proposals[picked[i]];
		float x0 = (objects[i].rect.x);
		float y0 = (objects[i].rect.y);
		float x1 = (objects[i].rect.x + objects[i].rect.width);
		float y1 = (objects[i].rect.y + objects[i].rect.height);

		x0 = x0 * ratio_x;
		y0 = y0 * ratio_y;
		x1 = x1 * ratio_x;
		y1 = y1 * ratio_y;

		x0 = std::max(std::min(x0, (float)(_width - 1)), 0.f);
		y0 = std::max(std::min(y0, (float)(_height - 1)), 0.f);
		x1 = std::max(std::min(x1, (float)(_width - 1)), 0.f);
		y1 = std::max(std::min(y1, (float)(_height - 1)), 0.f);

		objects[i].rect.x = x0;
		objects[i].rect.y = y0;
		objects[i].rect.width = x1 - x0;
		objects[i].rect.height = y1 - y0;

		out.push_back(objects[i].rect);
	}
}
#endif
