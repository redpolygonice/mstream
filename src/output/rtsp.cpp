#include "rtsp.h"
#include "common/log.h"
#include "common/config.h"

namespace output
{

Rtsp::Rtsp()
	: _active(false)
	, _ready(false)
	, _loop(nullptr)
	, _server(nullptr)
	, _mounts(nullptr)
	, _factory(nullptr)
	, _pts(0)
	, _frame(nullptr)
{
}

Rtsp::~Rtsp()
{
	close();
}

bool Rtsp::open()
{
	_thread = std::thread([this]() { run(); });
	return true;
}

void Rtsp::close()
{
	g_main_loop_quit(_loop);
	g_main_loop_unref(_loop);
	_pts = 0;
	_data.clear();

	if (_thread.joinable())
		_thread.join();
}

bool Rtsp::write(const MatPtr &frame)
{
	_data.push(frame);
	return true;
}

void Rtsp::run()
{
	gst_init(nullptr, nullptr);
	gst_debug_set_default_threshold(GST_LEVEL_WARNING);

	_loop = g_main_loop_new(NULL, FALSE);
	if (_loop == nullptr)
	{
		LOGE("[Rtsp] Can't create main loop!");
		return;
	}

	_server = gst_rtsp_server_new();
	if (_server == nullptr)
	{
		LOGE("[Rtsp] Can't create rtsp server!");
		return;
	}

	_mounts = gst_rtsp_server_get_mount_points(_server);
	if (_mounts == nullptr)
	{
		LOGE("[Rtsp] Can't get mount points!");
		return;
	}

	_factory = gst_rtsp_media_factory_new();
	if (_factory == nullptr)
	{
		LOGE("[Rtsp] Can't create media factory!");
		return;
	}

	string launchCmd = "( appsrc name=appsrc ! videoconvert ! ";
	if (GetConfig()->procType() == ProcType::Rpi3)
		launchCmd += "omxh264enc control-rate=2 target-bitrate=2000000 ! ";
	else if (GetConfig()->procType() == ProcType::Rk)
		launchCmd += "mpph264enc bps=2000000 ! ";
	else
		launchCmd += "x264enc speed-preset=superfast tune=zerolatency cabac=false byte-stream=true threads=4 ! ";
	launchCmd += "video/x-h264, profile=baseline ! ";
	launchCmd += "rtph264pay name=pay0 pt=96 )";

	gst_rtsp_media_factory_set_launch(_factory, launchCmd.c_str());
	g_signal_connect(_factory, "media-configure", (GCallback)media_configure, this);
	gst_rtsp_mount_points_add_factory(_mounts, "/camera", _factory);
	g_object_unref(_mounts);

	gst_rtsp_server_attach(_server, nullptr);
	g_print("Stream ready at rtsp://127.0.0.1:8554/camera\n");
	g_main_loop_run(_loop);
}

void Rtsp::media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer data)
{
	Rtsp *pipe = static_cast<Rtsp *>(data);
	if (pipe == nullptr)
		return;

	GstElement *element = gst_rtsp_media_get_element(media);
	GstElement *appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "appsrc");

	gst_util_set_object_arg(G_OBJECT(appsrc), "format", "time");
	GstCaps *caps = gst_caps_new_simple ("video/x-raw",
										 "format", G_TYPE_STRING, "BGR",
										 "width", G_TYPE_INT, GetConfig()->outputWidth(),
										 "height", G_TYPE_INT, GetConfig()->outputHeight(),
										 "framerate", GST_TYPE_FRACTION, GetConfig()->outputFps(), 1,
										 nullptr);
	g_object_set(G_OBJECT(appsrc), "caps", caps, nullptr);
	gst_caps_unref(caps);

	pipe->_ready = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	g_signal_connect(appsrc, "need-data", (GCallback)need_data, data);
	gst_object_unref(appsrc);
	gst_object_unref(element);
}

void Rtsp::need_data(GstElement *appsrc, guint unused, gpointer data)
{
	GstFlowReturn ret = GST_FLOW_OK;
	Rtsp *pipe = static_cast<Rtsp *>(data);
	if (pipe == nullptr)
		return;

	MatPtr frame = pipe->getNextFrame();
	if (frame == nullptr)
	{
		LOGE("[Rtsp] Need data error - no frames!");
		return;
	}

	guint size = frame->cols * frame->rows * frame->channels();
	GstBuffer *buffer = gst_buffer_new_and_alloc(size);
	gst_buffer_fill(buffer, 0, frame->data, size);

	GST_BUFFER_FLAG_SET(buffer, GST_BUFFER_FLAG_DROPPABLE);
	GST_BUFFER_PTS(buffer) = pipe->_pts;
	GST_BUFFER_DTS(buffer) = pipe->_pts;
	GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, GetConfig()->outputFps());
	pipe->_pts += GST_BUFFER_DURATION(buffer);

	g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
	gst_buffer_unref(buffer);

	if (ret != GST_FLOW_OK)
	{
		LOGE("[Rtsp] push buffer error!");
		return;
	}
}

MatPtr Rtsp::getNextFrame()
{
	MatPtr frame = _data.pop();
	if (frame != nullptr)
		_frame = frame;
	else if (frame == nullptr)
		return _frame;
	return frame;
}

}
