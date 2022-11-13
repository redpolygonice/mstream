#ifndef CONVERT_H
#define CONVERT_H

#include "common/types.h"
#include "image.h"

/** Algorithms for processing images */
class Imgproc
{
private:
	static bool loadImage(const string &fileName, Image &image);

public:
	static void grayscale(const uint8_t *in, uint8_t *out, int width, int height, int channels);
	static void grayscale2(const uint8_t *in, uint8_t *out, int width, int height, int channels);
	static void brightness(const uint8_t *in, uint8_t *out, int width, int height, int channels, double alpha, int beta);
	static void saturation1(const uint8_t *in, uint8_t *out, int width, int height, int channels, int blue, int green, int red);
	static void saturation2(const uint8_t *in, uint8_t *out, int width, int height, int channels, double blue, double green, double red);

	static void grayscale(const string &fileName);
	static void brightness(const string &fileName, double alpha, int beta);
	static void saturation1(const string &fileName, int blue, int green, int red);
	static void saturation2(const string &fileName, double blue, double green, double red);
};

#endif // CONVERT_H
