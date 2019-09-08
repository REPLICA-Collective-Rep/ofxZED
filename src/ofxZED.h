#pragma once

#include "ofMain.h"
#include <sl/Camera.hpp>

/* 

ZED SDK: SVO-related methods

int         getSVOPosition()
void        setSVOPosition(int frame_number)
int         getSVONumberOfFrames()
Resolution  getResolution() 
            returns { size_t width, size_t height }
float       getCameraFPS()
timeStamp   getTimestamp( sl::TIME_REFERENCE reference_time) 
            returns { typedef uint64_t timeStamp }

sl::TIME_REFERENCE {
    TIME_REFERENCE_IMAGE - defines the timestamp at the time the frame has been extracted from USB stream.
    TIME_REFERENCE_CURRENT - defines the timestamp at the time of the function call.
    TIME_REFERENCE_LAST
}

NB! make a function "guessDroppedFrames()"

**/


class ofxZED : public sl::Camera {
    ofTexture colorTexture, depthTexture;
    sl::Mat colorMat;
    sl::Mat depthMat;
public:
    sl::InitParameters init;
//    ofParameter<int> camera_fps;
//    ofParameter<int> depth_stabilisation;
//    ofParameter<int> depth_mode;
//    ofParameter<bool> camera_image_flip;
//    ofParameter<bool> camera_disable_imu;
//    ofParameter<bool> enable_right_side_measure;
//    ofParameter<string> svo_input_filename;

    int frameCount = 0;
    bool isRecording = false;
    bool frameNew = false;

    int getSerialNumber();

    void logSerial();

    ofPixels processDepthMat(sl::Mat & mat);
    ofPixels processColorMat(sl::Mat & mat);


    string getTimestamp(string format = "%Y-%m-%d_%H:%M:%S");

    void record(string path, bool b, bool autoPath = true);

    void toggleRecording(string path, bool autoPath = true);
    void update();
    bool isFrameNew();

    ofQuaternion eulerToQuat(const ofVec3f & rotationEuler);

    int getWidth();
    int getHeight();


    bool openSVO(string svoPath);

    bool openCamera(int i = -1);

    bool openWithParams();

    uint64_t getFrameTimestamp();
    uint64_t getLastTimestamp();

    void close(ofEventArgs &args);
    void close();

    void draw(ofRectangle r);

};
