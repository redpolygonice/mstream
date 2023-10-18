#ifndef RTSPOUTPUT_H
#define RTSPOUTPUT_H

#include "common/types.h"
#include "ioutput.h"
#include "common/threadsafequeue.h"

#include <gst/gst.h>
#include <rtsp-server/rtsp-server.h>

namespace output
{

// RTSP output implemetation
class Rtsp : public IOutput
{
private:
	std::atomic_bool _active;
	std::atomic_bool _ready;
	GMainLoop *_loop;
	GstRTSPServer *_server;
	GstRTSPMountPoints *_mounts;
	GstRTSPMediaFactory *_factory;
	GstClockTime _pts;
	ThreadSafeQueue<MatPtr> _data;
	std::thread _thread;
	MatPtr _frame;

private:
	static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer data);
	static void need_data(GstElement *appsrc, guint unused, gpointer data);
	MatPtr getNextFrame();
	void run();

public:
	Rtsp();
	virtual ~Rtsp();

public:
	static OutputPtr create() { return std::make_shared<Rtsp>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
	bool ready() const override { return _ready; }
};

}

#endif // RTSPOUTPUT_H
