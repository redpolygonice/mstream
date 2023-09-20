#ifndef LIBAVREADER_H
#define LIBAVREADER_H

#include "common/types.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

}

// LibAV video reader
class AvReader
{
private:
	AVFormatContext *_formatCtx;
	AVCodecContext *_codecCtx;
	AVFrame *_frame;
	SwsContext *_swsCtx;
	int _videoIndex;
	string _fileName;
	bool _test;

	int _width;
	int _height;
	int _fps;
	int _colordepth;
	long _bitrate;
	long _duration;

public:
	AvReader();
	virtual ~AvReader();

public:
	bool open(const string &fileName);
	string info() const;
	void close();
	bool read(uint8_t **data);

	void setTest(bool test = true) { _test = test; }
	int width() const { return _width; }
	int height() const { return _height; }
	int fps() const { return _fps; }
	int colordepth() const { return _colordepth; }
	int bitrate() const { return _bitrate; }
	int frameCount() const { return _fps * _duration; }

private:
	bool openMediaFile(const string &fileName);
	int decodePacket(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);
	bool fromYuv420ToRgba(const AVFrame *pSrcFrame, unsigned char **data);
};

#endif // LIBAVREADER_H
