#ifndef __V4L2SINK_H__
#define __V4L2SINK_H__
#include <linux/videodev2.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

namespace v4l2sink{

enum Format { 
    GRAY = V4L2_PIX_FMT_GREY, 
    GRAY16 = V4L2_PIX_FMT_GREY | V4L2_PIX_FMT_Y16,
    RGB = V4L2_PIX_FMT_RGB24, 
    RGBA = V4L2_PIX_FMT_ARGB32, 
    BGR = V4L2_PIX_FMT_BGR24, 
    BGRA = V4L2_PIX_FMT_ABGR32, 
    YUYV = V4L2_PIX_FMT_YUYV, 
    YUV420 = V4L2_PIX_FMT_YUV420, 
    JPEG = V4L2_PIX_FMT_JPEG,
    DEPTH = V4L2_PIX_FMT_Z16,
};

class v4l2Sink
{
    protected:
        int frameWidth, frameHeight;

    private:
        const char* path;
        int fd;
        struct v4l2_format vid_format;
        const char *err;
        Format pixelFormat = BGR;
        
    public:
        v4l2Sink(const char* name, Format videoFormat=BGR)
        {
            path = name;
            fd = -1;
            pixelFormat = videoFormat;
        }
        ~v4l2Sink()
        {
            close();
        }
        const char* error()
        {
            return err;
        }

        void print(void);

        bool open(void);
        void close(void);
        bool setResolution(int width, int height, Format format);
        bool write(const void *buffer, size_t size);
};
};

#endif
