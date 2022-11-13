#ifndef FILEOUTPUT_H
#define FILEOUTPUT_H

#include "common/types.h"
#include "ioutput.h"
#include "avwriter.h"

// Video file output implemetation
class FileOutput : public IOutput
{
private:
	std::atomic_bool _active;
	AvWriter _writer;

public:
	FileOutput();
	virtual ~FileOutput();

public:
	static OutputPtr create() { return std::make_shared<FileOutput>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
};

#endif // FILEOUTPUT_H
