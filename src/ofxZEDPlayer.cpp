#include "ofxZEDPlayer.h"

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



    bool Player::openSVO(string root, SVO * svo_) {
        svo = svo_;
        string p = ofFilePath::join(root, svo->filename);
        ofLog() << "path" << p;
        return ofxZED::Camera::openSVO(p);
    }

    int Player::grab(bool left, bool right, bool depth) {


        if (!sl::Camera::isOpened()) return  sl::Camera::getSVOPosition();

        frameNew = false;
        sl::RuntimeParameters runtime_parameters;
        runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL; // Use STANDARD sensing mode
        runtime_parameters.enable_depth = depth;

        if (sl::Camera::grab(runtime_parameters) == sl::SUCCESS) {

            frameNew = true;

            int w = getWidth();
            int h = getHeight();

            if (sl::Camera::getSVOPosition() == lastPosition)  sl::Camera::getSVOPosition();

//            ofLog() << "retrieving..." << w << h;

            if (left) sl::Camera::retrieveImage(leftMat, sl::VIEW_LEFT, sl::MEM_CPU, w,h);
            if (right) sl::Camera::retrieveImage(rightMat, sl::VIEW_RIGHT, sl::MEM_CPU, w,h);
            if (depth) sl::Camera::retrieveImage(depthMat, sl::VIEW_DEPTH, sl::MEM_CPU, w, h);


            if (left) processMatToPix(leftPix, leftMat);
            if (right) processMatToPix(rightPix, rightMat);
            if (depth) processMatToPix(depthPix, depthMat, true);


            if (left) leftTex.loadData(leftPix.getData(), w, h, GL_RGB);
            if (right) rightTex.loadData(rightPix.getData(), w, h, GL_RGB);
            if (depth) depthTex.loadData(depthPix.getData(), w, h, GL_RGB);

            lastPosition = sl::Camera::getSVOPosition();

        } else {

            ofLog() << "Did not grab";
        }

        return sl::Camera::getSVOPosition();
    }




}
