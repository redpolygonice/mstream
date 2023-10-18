#ifndef TYPES_H
#define TYPES_H

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <memory>
#include <fstream>
#include <atomic>
#include <future>

using std::string;
using std::wstring;

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <climits>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

typedef std::shared_ptr<cv::Mat> MatPtr;
typedef std::vector<cv::Rect> RectList;

#define BUFFER_SIZE 4096

// Path separator
#ifdef WIN32
#define PS "\\"
#else
#define PS "/"
#endif

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

// String list
typedef std::vector<string> StringList;
typedef std::vector<string>::iterator StringListIterator;
typedef std::vector<string>::const_iterator StringListConstIterator;
typedef std::shared_ptr<string> StringPtr;

// Byte array
typedef std::vector<unsigned char> ByteArray;
typedef std::vector<unsigned char>::iterator ByteArrayIterator;
typedef std::vector<unsigned char>::const_iterator ByteArrayConstIterator;
typedef std::shared_ptr<ByteArray> ByteArrayPtr;

// Char array
typedef std::vector<char> CharArray;
typedef std::vector<char>::iterator CharArrayIterator;
typedef std::vector<char>::const_iterator CharArrayConstIterator;

// Linux raspberrypi 5.10.92-v7l+ #1514 SMP Mon Jan 17 17:38:03 GMT 2022 armv7l GNU/Linux
// Linux Khadas 4.9.241 #8 SMP PREEMPT Sat Jan 8 09:27:25 CST 2022 aarch64 aarch64 aarch64 GNU/Linux
// Linux linaro-alip 4.4.154-90-rockchip-ga14f6502e045 #22 SMP Tue Jul 30 10:32:28 UTC 2019 aarch64 GNU/Linux
// Linux linaro-alip 4.4.194 #1 SMP Fri Dec 10 08:01:40 UTC 2021 aarch64 GNU/Linux

enum class DeviceName
{
	Unknown,
	Raspberry,
	KhadasVIM3,
	RockPi4,
	AsusTinkerBoard
};

typedef std::vector<float> FloatArray;
typedef std::vector<double> DoubleArray;

typedef unsigned char uchar;
//typedef char schar;
typedef unsigned short ushort;
inline uchar saturate_cast_uchar(ushort v)       { return (uchar)std::min((unsigned)v, (unsigned)UCHAR_MAX); }
inline uchar saturate_cast_uchar(int v)          { return (uchar)((unsigned)v <= UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0); }
inline uchar saturate_cast_uchar(short v)        { return saturate_cast_uchar((int)v); }
inline uchar saturate_cast_uchar(unsigned v)     { return (uchar)std::min(v, (unsigned)UCHAR_MAX); }
inline uchar saturate_cast_uchar(float v)        { int iv = round(v); return saturate_cast_uchar(iv); }
inline uchar saturate_cast_uchar(double v)       { int iv = round(v); return saturate_cast_uchar(iv); }
inline uchar saturate_cast_uchar(int64_t v)        { return (uchar)((uint64_t)v <= (uint64_t)UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0); }
inline uchar saturate_cast_uchar(uint64_t v)       { return (uchar)std::min(v, (uint64_t)UCHAR_MAX); }

#endif // TYPES_H
