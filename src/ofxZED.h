#pragma once

#ifndef TARGET_OSX
    #include "ofMain.h"
    #include <sl/Camera.hpp>
    #include "ofxZEDCamera.h"
    using namespace sl;


    namespace ofxZED {

        string getSDKVersion();
        int getNumDevices();
    }
#endif
