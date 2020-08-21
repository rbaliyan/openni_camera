#include <unistd.h>
#include <signal.h>
#include <OpenNI.h>
#include <thread>
#include "v4l2sink.h"
#include "openni2_stream.h"

//#define VIDEO_COLOR  "/dev/video10"
//#define VIDEO_DEPTH  "/dev/video11"
#define VIDEO_IR  "/dev/video12"


// Global flag
bool running = true;

void sig_hanlder(int sig)
{
    running = false;
}

int main(int argc, char*argv[]) 
{
    openni::Status rc;
    openni::Device device;
    signal(SIGINT, sig_hanlder);

    rc = openni::OpenNI::initialize();
    if (rc != openni::STATUS_OK)
	{
		std::cerr <<"OpenNI initialize failed:" << openni::OpenNI::getExtendedError() << std::endl;
		openni::OpenNI::shutdown();
		return -1;
	}

    rc = device.open(openni::ANY_DEVICE);
    if (rc != openni::STATUS_OK)
	{
		std::cerr <<"Device open failed:" << openni::OpenNI::getExtendedError() << std::endl;
		openni::OpenNI::shutdown();
		return -1;
	}

#ifdef VIDEO_COLOR
    int colorModeIndex = 18;
    OpenNI2Stream color(VIDEO_COLOR, openni::SENSOR_COLOR);
    // configure color
    if (!color.open(device)){
        std::cerr << "Failed to initialize video sink"<<color.error()<<std::endl;
        openni::OpenNI::shutdown();
        return -1;
    }
    color.setAutoExposureEnabled(true);
    color.setAutoWhiteBalanceEnabled(true);
    color.setMirroringEnabled(true);
    color.printConfig();
    // Start Streams
        color.start(colorModeIndex);
#endif

#ifdef VIDEO_IR
    OpenNI2Stream ir(VIDEO_IR, openni::SENSOR_IR);
    int irModeIndex = 4;
    // configure ir
    if (!ir.open(device)){
        std::cerr << "Failed to initialize video sink"<<ir.error()<<std::endl;
        openni::OpenNI::shutdown();
        return -1;
    }
    ir.start(irModeIndex);
#endif

#ifdef VIDEO_DEPTH    
    OpenNI2Stream depth(VIDEO_DEPTH, openni::SENSOR_DEPTH);
    int depthModeIndex = 5;

    // configure color
    if (!depth.open(device)){
        std::cerr << "Failed to initialize video sink"<<depth.error()<<std::endl;
        openni::OpenNI::shutdown();
        return -1;
    }
     depth.start(depthModeIndex);
#endif
    uint fps = 0;
    uint64_t frames = 0;
    char msg[1024];
    while (running){
        fps = 0;
        frames = 0;
       #ifdef VIDEO_COLOR
            fps = color.getCurrentFPS();
            frames = color.getFrameCount();
       #endif 

       #ifdef VIDEO_IR
            fps = ir.getCurrentFPS();
            frames += ir.getFrameCount();
       #endif 

       #ifdef VIDEO_DEPTH
            fps = depth.getCurrentFPS();
            frames = depth.getFrameCount();
       #endif 

       sprintf(msg,"\rFrames: %05lu, FPS: %03u", frames, fps);
       std::cerr << msg;
       std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


#ifdef VIDEO_COLOR
    // Cleanup
    color.stop();
    color.close();
#endif

#ifdef VIDEO_IR
    ir.stop();
    ir.close();
#endif

#ifdef VIDEO_DEPTH        
    depth.stop();
    depth.close();
#endif

	device.close();
	openni::OpenNI::shutdown();
    return 0;
}