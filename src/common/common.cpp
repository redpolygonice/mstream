#include "common.h"
#include "log.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <libgen.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>

string i2s(int number)
{
	char szNumber[20];
	sprintf(szNumber, "%d", number);
	return szNumber;
}

int s2i(const string &text)
{
	return atoi(text.c_str());
}

string currentTime()
{
	std::time_t time = std::time(NULL);
	char timeStr[50];
	std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", std::localtime(&time));
	return timeStr;
}

std::string currentTimeMs()
{
	static std::mutex timeMutex;
	std::lock_guard<std::mutex> lock(timeMutex);

	char timeStr[50];
	struct timeval tv;
	gettimeofday(&tv, NULL);
	std::time_t now = tv.tv_sec;
	struct tm *tm = std::localtime(&now);

	if (tm == nullptr)
		return currentTime();

	sprintf(timeStr, "%04d-%02d-%02d_%02d-%02d-%02d.%03d",
			tm->tm_year + 1900,
			tm->tm_mon + 1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec,
			static_cast<int>(tv.tv_usec / 1000));

	return timeStr;
}

int64_t timestamp()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t timestamp_micro()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool isFileExists(const std::string &fileName)
{
	struct stat st;
	if (stat(fileName.c_str(), &st) == -1)
		return false;

	return true;
}

string getFileExt(const string &fileName)
{
	int index = fileName.rfind(".");
	if (index == string::npos)
		return "";

	return fileName.substr(index);
}

string getStdout(string cmd)
{
	string data;
	const int max_buffer = 4096;
	char buffer[max_buffer];
	cmd.append(" 2>&1");

	FILE *fp = popen(cmd.c_str(), "r");
	if (fp)
	{
		while (!feof(fp))
		{
			if (fgets(buffer, max_buffer, fp) != NULL)
				data.append(buffer);
		}

		pclose(fp);
	}

	return data;
}

DeviceName detectDevice()
{
	string line = getStdout("uname -a");
	if (line.find("raspberrypi") != string::npos)
		return DeviceName::Raspberry;
	else if (line.find("Khadas") != string::npos)
		return DeviceName::KhadasVIM3;
	else if (line.find("rockchip") != string::npos)
		return DeviceName::RockPi4;
	else if (line.find("linaro-alip") != string::npos)
		return DeviceName::AsusTinkerBoard;
	return DeviceName::Unknown;
}

bool readFile(const string &fileName, uint8_t **data, size_t *size)
{
	FILE *fp = fopen(fileName.c_str(), "r");
	if (fp == nullptr)
		return false;

	fseek(fp, 0, SEEK_END);
	*size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	*data = new uint8_t[*size];
	const int buffer_size = 4096;
	uint8_t buffer[buffer_size];

	size_t totalBytes = 0;
	while (!feof(fp))
	{
		size_t bytesRead = fread(buffer, 1, buffer_size, fp);
		if (bytesRead > 0)
			memcpy(*data + totalBytes, buffer, bytesRead);
		totalBytes += bytesRead;
	}

	fclose(fp);
	return true;
}

bool writeFile(const string &fileName, const uint8_t *data, size_t size)
{
	FILE *fp = fopen(fileName.c_str(), "w+");
	if (fp == nullptr)
		return false;

	fwrite(data, 1, size, fp);
	fclose(fp);
	return true;
}

string getCurrentDir()
{
	char buffer[FILENAME_MAX];
	getcwd(buffer, FILENAME_MAX);
	return string(buffer);
}

bool createProcess(const char *path, char *const args[], char *const env[])
{
	pid_t pid = fork();
	if (pid != 0)
		return false;

	execve(path, args, env);
	return true;
}

int getVideoDevice()
{
	fs::directory_iterator begin("/dev");
	fs::directory_iterator end;

	std::vector<fs::path> files;
	std::copy_if(begin, end, std::back_inserter(files), [](const fs::path& path) {
		return path.string().find("video") != string::npos;
	});

	if (files.empty())
		return -1;

	cv::VideoCapture capture;
	for (fs::path &filePath : files)
	{
		string fileName = filePath.string();
		if (capture.open(fileName))
		{
			string dev;
			for (int i = fileName.size() - 1; i >= 0; --i)
			{
				if (std::isdigit(fileName[i], std::locale()))
					dev.push_back(fileName[i]);
				else
					break;
			}

			std::reverse(dev.begin(), dev.end());
			return s2i(dev);
		}
	}

	return -1;
}
