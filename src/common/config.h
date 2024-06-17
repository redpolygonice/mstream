#ifndef SETTINGS_H
#define SETTINGS_H

#include "types.h"

static const char kConfigFile[] = "config.json";

enum class InputType
{
	Camera,
	GstCamera,
	V4lCamera,
	LibCamera,
	File
};

enum class OutputType
{
	Null,
	Rtp,
	Rtsp,
	Tcp,
	Hls,
	File
};

enum class ProcType
{
	Unknown,
	Rpi3,
	Rpi4,
	Rk
};

enum class ImgprocType
{
	Binary,
	Saturation
};

enum class NnType
{
	DnnDarknet,
	DnnCaffe,
	DnnTensorflow,
	DnnTorch,
	DnnONNX,
	Rknn,
	Khadas,
	Tengine,
	Tengine8bit,
	TengineTimvx
};

// Application settings
class Config
{
private:
	// Common
	bool _detect;
	bool _async;
	ProcType _procType;

	// Imgproc
	ImgprocType _imgprocType;
	bool _imgproc;
	int _imgRed;
	int _imgGreen;
	int _imgBlue;

	// NN
	NnType _nnType;
	string _modelPath;
	string _cfgPath;
	int _backend;
	int _target;
	int _modelWidth;
	int _modelHeight;
	int _modelChannels;
	float _confThreshold;
	float _nmsThreshold;
	int _numClasses;
	string _classesFile;
	StringList _classes;

	// Input
	InputType _inputType;

	//Camera
	int _dev;
	int _width;
	int _height;
	int _fps;
	string _format;

	// Input file
	string _inputFilePath;

	// Output
	bool _window;
	OutputType _outputType;

	// Output params
	int _outputWidth;
	int _outputHeight;
	int _outputFps;
	int _outputBitrate;
	int _outputGop;

	// Rtp
	string _rtpHost;
	int _rtpPort;

	// HLS
	string _hlsAddress;
	string _hlsLocation;
	string _hlsPlaylistLocation;

	// Output file
	string _outputFilePath;
	int _outputFileCodec;

public:
	Config();

public:
	bool load();

	bool detect() const { return _detect; }
	bool async() const { return _async; }
	bool imgproc() const { return _imgproc; }
	ImgprocType imgprocType() const { return _imgprocType; }
	int imgRed() const { return _imgRed; }
	int imgGreen() const { return _imgGreen; }
	int imgBlue() const { return _imgBlue; }
	ProcType procType() const { return _procType; }
	NnType nnType() const { return _nnType; }
	string modelPath() const { return _modelPath; }
	string cfgPath() const { return _cfgPath; }
	int backend() const { return _backend; }
	int target() const { return _target; }
	int modelWidth() const { return _modelWidth; }
	int modelHeight() const { return _modelHeight; }
	int modelChannels() const { return _modelChannels; }
	float confThreshold() const { return _confThreshold; }
	float nmsThreshold() const { return _nmsThreshold; }
	int numClasses() const { return _numClasses; }
	string classesFile() const { return _classesFile; }
	InputType inputType() const { return _inputType; }
	int cameraDev() const { return _dev; }
	int cameraWidth() const { return _width; }
	int cameraHeight() const { return _height; }
	int cameraFps() const { return _fps; }
	string cameraFormat() const { return _format; }
	string inputFilePath() const { return _inputFilePath; }
	bool window() const { return _window; }
	OutputType outputType() const { return _outputType; }
	int outputWidth() const { return _outputWidth; }
	int outputHeight() const { return _outputHeight; }
	int outputFps() const { return _outputFps; }
	int outputBitrate() const { return _outputBitrate; }
	int outputGop() const { return _outputGop; }
	string rtpHost() const { return _rtpHost; }
	int rtpPort() const { return _rtpPort; }
	string hlsAddress() const { return _hlsAddress; }
	string hlsLocation() const { return _hlsLocation; }
	string hlsPlaylistLocation() const { return _hlsPlaylistLocation; }
	string outputFilePath() const { return _outputFilePath; }
	int outputFileCodec() const { return _outputFileCodec; }
};

using ConfigPtr = std::shared_ptr<Config>;

inline ConfigPtr GetConfig()
{
	static ConfigPtr config = nullptr;
	if (config == nullptr)
		config = std::make_shared<Config>();
	return config;
}

#endif // SETTINGS_H
