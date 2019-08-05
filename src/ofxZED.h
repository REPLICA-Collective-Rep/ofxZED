#pragma once

#include "ofMain.h"
#include <sl/Camera.hpp>
#include "ofxZEDCamera.h"
using namespace sl;


namespace ofxZED {

    string getSDKVersion();
    int getNumDevices();
}
