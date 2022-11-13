#ifndef RKNNPOSTPROCESS_H
#define RKNNPOSTPROCESS_H

#include "common/types.h"
#include "rknn_api.h"

#define IMG_WID 416
#define IMG_HGT 416
#define GRID0 13
#define GRID1 26
#define GRID2 52
#define nclasses 80
#define nyolo 3
#define nanchor 3
#define OBJ_THRESH 0.6
#define DRAW_CLASS_THRESH 0.6
#define NMS_THRESH 0.4

typedef struct Box
{
	float x, y, w, h;
} box_t;

typedef struct Detection
{
	box_t bbox;
	int classes;
	float *prob;
	float objectness;
	int sort_class;
} detection_t;


#ifdef __cplusplus
extern "C"{
#endif
int outputs_transform(rknn_output rknn_outputs[], int net_width, int net_height, detection_t* dets);
int do_nms_sort(detection_t *dets, int total, int classes, float thresh);
void get_det_rects(RectList &out, detection_t *dets, int width, int height, int total, float thresh);
void free_detections(detection_t *dets, int n);
#ifdef __cplusplus
}
#endif

class RknnPostProcess
{
public:
	RknnPostProcess();
};

#endif // RKNNPOSTPROCESS_H
