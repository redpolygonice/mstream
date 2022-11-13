#include "dnnetwork.h"
#include "common/log.h"
#include "common/common.h"
#include "common/config.h"

DNNetwork::DNNetwork(NnType type)
	: INNetwork(type)
	, _width(0)
	, _height(0)
{
}

DNNetwork::~DNNetwork()
{
}

bool DNNetwork::init(const std::string &model, const std::string &cfg, void *params)
{
	if (!isFileExists(model))
	{
		LOGE("Model or config file doesn't exist!");
		return false;
	}

	switch (_type) {
	case NnType::DnnDarknet:
		_net = cv::dnn::readNet(model, cfg, "Darknet");
		break;
	case NnType::DnnCaffe:
		_net = cv::dnn::readNet(model, cfg, "Caffe");
		break;
	case NnType::DnnTensorflow:
		_net = cv::dnn::readNet(model, cfg, "TensorFlow");
		break;
	case NnType::DnnTorch:
		_net = cv::dnn::readNet(model, cfg, "Torch");
		break;
	case NnType::DnnONNX:
		_net = cv::dnn::readNet(model, cfg, "ONNX");
		break;
	default:
		LOGE("Unknown type!");
		return false;
	}

	if (_net.empty())
	{
		LOGE("Net is empty!");
		return false;
	}

	cv::dnn::Backend backend = static_cast<cv::dnn::Backend>(Config::instance()->backend());
	cv::dnn::Target target = static_cast<cv::dnn::Target>(Config::instance()->target());

	_net.setPreferableBackend(backend);
	_net.setPreferableTarget(target);

	_layerNames = getOutputsNames();
	return true;
}

bool DNNetwork::setInput(const MatPtr &frame)
{
	cv::Mat blobFromImg;
	cv::dnn::blobFromImage(*frame, blobFromImg, 1 / 255.0, cv::Size(_modelWidth, _modelHeight), cv::Scalar(), true, false);
	_net.setInput(blobFromImg);

	_width = frame->cols;
	_height = frame->rows;

	return true;
}

std::vector<cv::String> DNNetwork::getOutputsNames()
{
	std::vector<cv::String> names;
	std::vector<int> outLayers = _net.getUnconnectedOutLayers();
	std::vector<cv::String> layersNames = _net.getLayerNames();

	names.resize(outLayers.size());
	for (size_t i = 0; i < outLayers.size(); ++i)
		names[i] = layersNames[outLayers[i] - 1];

	return names;
}

bool DNNetwork::detect(RectList &out)
{
	out.clear();
	bool result = false;

	std::vector<cv::Mat> output;
	_net.forward(output, _layerNames);

	std::vector<int> classIds;
	std::vector<float> confidences;
	std::vector<cv::Rect> rects;

	for (size_t i = 0; i < output.size(); ++i)
	{
		int rowsNoOfDetection =  output[i].rows;
		int colsCoordinatesPlusClassScore =  output[i].cols;

		for (int j = 0; j < rowsNoOfDetection; ++j)
		{
			cv::Mat scores =  output[i].row(j).colRange(5, colsCoordinatesPlusClassScore);
			cv::Point positionOfMax;
			double confidence;
			minMaxLoc(scores, 0, &confidence, 0, &positionOfMax);

			if (confidence > _confThreshold)
			{
				int centerX = (int)(output[i].at<float>(j, 0) * _width);
				int centerY = (int)(output[i].at<float>(j, 1) * _height);
				int width = (int)(output[i].at<float>(j, 2) * _width);
				int height = (int)(output[i].at<float>(j, 3) * _height);
				int left = centerX - width / 2;
				int top = centerY - height / 2;

				rects.push_back(cv::Rect(left, top, width, height));
				classIds.push_back(positionOfMax.x);
				confidences.push_back(confidence);
				result = true;
			}
		}
	}

	std::vector<int> indices;
	cv::dnn::NMSBoxes(rects, confidences, _confThreshold, _nmsThreshold, indices);
	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		cv::Rect box = rects[idx];
		int classId = classIds[idx];
		out.push_back(box);
	}

	return result;
}

