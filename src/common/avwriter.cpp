#include "avwriter.h"
#include "common/log.h"
#include "common/common.h"

AvWriter::AvWriter()
	: _active(false)
	, _formatCtx(nullptr)
	, _codecCtx(nullptr)
	, _stream(nullptr)
	, _frame(nullptr)
	, _swsCtx(nullptr)
	, _codec(AV_CODEC_ID_H264)
	, _format(AV_PIX_FMT_YUV420P)
	, _totalSize(0)
	, _lastPts(0)
	, _timeBase(0)
	, _width(1920)
	, _height(1080)
	, _fps(30)
	, _colordepth(24)
	, _bitrate(400000)
	, _gop(12)
{
}

AvWriter::~AvWriter()
{
	close();
}

bool AvWriter::open(const std::string &fileName, int codec)
{
	_codec = codec;
	_active = true;
	return openFile(fileName);
}

void AvWriter::close()
{
	if (_active == false)
		return;

	closeFile();

	if (_frame)
	{
		av_frame_free(&_frame);
		_frame = nullptr;
	}

	if (_codecCtx)
	{
		avcodec_free_context(&_codecCtx);
		_codecCtx = nullptr;
	}

	if (_formatCtx)
	{
		avformat_free_context(_formatCtx);
		_formatCtx = nullptr;
	}

	if (_stream)
	{
		//av_free(_stream);
		_stream = nullptr;
	}

	if (_swsCtx)
	{
		sws_freeContext(_swsCtx);
		_swsCtx = nullptr;
	}

	_active = false;
}

void AvWriter::writeFrame(const uint8_t *data)
{
	// Convert RGBA to YUV420P
	sws_scale(_swsCtx, (const uint8_t * const *)&data, _linesize, 0, _codecCtx->height, _frame->data, _frame->linesize);

	// PTS
	_frame->pts = _lastPts + _timeBase;
	_lastPts = _frame->pts;

	// Send frame to encoder
	if (avcodec_send_frame(_codecCtx, _frame) < 0)
	{
		LOGW("[AvWriter] Failed to send frame!");
		return;
	}

	// Init packet
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;
	packet.flags |= AV_PKT_FLAG_KEY;

	// Receive encoded packet and write to stream
	if (avcodec_receive_packet(_codecCtx, &packet) == 0)
	{
		_totalSize += packet.size;
		av_interleaved_write_frame(_formatCtx, &packet);
		av_packet_unref(&packet);
	}
	else
	{
		LOGW("[AvWriter] Receive packet error!");
	}
}

bool AvWriter::openFile(const string &fileName)
{
	_totalSize = 0;
	_lastPts = 0;
	_timeBase = 0;

	AVOutputFormat *outFormat = av_guess_format(nullptr, fileName.c_str(), nullptr);
	if (!outFormat)
	{
		LOGE("[AvWriter] Can't create output format");
		return false;
	}

	if (_codec > 0)
		outFormat->video_codec = (AVCodecID)_codec;

	if (avformat_alloc_output_context2(&_formatCtx, outFormat, nullptr, fileName.c_str()))
	{
		LOGE("[AvWriter] Can't create output context");
		return false;
	}

	AVCodec *codec = avcodec_find_encoder(outFormat->video_codec);
	if (!codec)
	{
		LOGE("[AvWriter] Can't find codec");
		return false;
	}

	_stream = avformat_new_stream(_formatCtx, codec);
	if (!_stream)
	{
		LOGE("[AvWriter] Can't create stream");
		return false;
	}

	_codecCtx = avcodec_alloc_context3(codec);
	if (!_codecCtx)
	{
		LOGE("[AvWriter] Can't create codec context");
		return false;
	}

	_stream->codecpar->codec_id = outFormat->video_codec;
	_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	_stream->codecpar->width = _width;
	_stream->codecpar->height = _height;
	_stream->codecpar->format =  _format;
	_stream->codecpar->bit_rate = _bitrate;
	_stream->avg_frame_rate = { _fps, 1 };
	_stream->time_base = { 1, _fps * 1000 };

	if (_timeBase == 0)
		_timeBase = (1.f / av_q2d(_stream->avg_frame_rate)) / av_q2d(_stream->time_base);

	if (avcodec_parameters_to_context(_codecCtx, _stream->codecpar) < 0)
	{
		LOGE("[AvWriter] Failed to copy stream parameters");
		return false;
	}

	_codecCtx->time_base = { 1, _fps * 1000 };
	_codecCtx->max_b_frames = 1;
	_codecCtx->gop_size = _gop;
	_codecCtx->framerate = { _fps, 1 };
	_codecCtx->ticks_per_frame = 2;

	if (_stream->codecpar->codec_id == AV_CODEC_ID_H264 ||
			_stream->codecpar->codec_id == AV_CODEC_ID_H265)
		av_opt_set(_codecCtx, "preset", "ultrafast", 0);

	if (avcodec_parameters_from_context(_stream->codecpar, _codecCtx) < 0)
	{
		LOGE("AvWriter] Failed to copy codec parameters");
		return false;
	}

	if (avcodec_open2(_codecCtx, codec, NULL) < 0)
	{
		LOGE("[AvWriter] Failed to open codec");
		return false;
	}

	if (avio_open(&_formatCtx->pb, fileName.c_str(), AVIO_FLAG_WRITE) < 0)
	{
		LOGE("[AvWriter] Failed to open file");
		return false;
	}

	if (avformat_write_header(_formatCtx, NULL) < 0)
	{
		LOGE("[AvWriter] Failed to write header");
		return false;
	}

	// Create frame
	_frame = av_frame_alloc();
	_frame->format = _format;
	_frame->width = _width;
	_frame->height = _height;

	// Allocate frame fuffer
	if (av_frame_get_buffer(_frame, 0) < 0)
	{
		LOGE("[AvWriter] failed to allocated buffer for AVFrame");
		return false;
	}

	// Create SWS for scale
	_swsCtx = sws_getContext(_codecCtx->width, _codecCtx->height, AV_PIX_FMT_RGB24, _codecCtx->width, _codecCtx->height,
							 _format, SWS_BICUBIC, 0, 0, 0);

	_linesize[0] = { _width * _colordepth / 8 };
	av_dump_format(_formatCtx, 0, fileName.c_str(), 1);
	return true;
}

void AvWriter::closeFile()
{
	if (_codecCtx == nullptr || _formatCtx == nullptr)
		return;

	AVPacket packet;
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;

	while (true)
	{
		avcodec_send_frame(_codecCtx, NULL);
		if (avcodec_receive_packet(_codecCtx, &packet) == 0)
		{
			av_interleaved_write_frame(_formatCtx, &packet);
			av_packet_unref(&packet);
		}
		else
			break;
	}

	av_write_trailer(_formatCtx);
	if (avio_close(_formatCtx->pb) < 0)
	{
		LOGE("[AvWriter] Failed to close file");
	}
}

