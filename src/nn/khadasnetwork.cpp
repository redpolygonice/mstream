#include "khadasnetwork.h"

KhadasNetwork::KhadasNetwork()
	: INNetwork(NnType::Khadas)
{
}

KhadasNetwork::~KhadasNetwork()
{
}

bool KhadasNetwork::init(const std::string &model, const std::string &cfg, void *params)
{
	return true;
}

bool KhadasNetwork::setInput(const MatPtr &origFrame)
{
	return true;
}

bool KhadasNetwork::detect(RectList &out)
{
	return true;
}
