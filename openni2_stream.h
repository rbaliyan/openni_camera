#ifndef __OPENNI2_STREAM_H__
#define __OPENNI2_STREAM_H__

#include <OpenNI.h>

#include "v4l2sink.h"

#define BUFFER_SIZE 10 * 1024 *


class  openNI2v4lSink: public v4l2sink::v4l2Sink, public openni::VideoStream::NewFrameListener
{
    private:
        uint64_t frameCount = 0;
        uint64_t lastFrameCount = 0;
        uint64_t startTimestamp = 0;
        uint64_t lastTimestamp = 0;
        uint fps = 0;
        openni::VideoFrameRef videoFrame;
    public:
        openNI2v4lSink(const char *vidDevice, v4l2sink::Format format):v4l2sink::v4l2Sink(vidDevice,format){}
        ~openNI2v4lSink()
        {
            close();
        }
        void onNewFrame (openni::VideoStream& stream);
        uint64_t getFrameCount()
        {
            return frameCount;
        }
        uint getFps()
        {
            return fps;
        }
        bool setResolution(int width, int height, openni::PixelFormat);
        //static v4l2sink::Format getVideoFormat(openni::PixelFormat);
};


class OpenNI2Stream
{
    private:
        openni::SensorType sensorType;
        const openni::SensorInfo *sensorInfo;
        openni::VideoStream stream;
        uint currentVideoModeIndex;
        const char* err;
        bool started = false;
        openNI2v4lSink sink;
        openni::CameraSettings* cameraSettings;
        openni::VideoMode *currentVideoMode = NULL;

        void selectVideoMode(uint index);
        void printVideoMode(uint index, const openni::VideoMode&);

    public:
        OpenNI2Stream(const char* vidDevice, openni::SensorType sensor):
            sink(vidDevice, sensor == openni::SENSOR_COLOR ? v4l2sink::YUYV: (sensor ==  openni::SENSOR_IR ?  v4l2sink::GRAY16 : v4l2sink::GRAY))
        {
            sensorType = sensor;
            currentVideoModeIndex = 0;
        }
        const char* error()
        {
            return err;
        }
        ~OpenNI2Stream()
        {
            close();
        }
        
        bool open(openni::Device& device);
        void close(void);
        void start(uint videoModeIndex);
        void stop(void);
        void setMirroringEnabled(bool enabled);
        void setAutoExposureEnabled(bool enabled);
        void setAutoWhiteBalanceEnabled(bool enabled);
        void setGain(int gain);
        void setExposure(int exposure);
        int getGain(void);
        int getExposure(void);
        bool getMirroringEnabled(void);
        bool getAutoExposureEnabled(void);
        bool getAutoWhiteBalanceEnabled(void);
        void printConfig(void);
        uint64_t getFrameCount(void);
        uint getCurrentFPS(void);
};

#endif