#ifndef CAMERA_H
#define CAMERA_H

#include "common/types.h"
#include "iinput.h"

namespace input
{

// Camera input implementation
class Camera : public IInput
{
private:
    std::atomic_bool _active;
    cv::VideoCapture _capture;

public:
    Camera();
    virtual ~Camera();

public:
    static InputPtr create() { return std::make_shared<Camera>(); }
    bool open() override;
    void close() override;
    bool read(MatPtr &frame) override;
};

}

#endif // CAMERA_H
