#ifdef USE_TENGINE
#include "tengine8bit.h"
#include "common/log.h"
#include "common/common.h"

#include <tengine/ocl_device.h>
#include <tengine/timvx_device.h>

Tengine8bit::Tengine8bit()
	: INNetwork(NnType::TengineTimvx)
	, _graph(nullptr)
	, _inputTensor(nullptr)
	, _inputScale(0.0f)
	, _inputZeroPoint(0)
	, _width(0)
	, _height(0)
{
}

Tengine8bit::~Tengine8bit()
{
}

bool Tengine8bit::init(const std::string &model, const std::string &cfg, void *params)
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
	opt.precision = TENGINE_MODE_UINT8;
	opt.affinity = 0;

	// Inital tengine
	if (init_tengine() != 0)
	{
		LOGE("Init tengine failed!");
		return false;
	}

	LOG("Tengine-lite library version: " << get_tengine_version());

	// Create graph, load tengine model xxx.tmfile
	_graph = create_graph(nullptr, "tengine", model.c_str());
	if (_graph == nullptr)
	{
		LOGE("Create graph failed!");
		return false;
	}

	int imgSize = _modelHeight * _modelWidth * _modelChannels;
	int dims[] = { 1, _modelChannels, _modelHeight, _modelWidth };
	_inputData.resize(imgSize);

	_inputTensor = get_graph_input_tensor(_graph, 0, 0);
	if (_inputTensor == nullptr)
	{
		LOGE("Get input tensor failed!");
		return false;
	}

	if (set_tensor_shape(_inputTensor, dims, 4) < 0)
	{
		LOGE("Set input tensor shape failed!");
		return false;
	}

	if (set_tensor_buffer(_inputTensor, _inputData.data(), imgSize) < 0)
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

	// Prepare process input data, set the data mem to input tensor
	get_tensor_quant_param(_inputTensor, &_inputScale, &_inputZeroPoint, 1);
	return true;
}

bool Tengine8bit::setInput(const MatPtr &frame)
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

				// quant to uint8
				float input_fp32 = (imgData[inIndex] - mean[c]) * scale[c];
				int udata = (round)(input_fp32 / _inputScale + (float)_inputZeroPoint);
				if (udata > 255)
					udata = 255;
				else if (udata < 0)
					udata = 0;

				_inputData[outIndex] = udata;
			}
		}
	}

	_width = frame->cols;
	_height = frame->rows;

	return true;
}

bool Tengine8bit::detect(RectList &out)
{
	out.clear();

	if (run_graph(_graph, 1) < 0)
		return false;

	postProcess(out);
	return true;
}

void Tengine8bit::postProcess(RectList &out)
{
	// Dequant output data
	tensor_t p8_output = get_graph_output_tensor(_graph, 2, 0);
	tensor_t p16_output = get_graph_output_tensor(_graph, 1, 0);
	tensor_t p32_output = get_graph_output_tensor(_graph, 0, 0);

	float p8_scale = 0.f;
	float p16_scale = 0.f;
	float p32_scale = 0.f;
	int p8_zero_point = 0;
	int p16_zero_point = 0;
	int p32_zero_point = 0;

	get_tensor_quant_param(p8_output, &p8_scale, &p8_zero_point, 1);
	get_tensor_quant_param(p16_output, &p16_scale, &p16_zero_point, 1);
	get_tensor_quant_param(p32_output, &p32_scale, &p32_zero_point, 1);

	int p8_count = get_tensor_buffer_size(p8_output) / sizeof(uint8_t);
	int p16_count = get_tensor_buffer_size(p16_output) / sizeof(uint8_t);
	int p32_count = get_tensor_buffer_size(p32_output) / sizeof(uint8_t);

	uint8_t* p8_data_u8 = (uint8_t*)get_tensor_buffer(p8_output);
	uint8_t* p16_data_u8 = (uint8_t*)get_tensor_buffer(p16_output);
	uint8_t* p32_data_u8 = (uint8_t*)get_tensor_buffer(p32_output);

	std::vector<float> p8_data(p8_count);
	std::vector<float> p16_data(p16_count);
	std::vector<float> p32_data(p32_count);

	for (int c = 0; c < p8_count; c++)
	{
		p8_data[c] = ((float)p8_data_u8[c] - (float)p8_zero_point) * p8_scale;
	}

	for (int c = 0; c < p16_count; c++)
	{
		p16_data[c] = ((float)p16_data_u8[c] - (float)p16_zero_point) * p16_scale;
	}

	for (int c = 0; c < p32_count; c++)
	{
		p32_data[c] = ((float)p32_data_u8[c] - (float)p32_zero_point) * p32_scale;
	}

	std::vector<Object> proposals;
	std::vector<Object> objects8;
	std::vector<Object> objects16;
	std::vector<Object> objects32;
	std::vector<Object> objects;

	_postProcess.generate_proposals(32, p32_data.data(), _confThreshold, objects32);
	proposals.insert(proposals.end(), objects32.begin(), objects32.end());
	_postProcess.generate_proposals(16, p16_data.data(), _confThreshold, objects16);
	proposals.insert(proposals.end(), objects16.begin(), objects16.end());
	_postProcess.generate_proposals(8, p8_data.data(), _confThreshold, objects8);
	proposals.insert(proposals.end(), objects8.begin(), objects8.end());

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
