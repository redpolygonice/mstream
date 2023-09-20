#ifdef USE_TENGINE
#ifndef TENGINE_H
#define TENGINE_H

#include "common/types.h"
#include "innetwork.h"
#include "tenginepp.h"

#include <tengine/c_api.h>

namespace nn
{

// Tengine neural network
class Tengine : public INNetwork, public std::enable_shared_from_this<Tengine>
{
private:
	bool _opencl;

	graph_t _graph;
	tensor_t _inputTensor;
	TenginePostProcess _postProcess;

	int _width;
	int _height;
	std::vector<float> _inputData;

	const float mean[3] = { 0, 0, 0 };
	const float scale[3] = { 0.003921, 0.003921, 0.003921 };
	const int threads = 1;
	const float anchors[18] = { 10, 13, 16, 30, 33, 23, 30, 61, 62, 45, 59, 119, 116, 90, 156, 198, 373, 326 };

public:
	Tengine();
	virtual ~Tengine();

public:
	static NNetworkPtr create() { return std::make_shared<Tengine>(); }
	bool init(const string &model, const string &cfg, void *params = nullptr) override;
	bool setInput(const MatPtr &frame) override;
	bool detect(RectList &out) override;

private:
	void postProcess(RectList &out);
};

}

#endif // TENGINE_H
#endif
