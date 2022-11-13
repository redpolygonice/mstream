#ifndef NNETWORK_H
#define NNETWORK_H

#include "common/types.h"
#include "innetwork.h"

#include <opencv2/dnn.hpp>

// DNN network implementation
class DNNetwork : public INNetwork
{
private:
	cv::dnn::Net _net;
	std::vector<cv::String> _layerNames;

	int _width;
	int _height;

public:
	DNNetwork(NnType type);
	virtual ~DNNetwork();

public:
	static NNetworkPtr create(NnType type) { return std::make_shared<DNNetwork>(type); }
	bool init(const string &model, const string &cfg, void *params = nullptr) override;
	bool setInput(const MatPtr &frame) override;
	bool detect(RectList &out) override;

private:
	std::vector<cv::String> getOutputsNames();
};

#endif // NNETWORK_H
