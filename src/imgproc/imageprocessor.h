#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "common/types.h"
#include "imgproc.h"
#include "common/config.h"

// Image processor
class ImageProcessor
{
private:
	int _red;
	int _green;
	int _blue;
	ImgprocType _type;

public:
	ImageProcessor();

public:
	bool load();
	void process(const MatPtr &frame);
};

#endif // IMAGEPROCESSOR_H
