#ifndef LIBAVWRITER_H
#define LIBAVWRITER_H

#include "common/types.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

// LibAV video writer
class AvWriter
{
private:
	std::atomic_bool _active;
	AVFormatContext *_formatCtx;
	AVCodecContext *_codecCtx;
	AVStream *_stream;
	AVFrame *_frame;
	SwsContext *_swsCtx;
	int _codec;

	const int _ptsCoeff = 1000;
	int _linesize[1];
	AVPixelFormat _format;

	uint64_t _totalSize;
	long _lastPts;
	long _timeBase;

	int _width;
	int _height;
	int _fps;
	int _colordepth;
	long _bitrate;
	int _gop;

public:
	AvWriter();
	virtual ~AvWriter();

public:
	bool open(const string &fileName, int codec = 0);
	void close();
	void writeFrame(const uint8_t *data);
	uint64_t size() const { return _totalSize; }
	string info() { return string(); }

	void setGop(int gop) { _gop = gop; }
	void setCodec(int codec) { _codec = codec; }
	void setFormat(AVPixelFormat format) { _format = format; }
	void setWidth(int width) { _width = width; }
	void setHeight(int height) { _height = height; }
	void setFps(int fps) { _fps = fps; }
	void setColorDepth(int colorDepth) { _colordepth = colorDepth; }
	void setBitrate(long bitrate) { _bitrate = bitrate; }
	void setTimebase(long timeBase) { _timeBase = timeBase; }

private:
	bool openFile(const string &fileName);
	void closeFile();
};

#endif
