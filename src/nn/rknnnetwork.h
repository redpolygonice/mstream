#ifndef RKNNNETWORK_H
#define RKNNNETWORK_H

#include "common/types.h"
#include "innetwork.h"

#include <rknn_api.h>

class RknnNetwork : public INNetwork
{
private:
	rknn_context _ctx;
	rknn_input_output_num _io_num;
	rknn_tensor_attr *_in_attr;
	rknn_tensor_attr *_out_attr;

	int _width;
	int _height;

public:
	RknnNetwork();
	virtual ~RknnNetwork();

public:
	static NNetworkPtr create() { return std::make_shared<RknnNetwork>(); }
	bool init(const string &model, const string &cfg, void *params = nullptr) override;
	bool setInput(const MatPtr &origFrame) override;
	bool detect(RectList &out) override;
};

#endif // RKNNNETWORK_H
