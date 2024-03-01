#include "hls.h"
#include "common/log.h"
#include "common/config.h"
#include "common/common.h"

#include <unistd.h>

namespace output
{

Hls::Hls()
	: _active(false)
{
}

Hls::~Hls()
{
}

bool Hls::open()
{
	if (!startHttpServer())
	{
		LOGE("HlsOutput startHttpServer error!");
		return false;
	}

	string gstString = "appsrc ! videoconvert ! videoscale ! video/x-raw";
	gstString += ",width=" + std::to_string(GetConfig()->outputWidth());
	gstString += ",height=" + std::to_string(GetConfig()->outputHeight());
	gstString += ",framerate=" + std::to_string(GetConfig()->outputFps()) + "/1";
	if (GetConfig()->procType() == ProcType::Rpi3)
		gstString += " ! omxh264enc control-rate=2 target-bitrate=2000000 ! mpegtsmux !";
	else if (GetConfig()->procType() == ProcType::Rk)
		gstString += " ! mpph264enc bps=2000000 ! mpegtsmux !";
	else
		gstString += " ! x264enc speed-preset=superfast tune=zerolatency bitrate=2000000 ! mpegtsmux !";
	gstString += " hlssink playlist-root=http://" + GetConfig()->hlsAddress();
	gstString += " location=" + GetConfig()->hlsLocation();
	gstString += " playlist-location=" + GetConfig()->hlsPlaylistLocation();
	gstString += " target-duration=5 max-files=10";

	LOG("HLS string: " << gstString);

	if (!_writer.open(gstString, 0, GetConfig()->outputFps(),  cv::Size(GetConfig()->outputWidth(), GetConfig()->outputHeight())))
	{
		LOGE("Can't open writer!");
		return false;
	}

	_active = true;
	return true;
}

void Hls::close()
{
	_active = false;
	_writer.release();
	stopHttpServer();
}

bool Hls::write(const MatPtr &frame)
{
	_writer.write(*frame);
	return true;
}

bool Hls::startHttpServer()
{
//	const char *path = "/usr/bin/python3";
//	char *const args[] = { (char*)"python3", (char*)"-m", (char*)"http.server", (char*)"8000", NULL };
//	char *const env[] = { (char*)"PWD=/home/alexey/projects/nnstream/data/hls", NULL };
//	createProcess(path, args, env);

	string hlsDir = getHlsDir();
	if (!isFileExists(hlsDir))
	{
		LOGE("Can't find HLS dir!");
		return false;
	}

	string curDir = getCurrentDir();
	LOG("HLS dir: " << hlsDir);

	_httpThread = std::thread([hlsDir]() {
		chdir(hlsDir.c_str());
		system("python3 -m http.server 8080");
	});

	sleepFor(50);
	chdir(curDir.c_str());
	return true;
}

void Hls::stopHttpServer()
{
	system("pkill python3");
}

string Hls::getHlsDir()
{
	string currentDir = getCurrentDir();
	string hlsDir = currentDir + "/data/hls";

	if (!isFileExists(hlsDir))
	{
		int index = currentDir.find_last_of("/");
		if (index == string::npos)
			return "";

		hlsDir = currentDir.substr(0, index) + "/data/hls";
		if (!isFileExists(hlsDir))
			return "";
	}

	return hlsDir;
}

}
