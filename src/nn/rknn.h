#ifdef USE_RKNN
#ifndef RKNN_H
#define RKNN_H

#include "common/types.h"
#include "innetwork.h"

#include <rknn_api.h>

class Rknn : public INNetwork
{
private:
	rknn_context _ctx;
	rknn_input_output_num _io_num;
	rknn_tensor_attr *_in_attr;
	rknn_tensor_attr *_out_attr;

	int _width;
	int _height;

public:
	Rknn();
	virtual ~Rknn();

public:
	static NNetworkPtr create() { return std::make_shared<Rknn>(); }
	bool init(const string &model, const string &cfg, void *params = nullptr) override;
	bool setInput(const MatPtr &origFrame) override;
	bool detect(RectList &out) override;
};

#endif // RKNN_H
#endif
