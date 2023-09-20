#ifndef INNETWORK_H
#define INNETWORK_H

#include "common/types.h"
#include "common/config.h"

namespace nn
{

// Neural network interface
class INNetwork
{
protected:
	NnType _type;
	float _confThreshold;
	float _nmsThreshold;
	int _modelChannels;
	int _modelWidth;
	int _modelHeight;
	int _numClasses;
	string _classesFile;
	std::vector<string> _classes;
	std::vector<float> _anchors;

public:
	INNetwork(NnType type)
		: _type(type)
		, _modelWidth(416)
		, _modelHeight(416)
		, _modelChannels(3)
		, _numClasses(80)
		, _confThreshold(0.6f)
		, _nmsThreshold(0.5f) {}
	virtual ~INNetwork() {}

public:
	virtual bool init(const string &model, const string &cfg, void *params = nullptr) = 0;
	virtual bool setInput(const MatPtr &frame) = 0;
	virtual bool detect(RectList &out) = 0;

	void setModelWidth(int arg) { _modelWidth = arg; }
	void setModelHeight(int arg) { _modelHeight = arg; }
	void setModelChannels(int arg) { _modelChannels = arg; }
	void setNumClasses(int arg) { _numClasses = arg; }
	void setConfThreshold(float arg) { _confThreshold = arg; }
	void setNmsThreshold(float arg) { _nmsThreshold = arg; }
	void setClassesFile(const string &fileName) { _classesFile = fileName; }
	void setClasses(const std::vector<string> &classes) { _classes = classes; }
	void setAnchors(const std::vector<float> &anchors) { _anchors = anchors; }

	int modelWidth() const { return _modelWidth; }
	int modelHeight() const { return _modelHeight; }
	int modelChannels() const { return _modelChannels; }
	int numClasses() const { return _numClasses; }
	int confThreshold() const { return _confThreshold; }
	int nmsThreshold() const { return _nmsThreshold; }
	string classesFile() const { return _classesFile; }
	std::vector<string> classes() const { return _classes; }
};

typedef std::shared_ptr<INNetwork> NNetworkPtr;

}

#endif // INNETWORK_H
