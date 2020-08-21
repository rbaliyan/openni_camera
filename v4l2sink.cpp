#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "v4l2sink.h"

using namespace v4l2sink;

bool v4l2Sink::open(void)
{
    std::cout<<"Open Stream: "<<path<<std::endl;
    fd = ::open(path, O_RDWR);
    if(fd < 0) 
    {
        err = strerror(errno);
        std::cerr << "ERROR: could not open output device!\n" << err << std::endl; 
        return false;
    }
    memset(&vid_format, 0, sizeof(vid_format));
    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (ioctl(fd, VIDIOC_G_FMT, &vid_format) < 0)
    {
        err = strerror(errno); 
        std::cerr << "ERROR: unable to get video format" << err<< std::endl;
        return false;
    }
    
    return setResolution(320, 240, pixelFormat);
}

void v4l2Sink::close(void)
{
    if(fd > 0)
    {
        std::cout<<"Close Stream"<<std::endl;
        ::close(fd);
        fd = -1;
    }
}


bool v4l2Sink::setResolution(int width, int height, Format format)
{
    if(frameWidth == width && frameHeight == height && pixelFormat == format)
    {
        std::cout << "Skip Video resolution update, already set" << std::endl;
        return true;
    }

    std::cout << "Old Resolutution : " << frameWidth << "X" << frameHeight <<":"<<pixelFormat<< std::endl;

    frameWidth = width;
    frameHeight = height;
    pixelFormat = format;
    vid_format.fmt.pix.width = frameWidth;
    vid_format.fmt.pix.height = frameHeight;
    vid_format.fmt.pix.pixelformat = pixelFormat;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &vid_format) < 0)
    {
        err = strerror(errno); 
        std::cerr << "ERROR: unable to set video format!"<<err<<std::endl;
        return false;
    }
    
    print();
    return true;
}

bool v4l2Sink::write(const void* buffer, size_t size)
{
    int written = ::write(fd, buffer, size);
    if (written < 0)
    {
        err = strerror(errno); 
        std::cerr << "ERROR: could not write to output device!"<<err<<std::endl;
        return false;
    }
    return true;
}

void v4l2Sink::print(void) 
{
    const char *formatStr;
    switch(pixelFormat)
    {
        case GRAY: formatStr = "GRAY"; break;
        case GRAY16: formatStr = "GRAY16"; break;
        case BGR: formatStr = "BGR"; break;
        case BGRA: formatStr = "BGRA"; break;
        case RGB: formatStr = "RGB"; break;
        case RGBA: formatStr = "RGB"; break;
        case YUYV:  formatStr = "RGB"; break;
        case YUV420:    formatStr = "RGB"; break;
        case JPEG: formatStr = "JPEG"; break;
        case DEPTH: formatStr = "DEPTH"; break;
    }
    std::cout << "Video configration :"<< std::endl;
    std::cout << "  Resolution                      = " <<   frameWidth << "X" << frameHeight << std::endl;
    std::cout << "  Format                          = " <<   formatStr << std::endl;
    std::cerr << "  vid_format.type                 = " << 	vid_format.type << std::endl;
    std::cerr << "  vid_format.fmt.pix.width        = " << 	vid_format.fmt.pix.width << std::endl;
    std::cerr << "  vid_format.fmt.pix.height       = " << 	vid_format.fmt.pix.height << std::endl;
    std::cerr << "  vid_format.fmt.pix.pixelformat  = " << 	vid_format.fmt.pix.pixelformat<< std::endl;
    std::cerr << "  vid_format.fmt.pix.sizeimage    = " << 	vid_format.fmt.pix.sizeimage << std::endl;
    std::cerr << "  vid_format.fmt.pix.field        = " << 	vid_format.fmt.pix.field << std::endl;
    std::cerr << "  vid_format.fmt.pix.bytesperline = " << 	vid_format.fmt.pix.bytesperline << std::endl;
    std::cerr << "  vid_format.fmt.pix.colorspace   = " << 	vid_format.fmt.pix.colorspace << std::endl;
}
