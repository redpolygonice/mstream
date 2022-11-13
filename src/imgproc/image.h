#ifndef IMAGE_H
#define IMAGE_H

#include "common/types.h"
#include <FreeImage.h>

/** The image object */
class Image
{
private:
	FIBITMAP *_dib;
	FREE_IMAGE_FORMAT _fif;
	int _width;
	int _height;
	unsigned _pitch;
	int _bpp;
	int _bytes;
	bool _hasAlpha;

public:
	Image();
	~Image();
	Image(const Image &other) = delete;
	void operator= (const Image &other) = delete;

	bool load(const string &fileName);
	void unload();
	static bool save(const string &fileName, unsigned char *data, int width, int height, int pitch, unsigned bpp, FREE_IMAGE_FORMAT format);

	unsigned char *data() const { return FreeImage_GetBits(_dib); }
	FREE_IMAGE_FORMAT format() const { return _fif; }
	int width() const { return _width; }
	int height() const { return _height; }
	int pitch() const { return _pitch; }
	int bpp() const { return _bpp; }
	int bytes() const { return _bytes; }
	int channels() const { return _bpp / 8; }
	bool hasAlpha() const { return _hasAlpha; }
};

#endif // IMAGE_H
