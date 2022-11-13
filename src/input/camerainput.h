#ifndef CAMERAINPUT_H
#define CAMERAINPUT_H

#include "common/types.h"
#include "iinput.h"

// Camera input implementation
class CameraInput : public IInput
{
public:
	CameraInput();
	virtual ~CameraInput();

public:
	static InputPtr create() { return std::make_shared<CameraInput>(); }
	bool open() override;
	void close() override;
	bool read(MatPtr &frame) override;
};

#endif // CAMERAINPUT_H
