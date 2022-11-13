#include "imageprocessor.h"
#include "common/log.h"

ImageProcessor::ImageProcessor()
	: _red(0)
	, _green(0)
	, _blue(0)
{
}

bool ImageProcessor::load()
{
	_red = Config::instance()->imgRed();
	_green = Config::instance()->imgGreen();
	_blue = Config::instance()->imgBlue();
	_type = Config::instance()->imgprocType();

	if (_red == 0 && _green == 0 && _blue == 0)
		return false;

	return true;
}

void ImageProcessor::process(const MatPtr &frame)
{
	if (_type == ImgprocType::Saturation)
		Imgproc::saturation1(frame->data, frame->data, frame->cols, frame->rows, frame->channels(), _blue, _green, _red);
	else if (_type == ImgprocType::Binary)
		Imgproc::grayscale2(frame->data, frame->data, frame->cols, frame->rows, frame->channels());
}
