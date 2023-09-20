#ifndef DNN_H
#define DNN_H

#include "common/types.h"
#include "innetwork.h"

#include <opencv2/dnn.hpp>

namespace nn
{

// DNN network implementation
class Dnn : public INNetwork
{
private:
	cv::dnn::Net _net;
	std::vector<cv::String> _layerNames;

	int _width;
	int _height;

public:
	Dnn(NnType type);
	virtual ~Dnn();

public:
	static NNetworkPtr create(NnType type) { return std::make_shared<Dnn>(type); }
	bool init(const string &model, const string &cfg, void *params = nullptr) override;
	bool setInput(const MatPtr &frame) override;
	bool detect(RectList &out) override;

private:
	std::vector<cv::String> getOutputsNames();
};

}

#endif // DNN_H
