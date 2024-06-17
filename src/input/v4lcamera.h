#ifndef V4LCAMERA_H
#define V4LCAMERA_H

#include "common/types.h"
#include "iinput.h"

#include <linux/videodev2.h>

namespace input
{

// V4l2 Camera input implementation
class V4lCamera : public IInput
{
public:
    enum OutFormat
    {
        RawFormat,
        RgbFormat,
        JpegFormat
    };

    struct buffer
    {
        void   *start = NULL;
        size_t  length = 0;
    };

private:
    bool _active;
    int _fd;
    buffer *_buffers;
    unsigned int _bufferCount;
    v4l2_buf_type _buftype;
    struct v4l2_format _fmt;

    int _width;
    int _height;
    int _imageQuality;
    int _brightness;
    unsigned int _format;
    OutFormat _outformat;

    bool init();
    void uninit();
    int getFrame(MatPtr &frame);
    bool open(const string &dev);

public:
    V4lCamera();
    ~V4lCamera();

public:
    static InputPtr create() { return std::make_shared<V4lCamera>(); }
    bool open() override;
    void close() override;
    bool read(MatPtr &frame) override;

    void setWidth(int width) { _width = width; }
    void setHeight(int height) { _height = height; }
    void setImageQuality(int quality) { _imageQuality = quality; }
    void setImageBrightness(int brightness) { _brightness = brightness; }
    void setFormat(unsigned int format) { _format = format; }
    void setOutFormat(OutFormat format) { _outformat = format; }
    bool isOpen() const { return _active; }
    void release() { close(); }
    bool read(ByteArray &data);
};

}

#endif // V4LCAMERA_H
