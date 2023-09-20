#include "avreader.h"
#include "common/log.h"
#include "common/common.h"

AvReader::AvReader()
	: _formatCtx(nullptr)
	, _codecCtx(nullptr)
	, _frame(nullptr)
	, _swsCtx(nullptr)
	, _videoIndex(-1)
	, _test(false)
	, _width(0)
	, _height(0)
	, _fps(0)
	, _colordepth(0)
	, _bitrate(0)
	, _duration(0)
{
}

AvReader::~AvReader()
{
	close();
}

bool AvReader::open(const std::string &fileName)
{
	_fileName = fileName;
	if (!openMediaFile(fileName))
		return false;

	return true;
}

string AvReader::info() const
{
	if (_formatCtx == nullptr || _codecCtx == nullptr)
		return string();

	int hours, mins, secs, us;
	int64_t duration = _formatCtx->duration + 5000;
	secs  = duration / AV_TIME_BASE;
	us    = duration % AV_TIME_BASE;
	mins  = secs / 60;
	secs %= 60;
	hours = mins / 60;
	mins %= 60;
	string dur(11, 0);
	sprintf(&dur[0], "%02d:%02d:%02d.%02d", hours, mins, secs, (100 * us) / AV_TIME_BASE);

	string text = "File '" + _fileName + "' dur: " + dur + ", codec: " + std::to_string(_codecCtx->codec_id) +
			", fmt: " + std::to_string(_codecCtx->pix_fmt) + ", rs: " + std::to_string(_codecCtx->width) +
			"x" + std::to_string(_codecCtx->height) + ", fps: " + std::to_string(_formatCtx->streams[_videoIndex]->avg_frame_rate.num);
	return text;
}

void AvReader::close()
{
	_videoIndex = -1;

	if (_frame != nullptr)
	{
		av_frame_free(&_frame);
		_frame = nullptr;
	}

	if (_formatCtx != nullptr)
	{
		avformat_close_input(&_formatCtx);
		_formatCtx = nullptr;
	}

	if (_codecCtx != nullptr)
	{
		avcodec_free_context(&_codecCtx);
		_codecCtx = nullptr;
	}
}

bool AvReader::read(uint8_t **data)
{
	AVPacket *pPacket = av_packet_alloc();
	if (!pPacket)
	{
		LOG("[AvReader] failed to allocated memory for AVPacket");
		return false;
	}

	bool result = true;
	if (av_read_frame(_formatCtx, pPacket) >= 0)
	{
		// If it's the video stream
		if (pPacket->stream_index == _videoIndex)
		{
			int response = decodePacket(pPacket, _codecCtx, _frame);
			if (response == 0)
			{
				if (_frame->data[0] == nullptr)
				{
					LOGW("[AvReader] null frame after decode!");
					result = false;
				}
				else if (!fromYuv420ToRgba(_frame, data))
				{
					LOGE("[AvReader] fromYuv420ToRgba error!");
					result = false;
				}
			}
			else
			{
				LOGE("[AvReader] decodePacket error!");
				result = false;
			}
		}

		av_packet_unref(pPacket);
	}

	av_packet_free(&pPacket);
	return result;
}

bool AvReader::openMediaFile(const std::string &fileName)
{
	close();

	_formatCtx = avformat_alloc_context();
	if (!_formatCtx)
	{
		LOGE("[AvReader] avformat_alloc_context error!");
		return false;
	}

	if (avformat_open_input(&_formatCtx, fileName.c_str(), nullptr, nullptr) != 0)
	{
		LOGE("[AvReader] avformat_open_input error!");
		return false;
	}

	if (avformat_find_stream_info(_formatCtx,  nullptr) < 0)
	{
		LOGE("[AvReader] avformat_find_stream_info error!");
		return false;
	}

	AVCodec *pCodec = nullptr;
	AVCodecParameters *pCodecParameters =  nullptr;

	for (unsigned int i = 0; i < _formatCtx->nb_streams; i++)
	{
		AVCodecParameters *pLocalCodecParameters =  nullptr;
		pLocalCodecParameters = _formatCtx->streams[i]->codecpar;

		AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
		if (pLocalCodec == nullptr)
		{
			LOGE("[AvReader] ERROR unsupported codec!");
			continue;
		}

		// when the stream is a video we store its index, codec parameters and codec
		if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (_videoIndex == -1)
			{
				_videoIndex = i;
				pCodec = pLocalCodec;
				pCodecParameters = pLocalCodecParameters;
			}
		}
	}

	if (_videoIndex == -1)
	{
		LOGE("[AvReader] File " << fileName << "does not contain a video stream!");
		return false;
	}

	_codecCtx = avcodec_alloc_context3(pCodec);
	if (!_codecCtx)
	{
		LOGE("[AvReader] failed to allocated memory for AVCodecContext");
		return false;
	}

	if (avcodec_parameters_to_context(_codecCtx, pCodecParameters) < 0)
	{
		LOGE("[AvReader] failed to copy codec params to codec context");
		return false;
	}

	if (avcodec_open2(_codecCtx, pCodec, NULL) < 0)
	{
		LOGE("[AvReader] failed to open codec through avcodec_open2");
		return false;
	}

	// Create frame
	_frame = av_frame_alloc();
	if (!_frame)
	{
		LOGE("[AvReader] failed to allocated memory for AVFrame");
		return false;
	}

	_frame->format = AV_PIX_FMT_YUV420P;
	_frame->width = _codecCtx->width;
	_frame->height = _codecCtx->height;

	if (av_frame_get_buffer(_frame, 0) < 0)
	{
		LOGE("[AvReader] failed to allocated buffer for AVFrame");
		return false;
	}

	// Create SWS for scale
	_swsCtx = sws_getContext(_codecCtx->width, _codecCtx->height, AV_PIX_FMT_YUV420P, _codecCtx->width, _codecCtx->height,
							 AV_PIX_FMT_RGB24, SWS_BICUBIC, 0, 0, 0);

	_width = _codecCtx->width;
	_height = _codecCtx->height;
	_colordepth = 24;
	_fps = _formatCtx->streams[_videoIndex]->avg_frame_rate.num;
	_bitrate = _formatCtx->bit_rate;
	_duration = (_formatCtx->duration + 5000) / AV_TIME_BASE;

	av_dump_format(_formatCtx, _videoIndex, fileName.c_str(), 0);
	return true;
}

int AvReader::decodePacket(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame)
{
	// Supply raw packet data as input to a decoder
	int response = avcodec_send_packet(pCodecContext, pPacket);
	if (response < 0)
	{
		LOGW("[AvReader] Error while sending a packet to the decoder!");
		return response;
	}

	while (response >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		response = avcodec_receive_frame(pCodecContext, pFrame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
			break;
		else if (response < 0)
		{
			LOGW("[AvReader] Error while receiving a frame from the decoder!");
			return response;
		}

		if (response >= 0)
		{
			//LOGD("[AvReader] Frame: " << pCodecContext->frame_number << ", type: " << av_get_picture_type_char(pFrame->pict_type)
			//	<< ", size: " << pFrame->pkt_size << ", format: " << pFrame->format << ", pts: "
			//	<< pFrame->pts << ", key_frame: " << pFrame->key_frame << ", packet pts: " << pPacket->pts);

			return 0;
		}
	}

	return response;
}

bool AvReader::fromYuv420ToRgba(const AVFrame *pSrcFrame, unsigned char **data)
{
	AVFrame *pFrame = av_frame_alloc();
	if (!pFrame)
	{
		LOGE("[AvReader] failed to allocated memory for AVFrame");
		return false;
	}

	pFrame->format = AV_PIX_FMT_RGB24;
	pFrame->width = pSrcFrame->width;
	pFrame->height = pSrcFrame->height;

	if (av_frame_get_buffer(pFrame, 0) < 0)
	{
		LOGE("[AvReader] failed to allocated buffer for AVFrame");
		return false;
	}

	int result = sws_scale(_swsCtx, (const uint8_t * const *)&pSrcFrame->data, pSrcFrame->linesize, 0, pSrcFrame->height, pFrame->data, pFrame->linesize);
	if (result <= 0)
	{
		LOGE("[AvReader] failed to rescale AVFrame");
		return false;
	}

	*data = new unsigned char[pFrame->linesize[0] * pSrcFrame->height];
	memcpy(*data, pFrame->data[0], pFrame->linesize[0] * pSrcFrame->height);

	av_frame_free(&pFrame);
	return true;
}
