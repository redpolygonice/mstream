#include "image.h"

Image::Image()
:_dib(nullptr)
,_fif(FIF_UNKNOWN)
,_width(0)
,_height(0)
,_pitch(0)
,_bpp(0)
,_bytes(0)
,_hasAlpha(false)
{
}

Image::~Image()
{
	unload();
}

bool Image::load(const string &fileName)
{
	_fif = FreeImage_GetFileType(fileName.c_str(), 0);

	if (_fif == FIF_UNKNOWN)
		_fif = FreeImage_GetFIFFromFilename(fileName.c_str());

	if (_fif == FIF_UNKNOWN)
		return false;

	if (FreeImage_FIFSupportsReading(_fif))
	{
		_dib = FreeImage_Load(_fif, fileName.c_str());
		if (!_dib)
			return false;
	}

	_pitch = FreeImage_GetPitch(_dib);
	_bpp = FreeImage_GetBPP(_dib);
	_width = FreeImage_GetWidth(_dib);
	_height = FreeImage_GetHeight(_dib);
	_bytes = FreeImage_GetDIBSize(_dib);
	if (FreeImage_GetColorType(_dib) == FIC_RGBALPHA)
		_hasAlpha = true;

	return true;
}

void Image::unload()
{
	if (_dib)
	{
		FreeImage_Unload(_dib);
		_dib = nullptr;
	}
}

bool Image::save(const std::string &fileName, unsigned char *data, int width, int height, int pitch, unsigned bpp, FREE_IMAGE_FORMAT format)
{
	FIBITMAP *dib = FreeImage_ConvertFromRawBits(data, width, height, pitch, bpp, 0xFF, 0xFF, 0xFF, false);
	if (FreeImage_Save(format, dib, fileName.c_str()))
		return true;
	return false;
}
