#include "dnnonnx.h"
#include "common/log.h"
#include "common/common.h"
#include "common/config.h"

namespace nn
{

DnnOnnx::DnnOnnx(NnType type)
	: INNetwork(type)
	, _dnnType(DnnType::Darknet)
	, _width(0)
	, _height(0)
{
}

DnnOnnx::~DnnOnnx()
{
}

bool DnnOnnx::init(const std::string &model, const std::string &cfg, void *params)
{
	if (!isFileExists(model))
	{
		LOGE("Model or config file doesn't exist!");
		return false;
	}

	try
	{
		_net = cv::dnn::readNet(model, cfg);
	}
	catch (...)
	{
		LOGE("Can't read Net!");
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
	_layerNames = _net.getUnconnectedOutLayersNames();

	if (isFileExists(_classesFile))
		loadClasses(_classesFile, _classes);

	return true;
}

bool DnnOnnx::setInput(const MatPtr &frame)
{
	cv::Mat blobFromImg;

	if (_dnnType == DnnType::Darknet)
		cv::dnn::blobFromImage(*frame, blobFromImg, 1.0 / 255.0, cv::Size(_modelWidth, _modelHeight), cv::Scalar(), true, false);
	else if (_dnnType == DnnType::Mobilenet2)
		cv::dnn::blobFromImage(*frame, blobFromImg, 0.226, cv::Size(_modelWidth, _modelHeight), cv::Scalar(0.485, 0.456, 0.406), true, true);
	else if (_dnnType == DnnType::Yolov5)
		cv::dnn::blobFromImage(*frame, blobFromImg, 1.0 / 255.0, cv::Size(_modelWidth, _modelHeight), cv::Scalar(), true, false);
	else if (_dnnType == DnnType::Yolox)
		cv::dnn::blobFromImage(*frame, blobFromImg, 0.226, cv::Size(_modelWidth, _modelHeight), cv::Scalar(0.485, 0.456, 0.406), true, false);
	else if (_dnnType == DnnType::Nanodet)
		cv::dnn::blobFromImage(*frame, blobFromImg, 0.226, cv::Size(_modelWidth, _modelHeight), cv::Scalar(0.485, 0.456, 0.406), true, true);

	_net.setInput(blobFromImg);
	_width = frame->cols;
	_height = frame->rows;

	return true;
}

bool DnnOnnx::detect(RectList &out)
{
	if (_dnnType == DnnType::Darknet)
		return detectDarknet(out);
	else if (_dnnType == DnnType::Mobilenet2)
		return detectMobilenet2(out);
	else if (_dnnType == DnnType::Yolov5)
		return detectYolov5(out);
	else if (_dnnType == DnnType::Yolox)
		return detectYolox(out);
	else if (_dnnType == DnnType::Nanodet)
		return detectNanodet(out);
	return false;
}

bool DnnOnnx::detectDarknet(RectList &out)
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
			double confidence = 0.0;
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

bool DnnOnnx::detectMobilenet2(RectList &out)
{
	out.clear();
	bool result = false;

	std::vector<cv::Mat> output;
	_net.forward(output, _layerNames);

	std::vector<std::pair<float,int>> probs;
	softmax(output, probs);
	std::sort(probs.begin(), probs.end(), std::greater<std::pair<float,int>>());
	result = true;

	static std::vector<int> outLayers = _net.getUnconnectedOutLayers();
	static std::string outLayerType = _net.getLayer(outLayers[0])->type;
	LOGD(outLayerType);

	for (int i = 0; i < 5; ++i)
	{
		LOGD(probs[i].first << ": " << probs[i].second << ", " << _classes[probs[i].second - 1]);
	}

	return result;
}

bool DnnOnnx::detectYolov5(RectList &out)
{
	out.clear();
	bool result = false;

	std::vector<cv::Mat> output;
	_net.forward(output, _layerNames);

	// Initialize vectors to hold respective outputs while unwrapping detections.
	std::vector<int> classIds;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	// Resizing factor.
	float x_factor = _width / _modelWidth;
	float y_factor = _height / _modelHeight;

	float *data = (float *)output[0].data;
	const int dimensions = 85;
	const int rows = 25200;

	// Iterate through 25200 detections.
	for (int i = 0; i < rows; ++i)
	{
		float confidence = data[4];

		// Discard bad detections and continue.
		if (confidence >= _confThreshold)
		{
			float *classes_scores = data + 5;

			// Create a 1x85 Mat and store class scores of 80 classes.
			cv::Mat scores(1, 80, CV_32FC1, classes_scores);

			// Perform minMaxLoc and acquire index of best class score.
			cv::Point class_id;
			double max_class_score;
			minMaxLoc(scores, 0, &max_class_score, 0, &class_id);

			// Continue if the class score is above the threshold.
			if (max_class_score > _confThreshold)
			{
				// Store class ID and confidence in the pre-defined respective vectors.
				confidences.push_back(confidence);
				classIds.push_back(class_id.x);

				// Center.
				float cx = data[0];
				float cy = data[1];

				// Box dimension.
				float w = data[2];
				float h = data[3];

				// Bounding box coordinates.
				int left = int((cx - 0.5 * w) * x_factor);
				int top = int((cy - 0.5 * h) * y_factor);
				int width = int(w * x_factor);
				int height = int(h * y_factor);

				// Store good detections in the boxes vector.
				boxes.push_back(cv::Rect(left, top, width, height));
				result = true;
			}
		}

		// Jump to the next column.
		data += dimensions;
	}

	// Perform Non Maximum Suppression and draw predictions.
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, _confThreshold, _nmsThreshold, indices);
	for (int i = 0; i < indices.size(); i++)
	{
		int idx = indices[i];
		cv::Rect box = boxes[idx];

		int left = box.x;
		int top = box.y;
		int width = box.width;
		int height = box.height;
		out.push_back(cv::Rect(left, top, width, height));
	}

	return result;
}

bool DnnOnnx::detectYolox(RectList &out)
{
	out.clear();
	bool result = false;

	std::vector<cv::Mat> output;
	_net.forward(output, _layerNames);

	if (output[0].dims == 3)
	{
		const int num_proposal = output[0].size[1];
		output[0] = output[0].reshape(0, num_proposal);
	}

	std::vector<int> classIds;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	float x_factor = _width / _modelWidth;
	float y_factor = _height / _modelHeight;
	float *data = (float *)output[0].data;

	int row = 0;
	int dim = _classes.size() + 5;

	for (int n = 0; n < 3; n++)
	{
		const int num_grid_x = (_modelHeight / _stride[n]);
		const int num_grid_y = (_modelWidth / _stride[n]);

		for (int i = 0; i < num_grid_y; i++)
		{
			for (int j = 0; j < num_grid_x; j++)
			{
				float boxScore = data[4];
				cv::Mat scores = output[0].row(row).colRange(5, output[0].cols);
				cv::Point positionOfMax;
				double confidence = 0.0;
				minMaxLoc(scores, 0, &confidence, 0, &positionOfMax);

				int classId = positionOfMax.x;
				float classScore = data[5 + classId];
				float boxProb = boxScore * classScore;

				if (boxProb > _confThreshold)
				{
					//LOGD("data[0] " << data[0] << ", data[1] " << data[1] << ", data[2] " << data[2] << ", data[3] " << data[3]);

					float cx = (data[0] + j) * _stride[n];
					float cy = (data[1] + i) * _stride[n];
					float width = exp(data[2]) * _stride[n];
					float height = exp(data[3]) * _stride[n];

					//cx = cx - width / 2;
					//cy = cy - height / 2;
					//width = cx + width / 2;
					//height = cy + height / 2;

					float x = cx - width * 0.5f;
					float y = cy - height * 0.5f;

					classIds.push_back(classId);
					confidences.push_back(classId);
					boxes.push_back(cv::Rect(x, y, width, height));
					result = true;
				}

				data += dim;
				row++;
			}
		}
	}

	float scale = 1.0;
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, _confThreshold, _nmsThreshold, indices);
	for (int i = 0; i < indices.size(); i++)
	{
		int idx = indices[i];
		cv::Rect box = boxes[idx];

		// adjust offset to original unpadded
		float x0 = (box.x) / scale;
		float y0 = (box.y) / scale;
		float x1 = (box.x + box.width) / scale;
		float y1 = (box.y + box.height) / scale;

		// clip
		x0 = std::max(std::min(x0, (float)(_width - 1)), 0.f);
		y0 = std::max(std::min(y0, (float)(_height - 1)), 0.f);
		x1 = std::max(std::min(x1, (float)(_width - 1)), 0.f);
		y1 = std::max(std::min(y1, (float)(_height - 1)), 0.f);

		out.push_back(cv::Rect(x0, y0, x1, y1));
	}

	return result;
}

bool DnnOnnx::detectNanodet(RectList &out)
{
	out.clear();
	bool result = false;

	std::vector<cv::Mat> output;
	_net.forward(output, _layerNames);
	result = true;

	return result;
}

void DnnOnnx::softmax(const std::vector<cv::Mat> &in, std::vector<std::pair<float,int>> &out) const
{
	const cv::Mat &mat = in[0];
	float m = -INFINITY;

	for (int i = 0; i < mat.cols; ++i)
	{
		if (m < mat.at<float>(0, i))
			m = mat.at<float>(0, i);
	}

	float sum = 0.0f;
	for (int i = 0; i < mat.cols; ++i)
		sum += expf(mat.at<float>(0, i) - m);

	float offset = m + logf(sum);
	for (int i = 0; i < mat.cols; ++i)
		out.push_back(std::make_pair(expf(mat.at<float>(0, i) - offset), i));
}

void DnnOnnx::loadClasses(const std::string &fileName, std::vector<std::string> &names) const
{
	names.clear();
	std::ifstream ifs(fileName);
	if (!ifs.is_open())
		return;

	string line;
	while (std::getline(ifs, line))
		names.push_back(line);

	ifs.close();
}

}
