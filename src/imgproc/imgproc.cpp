#include "imgproc.h"
#include "common/common.h"

#include <math.h>

void writeImage(const std::string &fileName, const ByteArray &data)
{
	FILE *fp = fopen(fileName.c_str(), "w+");
	if (fp != NULL)
	{
		fwrite(data.data(), 1, data.size(), fp);
		fflush(fp);
		fclose(fp);
	}
}

bool Imgproc::loadImage(const std::string &fileName, Image &image)
{
	image.unload();

	if (!isFileExists(fileName))
	{
		std::cout << "File \"" << fileName << "\" does not exist!" << std::endl;
		return false;
	}

	if (!image.load(fileName))
	{
		std::cout << "Can't load image!" << std::endl;
		return false;
	}

	return true;
}

void Imgproc::grayscale(const uint8_t *in, uint8_t *out, int width, int height, int channels)
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			int index = i * height + j;
			int bgr = index * channels;
			unsigned char b = in[bgr + 0];
			unsigned char g = in[bgr + 1];
			unsigned char r = in[bgr + 2];
			out[index] = b * 0.114f + r * 0.299f + g * 0.587f;
		}
	}
}

void Imgproc::grayscale2(const uint8_t *in, uint8_t *out, int width, int height, int channels)
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			int index = (i * height + j) * channels;
			uint8_t data = out[index + 0] * 0.114f + out[index + 1] * 0.587f +  out[index + 2] * 0.299f;
			out[index + 0] = data;
			out[index + 1] = data;
			out[index + 2] = data;
		}
	}
}

void Imgproc::grayscale(const std::string &fileName)
{
	Image image;
	if (!loadImage(fileName, image))
		return;

	string outFile = fileName + "-gray" + getFileExt(fileName);
	ByteArray srcArray(image.data(), image.data() + image.bytes());
	ByteArray destArray(srcArray.size(), 0);

	int width = image.width();
	int height = image.height();
	int channels = image.channels();

	grayscale(srcArray.data(), destArray.data(), width, height, channels);
	Image::save(outFile, destArray.data(), width, height, width, 8, image.format());
}

void Imgproc::brightness(const uint8_t *in, uint8_t *out, int width, int height, int channels, double alpha, int beta)
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			int index = (i * height + j) * channels;
			for (int k = 0; k < channels; ++k)
			{
				out[index + k] = saturate_cast_uchar(alpha * in[index + k] + beta);
			}
		}
	}
}

void Imgproc::brightness(const std::string &fileName, double alpha, int beta)
{
	Image image;
	if (!loadImage(fileName, image))
		return;

	string outFile = fileName + "-bright" + getFileExt(fileName);

	ByteArray srcArray(image.data(), image.data() + image.bytes());
	ByteArray destArray(srcArray.size(), 0);

	int width = image.width();
	int height = image.height();
	int channels = image.channels();

	brightness(srcArray.data(), destArray.data(), width, height, channels, alpha, beta);
	Image::save(outFile, destArray.data(), width, height, width * channels, image.bpp(), image.format());
}

void Imgproc::saturation1(const uint8_t *in, uint8_t *out, int width, int height, int channels, int blue, int green, int red)
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			int index = (i * height + j) * channels;
			out[index + 0] = saturate_cast_uchar(in[index + 0] + blue);
			out[index + 1] = saturate_cast_uchar(in[index + 1] + green);
			out[index + 2] = saturate_cast_uchar(in[index + 2] + red);
		}
	}
}

void Imgproc::saturation1(const std::string &fileName, int blue, int green, int red)
{
	Image image;
	if (!loadImage(fileName, image))
		return;

	string outFile = fileName + "-sat1" + getFileExt(fileName);

	ByteArray srcArray(image.data(), image.data() + image.bytes());
	ByteArray destArray(srcArray.size(), 0);
	image.unload();

	int width = image.width();
	int height = image.height();
	int channels = image.channels();

	saturation1(srcArray.data(), destArray.data(), width, height, channels, blue, green, red);
	Image::save(outFile, destArray.data(), width, height, width * channels, image.bpp(), image.format());
}

void Imgproc::saturation2(const uint8_t *in, uint8_t *out, int width, int height, int channels, double blue, double green, double red)
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			int index = (i * height + j) * channels;
			out[index + 0] = saturate_cast_uchar(in[index + 0] * blue);
			out[index + 1] = saturate_cast_uchar(in[index + 1] * green);
			out[index + 2] = saturate_cast_uchar(in[index + 2] * red);
		}
	}
}

void Imgproc::saturation2(const std::string &fileName, double blue, double green, double red)
{
	Image image;
	if (!loadImage(fileName, image))
		return;

	string outFile = fileName + "-sat2" + getFileExt(fileName);

	ByteArray srcArray(image.data(), image.data() + image.bytes());
	ByteArray destArray(srcArray.size(), 0);
	image.unload();

	int width = image.width();
	int height = image.height();
	int channels = image.channels();

	saturation2(srcArray.data(), destArray.data(), width, height, channels, blue, green, red);
	Image::save(outFile, destArray.data(), width, height, width * channels, image.bpp(), image.format());
}

