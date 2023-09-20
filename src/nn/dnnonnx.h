#ifndef DNNONNX_H
#define DNNONNX_H

#include "common/types.h"
#include "innetwork.h"

#include <opencv2/dnn.hpp>

namespace nn
{

// DNN network implementation
class DnnOnnx : public INNetwork
{
public:
	enum class DnnType
	{
		Darknet,
		Mobilenet2,
		Yolov5,
		Yolox,
		Nanodet
	};

private:
	DnnType _dnnType;
	cv::dnn::Net _net;
	std::vector<cv::String> _layerNames;

	int _width;
	int _height;

	MatPtr _input;
	const int _stride[3] = { 8, 16, 32 };
	const float _mean[3] = { 0.485, 0.456, 0.406 };
	const float _std[3] = { 0.229, 0.224, 0.225 };

public:
	DnnOnnx(NnType type);
	virtual ~DnnOnnx();

public:
	static NNetworkPtr create(NnType type) { return std::make_shared<DnnOnnx>(type); }
	bool init(const string &model, const string &cfg, void *params = nullptr) override;
	bool setInput(const MatPtr &frame) override;
	bool detect(RectList &out) override;

private:
	bool detectDarknet(RectList &out);
	bool detectMobilenet2(RectList &out);
	bool detectYolov5(RectList &out);
	bool detectYolox(RectList &out);
	bool detectNanodet(RectList &out);
	void softmax(const std::vector<cv::Mat> &in, std::vector<std::pair<float,int>> &out) const;
	void loadClasses(const string &fileName, std::vector<string> &names) const;
};

}

#endif // DNNONNX_H
