#ifdef USE_TENGINE
#include "tenginepp.h"

namespace nn
{

static inline float sigmoid(float x)
{
	return static_cast<float>(1.f / (1.f + exp(-x)));
}

static inline float intersection_area(const Object& a, const Object& b)
{
	cv::Rect_<float> inter = a.rect & b.rect;
	return inter.area();
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right)
{
	int i = left;
	int j = right;
	float p = faceobjects[(left + right) / 2].prob;

	while (i <= j)
	{
		while (faceobjects[i].prob > p)
			i++;

		while (faceobjects[j].prob < p)
			j--;

		if (i <= j)
		{
			// swap
			std::swap(faceobjects[i], faceobjects[j]);

			i++;
			j--;
		}
	}

#pragma omp parallel sections
	{
#pragma omp section
		{
			if (left < j) qsort_descent_inplace(faceobjects, left, j);
		}
#pragma omp section
		{
			if (i < right) qsort_descent_inplace(faceobjects, i, right);
		}
	}
}

TenginePostProcess::TenginePostProcess()
{
}

TenginePostProcess::~TenginePostProcess()
{
}

void TenginePostProcess::sort_descent_inplace(std::vector<Object> &objects)
{
	if (objects.empty())
		return;

	qsort_descent_inplace(objects, 0, objects.size() - 1);
}

void TenginePostProcess::nms_sorted_bboxes(const std::vector<Object> &objects, std::vector<int> &picked, float nms_threshold)
{
	picked.clear();

	const int n = objects.size();

	std::vector<float> areas(n);
	for (int i = 0; i < n; i++)
	{
		areas[i] = objects[i].rect.area();
	}

	for (int i = 0; i < n; i++)
	{
		const Object& a = objects[i];

		int keep = 1;
		for (int j = 0; j < (int)picked.size(); j++)
		{
			const Object& b = objects[picked[j]];

			// intersection over union
			float inter_area = intersection_area(a, b);
			float union_area = areas[i] + areas[picked[j]] - inter_area;
			if (inter_area / union_area > nms_threshold)
				keep = 0;
		}

		if (keep)
			picked.push_back(i);
	}
}

void TenginePostProcess::generate_proposals(int stride, const float *feat, float prob_threshold, std::vector<Object> &objects)
{
	static float anchors[18] = {10, 13, 16, 30, 33, 23, 30, 61, 62, 45, 59, 119, 116, 90, 156, 198, 373, 326};

	int anchor_num = 3;
	int feat_w = _parent->modelWidth() / stride;
	int feat_h = _parent->modelHeight() / stride;
	int cls_num = _parent->numClasses();
	int anchor_group = 0;
	if (stride == 8)
		anchor_group = 1;
	if (stride == 16)
		anchor_group = 2;
	if (stride == 32)
		anchor_group = 3;

	for (int h = 0; h <= feat_h - 1; h++)
	{
		for (int w = 0; w <= feat_w - 1; w++)
		{
			for (int anchor = 0; anchor <= anchor_num - 1; anchor++)
			{
				int class_index = 0;
				float class_score = -FLT_MAX;
				int channel_size = feat_h * feat_w;
				for (int s = 0; s <= cls_num - 1; s++)
				{
					int score_index = anchor * (cls_num + 5) * channel_size + feat_w * h + w + (s + 5) * channel_size;
					float score = feat[score_index];
					if (score > class_score)
					{
						class_index = s;
						class_score = score;
					}
				}
				float box_score = feat[anchor * (cls_num + 5) * channel_size + feat_w * h + w + 4 * channel_size];
				float final_score = sigmoid(box_score) * sigmoid(class_score);
				if (final_score >= prob_threshold)
				{
					int dx_index = anchor * (cls_num + 5) * channel_size + feat_w * h + w + 0 * channel_size;
					int dy_index = anchor * (cls_num + 5) * channel_size + feat_w * h + w + 1 * channel_size;
					int dw_index = anchor * (cls_num + 5) * channel_size + feat_w * h + w + 2 * channel_size;
					int dh_index = anchor * (cls_num + 5) * channel_size + feat_w * h + w + 3 * channel_size;

					float dx = sigmoid(feat[dx_index]);
					float dy = sigmoid(feat[dy_index]);

					float dw = feat[dw_index];
					float dh = feat[dh_index];

					float anchor_w = anchors[(anchor_group - 1) * 6 + anchor * 2 + 0];
					float anchor_h = anchors[(anchor_group - 1) * 6 + anchor * 2 + 1];

					float pred_x = (w + dx) * stride;
					float pred_y = (h + dy) * stride;
					float pred_w = exp(dw) * anchor_w;
					float pred_h = exp(dh) * anchor_h;

					float x0 = (pred_x - pred_w * 0.5f);
					float y0 = (pred_y - pred_h * 0.5f);
					float x1 = (pred_x + pred_w * 0.5f);
					float y1 = (pred_y + pred_h * 0.5f);

					Object obj;
					obj.rect.x = x0;
					obj.rect.y = y0;
					obj.rect.width = x1 - x0;
					obj.rect.height = y1 - y0;
					obj.label = class_index;
					obj.prob = final_score;
					objects.push_back(obj);
				}
			}
		}
	}
}

}

#endif
