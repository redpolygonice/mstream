#ifdef USE_RKNN
#include "rknn.h"
#include "common/log.h"
#include "common/common.h"

#include "rknnpp.h"

namespace nn
{

#ifdef X86_64
inline const char* get_format_string(rknn_tensor_format fmt)
{
	switch(fmt) {
	case RKNN_TENSOR_NCHW: return "NCHW";
	case RKNN_TENSOR_NHWC: return "NHWC";
	default: return "UNKNOW";
	}
}

inline const char* get_qnt_type_string(rknn_tensor_qnt_type type)
{
	switch(type) {
	case RKNN_TENSOR_QNT_NONE: return "NONE";
	case RKNN_TENSOR_QNT_DFP: return "DFP";
	case RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC: return "AFFINE";
	default: return "UNKNOW";
	}
}

inline const char* get_type_string(rknn_tensor_type type)
{
	switch(type) {
	case RKNN_TENSOR_FLOAT32: return "FP32";
	case RKNN_TENSOR_FLOAT16: return "FP16";
	case RKNN_TENSOR_INT8: return "INT8";
	case RKNN_TENSOR_UINT8: return "UINT8";
	case RKNN_TENSOR_INT16: return "INT16";
	default: return "UNKNOW";
	}
}
#endif

static void dump_tensor_attr(rknn_tensor_attr* attr)
{
	char dump[1024];
	sprintf(dump, "  index=%d, name=%s, n_dims=%d, dims=[%d, %d, %d, %d], n_elems=%d, size=%d, fmt=%s, type=%s, qnt_type=%s, "
				  "zp=%d, scale=%f\n",
			attr->index, attr->name, attr->n_dims, attr->dims[0], attr->dims[1], attr->dims[2], attr->dims[3],
			attr->n_elems, attr->size, get_format_string(attr->fmt), get_type_string(attr->type),
			get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);

	LOG(dump);
}

Rknn::Rknn()
	: INNetwork(NnType::Rknn)
	, _width(0)
	, _height(0)
{
}

Rknn::~Rknn()
{
}

bool Rknn::init(const std::string &model, const std::string &cfg, void *params)
{
	if (!isFileExists(model))
	{
		LOGE("Model or config file doesn't exist!");
		return false;
	}

	uint8_t *modelData = nullptr;
	size_t modelSize = 0;
	if (!readFile(model, &modelData, &modelSize))
	{
		LOGE("Can't read model file!");
		return false;
	}

	LOG("Init model");

#ifdef X86_64
	int result = rknn_init(&_ctx, modelData, modelSize, 0);
#else
	int result = rknn_init(&_ctx, modelData, modelSize, 0, 0);
#endif
	if (result < 0)
	{
		LOGE("rknn_init error, return code " << result);
		return false;
	}

	LOG("Query the number of input & output tensor");

	result = rknn_query(_ctx, RKNN_QUERY_IN_OUT_NUM, &_io_num, sizeof(_io_num));
	if (result != RKNN_SUCC)
	{
		LOGE("rknn_query RKNN_QUERY_IN_OUT_NUM error, return code " << result);
		return false;
	}

	LOG("Model input num: " << _io_num.n_input << ", output num: " << _io_num.n_output);

	LOG("Input tensors:");
	_in_attr = new rknn_tensor_attr[_io_num.n_input];
	memset(_in_attr, 0, sizeof(rknn_tensor_attr) *  _io_num.n_input);

	for (int i = 0; i < _io_num.n_input; i++)
	{
		_in_attr[i].index = i;
		result = rknn_query(_ctx, RKNN_QUERY_INPUT_ATTR, &(_in_attr[i]), sizeof(rknn_tensor_attr));
		if (result != RKNN_SUCC)
		{
			LOGE("rknn_query RKNN_QUERY_INPUT_ATTR error, return code " << result);
			return false;
		}

		dump_tensor_attr(&(_in_attr[i]));
	}

	LOG("Output tensors:");
	_out_attr = new rknn_tensor_attr[_io_num.n_output];
	memset(_out_attr, 0, sizeof(rknn_tensor_attr) * _io_num.n_output);

	for (int i = 0; i < _io_num.n_output; i++)
	{
		_out_attr[i].index = i;
		result = rknn_query(_ctx, RKNN_QUERY_OUTPUT_ATTR, &(_out_attr[i]), sizeof(rknn_tensor_attr));
		if (result != RKNN_SUCC)
		{
			LOGE("rknn_query RKNN_QUERY_OUTPUT_ATTR error, return code " << result);
			return false;
		}

		dump_tensor_attr(&(_out_attr[i]));
	}

	return true;
}

bool Rknn::setInput(const MatPtr &origFrame)
{
	cv::Mat frame = origFrame->clone();
	if (origFrame->cols != _modelWidth || origFrame->rows != _modelHeight)
		cv::resize(*origFrame, frame, cv::Size(_modelWidth, _modelHeight));

	// Set Input Data
	rknn_input inputs[1];
	memset(inputs, 0, sizeof(inputs));

	inputs[0].index = 0;
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].size = frame.cols * frame.rows * frame.channels();
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].buf = frame.data;

	int result = rknn_inputs_set(_ctx, _io_num.n_input, inputs);
	if (result < 0)
	{
		LOGE("rknn_inputs_set error, return code " << result);
		return false;
	}

	return true;
}

bool Rknn::detect(RectList &out)
{
	out.clear();

	// Run
	int result = rknn_run(_ctx, nullptr);
	if (result < 0)
	{
		LOGE("rknn_run error, return code " << result);
		return false;
	}

	// Get Output
	rknn_output outputs[_io_num.n_output];
	memset(outputs, 0, sizeof(outputs));

	for (int i = 0; i < _io_num.n_output; ++i)
	{
		outputs[i].want_float = true;
		outputs[i].is_prealloc = false;
		outputs[i].index = i;
	}

	result = rknn_outputs_get(_ctx, _io_num.n_output, outputs, nullptr);
	if (result < 0)
	{
		LOGE("rknn_outputs_get error, return code " << result);
		return false;
	}

	// Post Process
	detection_t* dets = 0;
	int nboxes_total = (GRID0 * GRID0 + GRID1 * GRID1 + GRID2 * GRID2) * nanchor;
	dets = (detection_t*)calloc(nboxes_total, sizeof(detection_t));
	for(int i = 0; i < nboxes_total; ++i)
	{
		dets[i].prob = (float*) calloc(nclasses, sizeof(float));
		dets[i].objectness = 0;
	}

	outputs_transform(outputs, IMG_WID, IMG_HGT, dets);
	int nboxes_left = do_nms_sort(dets, nboxes_total, nclasses, NMS_THRESH);
	get_det_rects(out, dets, _width, _height, nboxes_left, DRAW_CLASS_THRESH);

	rknn_outputs_release(_ctx, _io_num.n_output, outputs);
	free_detections(dets, _io_num.n_output);
	return true;
}

}

#endif
