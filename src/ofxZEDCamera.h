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


namespace ofxZED {


    class Camera : public sl::Camera {
    public:


        static string humanTimestamp(uint64_t & timestamp, string format = "Date %Y-%m-%d Time %H:%M:%S ");
        static std::time_t TimestampToTimeT(sl::timeStamp timestamp);


        int lastPosition;
        float stereoOffset;
        bool stereoAlternate;
        ofTexture leftTex, rightTex, depthTex;
        ofPixels leftPix, rightPix, depthPix;
        sl::Mat leftMat, rightMat, depthMat;
        sl::InitParameters init;
        int frameCount = 0;
        bool isRecording = false;
        bool frameNew = false;

        Camera();

        int getSerialNumber();

        void logSerial();

        void processMatToPix(ofPixels & pix, sl::Mat & mat, bool psychedelic = false);


        string getTimestamp(string format = "%Y-%m-%d_%H:%M:%S");

        void record(string path, bool b, bool autoPath = true);

        void toggleRecording(string path, bool autoPath = true);
        void updateRecording();
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

        void draw(ofRectangle r, bool left = true, bool right = false, bool depth = false);

    };



}

