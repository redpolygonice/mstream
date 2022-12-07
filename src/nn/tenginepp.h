#ifdef USE_TENGINE
#ifndef TENGINEPOSTPROCESS_H
#define TENGINEPOSTPROCESS_H

#include "common/types.h"
#include "innetwork.h"

struct Object
{
	cv::Rect_<float> rect;
	int label;
	float prob;
};

// Tengine post process
class TenginePostProcess
{
private:
	NNetworkPtr _parent;

public:
	TenginePostProcess();
	~TenginePostProcess();

public:
	void setParent(const NNetworkPtr &parent) { _parent = parent; }
	void sort_descent_inplace(std::vector<Object>& objects);
	void nms_sorted_bboxes(const std::vector<Object>& objects, std::vector<int>& picked, float nms_threshold);
	void generate_proposals(int stride, const float* feat, float prob_threshold, std::vector<Object>& objects);
};

#endif // TENGINEPOSTPROCESS_H
#endif
