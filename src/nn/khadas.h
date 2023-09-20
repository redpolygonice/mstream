#ifndef KHADAS_H
#define KHADAS_H

#include "common/types.h"
#include "innetwork.h"

namespace nn
{

// Khadas neural network
class Khadas : public INNetwork
{
public:
	Khadas();
	virtual ~Khadas();

public:
	static NNetworkPtr create() { return std::make_shared<Khadas>(); }
	bool init(const string &model, const string &cfg, void *params = nullptr) override;
	bool setInput(const MatPtr &origFrame) override;
	bool detect(RectList &out) override;
};

}

#endif // KHADAS_H
