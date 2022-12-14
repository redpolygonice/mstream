#include "hlsoutput.h"
#include "common/log.h"
#include "common/config.h"
#include "common/common.h"

#include <unistd.h>

HlsOutput::HlsOutput()
	: _active(false)
{
}

HlsOutput::~HlsOutput()
{
}

bool HlsOutput::open()
{
	if (!startHttpServer())
	{
		LOGE("HlsOutput startHttpServer error!");
		return false;
	}

	string gstString = "appsrc ! videoconvert ! videoscale ! video/x-raw";
	gstString += ",width=" + std::to_string(Config::instance()->outputWidth());
	gstString += ",height=" + std::to_string(Config::instance()->outputHeight());
	gstString += ",framerate=" + std::to_string(Config::instance()->outputFps()) + "/1";
	if (Config::instance()->procType() == ProcType::Unknown)
		gstString += " ! x264enc tune=zerolatency ! mpegtsmux !";
	else if (Config::instance()->procType() == ProcType::Rk)
		gstString += " ! mpph264enc ! mpegtsmux !";
	gstString += " hlssink playlist-root=http://" + Config::instance()->hlsAddress();
	gstString += " location=" + Config::instance()->hlsLocation();
	gstString += " playlist-location=" + Config::instance()->hlsPlaylistLocation();
	gstString += " target-duration=5 max-files=10";

	LOG("HLS string: " << gstString);

	if (!_writer.open(gstString, 0, Config::instance()->outputFps(),  cv::Size(Config::instance()->outputWidth(), Config::instance()->outputHeight())))
	{
		LOGE("Can't open writer!");
		return false;
	}

	_active = true;
	return true;
}

void HlsOutput::close()
{
	_active = false;
	_writer.release();
	stopHttpServer();
}

bool HlsOutput::write(const MatPtr &frame)
{
	_writer.write(*frame);
	return true;
}

bool HlsOutput::startHttpServer()
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

	sleep(50);
	chdir(curDir.c_str());
	return true;
}

void HlsOutput::stopHttpServer()
{
	system("pkill python3");
}

string HlsOutput::getHlsDir()
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
