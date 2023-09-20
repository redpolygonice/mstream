#include "khadas.h"

namespace nn
{

Khadas::Khadas()
	: INNetwork(NnType::Khadas)
{
}

Khadas::~Khadas()
{
}

bool Khadas::init(const std::string &model, const std::string &cfg, void *params)
{
	return true;
}

bool Khadas::setInput(const MatPtr &origFrame)
{
	return true;
}

bool Khadas::detect(RectList &out)
{
	return true;
}

}
