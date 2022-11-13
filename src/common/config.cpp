#include "config.h"
#include "common.h"
#include "log.h"

#include <jsoncpp/json/json.h>

Config::Config()
{
}

bool Config::load()
{
	Json::Value root;
	std::ifstream ifs(kConfigFile, std::ios::in);
	if (!ifs.is_open())
	{
		string fileName = getCurrentDir() + "/data/" + kConfigFile;
		ifs.open(fileName, std::ios::in);

		if (!ifs.is_open())
		{
			// Develop version
			fileName = string("../data/") + kConfigFile;
			ifs.open(fileName, std::ios::in);

			if (!ifs.is_open())
			{
				LOGE("[Config] Can't open settings file: " << kConfigFile);
				return false;
			}
		}
	}

	Json::CharReaderBuilder builder;
	JSONCPP_STRING error;
	if (!parseFromStream(builder, ifs, &root, &error))
	{
		LOGE("[Config] Json parse error: " << error);
		return false;
	}

	// Common
	Json::Value commonObject = root["common"];
	if (commonObject.isNull())
	{
		LOGE("[Config] There is no \"common\" object!");
		return false;
	}

	_detect = commonObject["detect"].asBool();
	_async = commonObject["async"].asBool();
	_procType = static_cast<ProcType>(commonObject["procType"].asInt());

	// Image processing
	Json::Value imgprocObject = root["imgproc"];
	if (imgprocObject.isNull())
	{
		LOGE("[Config] There is no \"imgproc\" object!");
		return false;
	}

	_imgprocType = static_cast<ImgprocType>(imgprocObject["type"].asInt());
	_imgproc = imgprocObject["active"].asBool();
	_imgRed  = imgprocObject["red"].asInt();
	_imgGreen = imgprocObject["green"].asInt();
	_imgBlue = imgprocObject["blue"].asInt();

	// Neural network
	Json::Value nnObject = root["nn"];
	if (nnObject.isNull())
	{
		LOGE("[Config] There is no \"NN\" object!");
		return false;
	}

	_nnType = static_cast<NnType>(nnObject["type"].asInt());
	_modelPath = nnObject["modelPath"].asString();
	_cfgPath = nnObject["cfgPath"].asString();
	_backend = nnObject["backend"].asInt();
	_target = nnObject["target"].asInt();
	_modelWidth = nnObject["modelWidth"].asInt();
	_modelHeight = nnObject["modelHeight"].asInt();
	_modelChannels = nnObject["modelChannels"].asInt();
	_numClasses = nnObject["numClasses"].asInt();
	_confThreshold = nnObject["confThreshold"].asFloat();
	_nmsThreshold = nnObject["nmsThreshold"].asFloat();

	// Input object
	Json::Value inputObject = root["input"];
	if (inputObject.isNull())
	{
		LOGE("[Config] There is no \"input\" object!");
		return false;
	}

	_inputType = static_cast<InputType>(inputObject["type"].asInt());

	// Camera input
	if (_inputType == InputType::Camera)
	{
		Json::Value cameraObject = inputObject["camera"];
		if (cameraObject.isNull())
		{
			LOGE("[Config] There is no \"camera\" object!");
			return false;
		}

		_dev = cameraObject["dev"].asInt();
		_width = cameraObject["width"].asInt();
		_height = cameraObject["height"].asInt();
		_fps = cameraObject["fps"].asInt();
		_format = cameraObject["format"].asString();
	}
	// File input
	else if (_inputType == InputType::File)
	{
		Json::Value inputFileObject = inputObject["file"];
		if (inputFileObject.isNull())
		{
			LOGE("[Config] There is no \"input file\" object!");
			return false;
		}

		_inputFilePath = inputFileObject["path"].asString();
	}

	// Output object
	Json::Value outputObject = root["output"];
	if (outputObject.isNull())
	{
		LOGE("[Config] There is no \"output\" object!");
		return false;
	}

	_window = outputObject["window"].asBool();
	_outputType = static_cast<OutputType>(outputObject["type"].asInt());

	// Output params
	Json::Value outputParams = outputObject["params"];
	if (outputParams.isNull())
	{
		LOGE("[Config] There is no \"output params\" object!");
		return false;
	}

	_outputWidth = outputParams["width"].asInt();
	_outputHeight = outputParams["height"].asInt();
	_outputFps = outputParams["fps"].asInt();
	_outputBitrate = outputParams["bitrate"].asInt();
	_outputGop = outputParams["gop"].asInt();

	// RTP output
	if (_outputType == OutputType::Rtp)
	{
		Json::Value rtpObject = outputObject["rtp"];
		if (rtpObject.isNull())
		{
			LOGE("[Config] There is no \"rtp\" object!");
			return false;
		}

		_rtpHost = rtpObject["host"].asString();
		_rtpPort = rtpObject["port"].asInt();
	}
	// HLS output
	else if (_outputType == OutputType::Hls)
	{
		Json::Value hlsObject = outputObject["hls"];
		if (hlsObject.isNull())
		{
			LOGE("[Config] There is no \"hls\" object!");
			return false;
		}

		_hlsAddress = hlsObject["address"].asString();
		_hlsLocation = hlsObject["location"].asString();
		_hlsPlaylistLocation = hlsObject["playlistLocation"].asString();
	}
	// File output
	else if (_outputType == OutputType::File)
	{
		Json::Value outputFileObject = outputObject["file"];
		if (outputFileObject.isNull())
		{
			LOGE("[Config] There is no \"output file\" object!");
			return false;
		}

		_outputFilePath = outputFileObject["path"].asString();
		_outputFileCodec = outputFileObject["codecid"].asInt();
	}

	return true;
}
