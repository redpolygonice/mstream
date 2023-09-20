#ifndef FILEOUTPUT_H
#define FILEOUTPUT_H

#include "common/types.h"
#include "ioutput.h"
#include "common/avwriter.h"

namespace output
{

// Video file output implemetation
class File : public IOutput
{
private:
	std::atomic_bool _active;
	AvWriter _writer;

public:
	File();
	virtual ~File();

public:
	static OutputPtr create() { return std::make_shared<File>(); }
	bool open() override;
	void close() override;
	bool write(const MatPtr &frame) override;
};

}

#endif // FILEOUTPUT_H
