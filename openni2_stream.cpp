#include <opencv2/opencv.hpp>
#include <OpenNI.h>
#include <time.h>
#include "v4l2sink.h"
#include "openni2_stream.h"


bool openNI2v4lSink::setResolution(int width, int height, openni::PixelFormat pixelFormat)
{
    v4l2sink::Format format;
    switch(pixelFormat)
    {

        case openni::PIXEL_FORMAT_GRAY8: format = v4l2sink::GRAY;break;
        case openni::PIXEL_FORMAT_GRAY16: format = v4l2sink::GRAY; break;

        // Color
        case openni::PIXEL_FORMAT_RGB888:   format = v4l2sink::BGR; break;
        case openni::PIXEL_FORMAT_JPEG: format = v4l2sink::JPEG; break;
        case openni::PIXEL_FORMAT_YUV422:
        case openni::PIXEL_FORMAT_YUYV: format = v4l2sink::YUYV; break;

        // Depth
        case openni::PIXEL_FORMAT_DEPTH_1_MM: format = v4l2sink::DEPTH; break;
        case openni::PIXEL_FORMAT_DEPTH_100_UM: format = v4l2sink::DEPTH; break;
        case openni::PIXEL_FORMAT_SHIFT_9_2:  
        case openni::PIXEL_FORMAT_SHIFT_9_3:
        default:
            format = v4l2sink::RGB; break;
    }
    return v4l2Sink::setResolution(width, height, format);
}

void openNI2v4lSink::onNewFrame (openni::VideoStream& stream)
{
    const void *buffer;
    stream.readFrame(&videoFrame);
    uint64_t delta = (videoFrame.getTimestamp() - lastTimestamp);
    frameCount++;
    if(startTimestamp == 0)
    {
        startTimestamp = videoFrame.getTimestamp();
        lastTimestamp = startTimestamp;
        lastFrameCount = frameCount;
    }
    else if(delta >= 1000000L)
    {
        fps = frameCount - lastFrameCount;
        lastFrameCount = frameCount;
        lastTimestamp = videoFrame.getTimestamp();
    }

    write(videoFrame.getData(), videoFrame.getDataSize());
}


void OpenNI2Stream::start(uint videoModeIndex)
{
    selectVideoMode(videoModeIndex);
    stream.start();
    stream.addNewFrameListener(&sink);
    started = true;
}

void OpenNI2Stream::stop(void)
{
    if(started)
    {
        stream.removeNewFrameListener(&sink);
        stream.stop();
    }
}

void OpenNI2Stream::printVideoMode(uint index, const openni::VideoMode& mode)
{
    const char* pixleFormat;
    int x = mode.getResolutionX();
    int y = mode.getResolutionY();
    int fps = mode.getFps();

    switch(mode.getPixelFormat())
    {
        // DEPTH
        case openni::PIXEL_FORMAT_DEPTH_1_MM: pixleFormat = "PIXEL_FORMAT_DEPTH_1_MM"; break;
        case openni::PIXEL_FORMAT_DEPTH_100_UM: pixleFormat = "PIXEL_FORMAT_DEPTH_100_UM"; break;
        case openni::PIXEL_FORMAT_SHIFT_9_2: pixleFormat = "PIXEL_FORMAT_SHIFT_9_2"; break;
        case openni::PIXEL_FORMAT_SHIFT_9_3: pixleFormat = "PIXEL_FORMAT_SHIFT_9_3"; break;

        // Color
        case openni::PIXEL_FORMAT_RGB888: pixleFormat = "PIXEL_FORMAT_RGB888"; break;
        case openni::PIXEL_FORMAT_YUV422: pixleFormat = "PIXEL_FORMAT_YUV422"; break;
        case openni::PIXEL_FORMAT_GRAY8: pixleFormat = "PIXEL_FORMAT_GRAY8"; break;
        case openni::PIXEL_FORMAT_GRAY16: pixleFormat = "PIXEL_FORMAT_GRAY16"; break;
        case openni::PIXEL_FORMAT_JPEG: pixleFormat = "PIXEL_FORMAT_JPEG"; break;
        case openni::PIXEL_FORMAT_YUYV: pixleFormat = "PIXEL_FORMAT_YUYV"; break;
    }
    std::cout<< index << " " << x << "X" << y << "@" << fps << " " << pixleFormat << "=" << mode.getPixelFormat() << std::endl;
}



void OpenNI2Stream::selectVideoMode(uint index)
{
    if(index != currentVideoModeIndex)
    {
        if(currentVideoMode)
        {
            delete(currentVideoMode);
            currentVideoMode = NULL;
        }
        const openni::Array< openni::VideoMode>& videoModes = sensorInfo->getSupportedVideoModes();
        if(index < (uint)videoModes.getSize())
        {
            currentVideoModeIndex = index;
            std::cout<<"Supported Video modes: "<< std::endl;
            for (uint i = 0; i < (uint)videoModes.getSize(); i++)
            {
                printVideoMode(i, videoModes[i]);
            }

            std::cout<<"Selecting Video mode: "<< currentVideoModeIndex << std::endl;
            openni::Status rc = stream.setVideoMode(videoModes[currentVideoModeIndex]);
            if (openni::STATUS_OK == rc)
            {
                currentVideoMode = new openni::VideoMode(videoModes[currentVideoModeIndex]);
                std::cout << "Set Video resolution:" << std::endl;
                sink.setResolution(videoModes[currentVideoModeIndex].getResolutionX(), videoModes[currentVideoModeIndex].getResolutionY(), currentVideoMode->getPixelFormat());
            }
            else
            {
                std::cout << "error: depth fromat not supprted..." << std::endl;
            }
        } 
        else
        {
            std::cout << "Video Mode with index: " << index << " not supported, max: " << videoModes.getSize() << std::endl;
        }
    } 
    else
    {
        std::cout << "Video Mode already seleced " << index << std::endl;
    }
}

void OpenNI2Stream::close()
{
    if(stream.isValid())
    {
        stop();
        stream.destroy();
    }
    sink.close();
}

bool OpenNI2Stream::open(openni::Device& device)
{
    openni::Status rc;
    if (!sink.open())
    {
        err = sink.error();
        std::cerr << "Failed to initialize video sink"<< err <<std::endl;    
        return false;
    }
    if(!device.hasSensor(sensorType))
    {
        err = "Device does not have sensor";
        std::cerr <<"Failed to get sensor " << err << std::endl;
        return false;
    }
    sensorInfo = device.getSensorInfo(sensorType);
    if (sensorInfo == NULL)
    {
        err = openni::OpenNI::getExtendedError();
        std::cerr <<"Failed to get sensor info:" << err << std::endl;
        return false;
    }

   
    rc = stream.create(device, sensorType);
    if (rc != openni::STATUS_OK)
    {
        err = openni::OpenNI::getExtendedError();
        std::cerr <<"Couldn't find color stream" << err << std::endl;
        return false;
    }

    cameraSettings  = stream.getCameraSettings();
    return true;
}

void OpenNI2Stream::setAutoExposureEnabled(bool enabled)
{
    if(cameraSettings != NULL)
    {
        cameraSettings->setAutoExposureEnabled(enabled);    
    }
}

void OpenNI2Stream::setAutoWhiteBalanceEnabled(bool enabled)
{
    if(cameraSettings != NULL)
    {
        cameraSettings->setAutoWhiteBalanceEnabled(enabled);
    }
}

void OpenNI2Stream::setGain(int gain)
{
    if(cameraSettings != NULL)
    {
        cameraSettings->setGain(gain);
    }
}

void OpenNI2Stream::setExposure(int exposure)
{
    if(cameraSettings != NULL)
    {
        cameraSettings->setExposure(exposure);
    }
}

bool OpenNI2Stream::getAutoExposureEnabled(void)
{
    if(cameraSettings == NULL)
    {
        return false;
    }
    return cameraSettings->getAutoExposureEnabled();
}

bool OpenNI2Stream::getAutoWhiteBalanceEnabled(void)
{
    if(cameraSettings == NULL)
    {
        return false;
    }
    return cameraSettings->getAutoWhiteBalanceEnabled();
}

bool OpenNI2Stream::getMirroringEnabled(void)
{   
    return stream.getMirroringEnabled();
}

void OpenNI2Stream::setMirroringEnabled(bool enabled)
{
    stream.setMirroringEnabled(enabled);
}

int OpenNI2Stream::getGain(void)
{
    if(cameraSettings == NULL)
    {
        return 0;
    }
    return cameraSettings->getGain();
}

int OpenNI2Stream::getExposure(void)
{
    if(cameraSettings == NULL)
    {
        return 0;
    }
    return cameraSettings->getExposure();
}

uint OpenNI2Stream::getCurrentFPS(void)
{
    return sink.getFps();
}

uint64_t OpenNI2Stream::getFrameCount(void)
{   
    return sink.getFrameCount();
}

void OpenNI2Stream::printConfig(void)
{
    std::cout << "Camera Config:" << std::endl;
    std::cout << "Auto Exposure      =" << getAutoExposureEnabled() << std::endl;
    std::cout << "Auto WhiteBalance  =" << getAutoWhiteBalanceEnabled() << std::endl;
    std::cout << "Exposure           =" << getExposure() << std::endl;
    std::cout << "Gain               =" << getGain() << std::endl;
    std::cout << "Mirror             =" << getMirroringEnabled() << std::endl;    
}