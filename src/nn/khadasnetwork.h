#ifndef KHADASNETWORK_H
#define KHADASNETWORK_H

#include "common/types.h"
#include "innetwork.h"

// Khadas neural network
class KhadasNetwork : public INNetwork
{
public:
	KhadasNetwork();
	virtual ~KhadasNetwork();

public:
	static NNetworkPtr create() { return std::make_shared<KhadasNetwork>(); }
	bool init(const string &model, const string &cfg, void *params = nullptr) override;
	bool setInput(const MatPtr &origFrame) override;
	bool detect(RectList &out) override;
};

#endif // KHADASNETWORK_H
