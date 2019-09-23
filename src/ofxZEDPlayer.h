#pragma once

#include "ofMain.h"
#include "ofxZEDCamera.h";
#include "ofxZEDSVO.h";

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

    class Player : public ofxZED::Camera {
    public:
        bool isSettingPosition;

        void init();
        ofShader shader;
        ofPlanePrimitive plane;
        Player();
        SVO * svo;
        bool openSVO(SVO * svo_);
        int grab(bool left, bool right, bool depth);
        void setSVOPosition(int i );
        void nudge( int frames );
        // void drawStereoscopic(ofRectangle r);
    };


}

