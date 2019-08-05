#include "ofxZED.h"
namespace ofxZED {

    string getSDKVersion() {

        sl::Camera zed;
        std::string sdk(zed.getSDKVersion().get());
        return sdk;
    }

    int getNumDevices() {

        sl::Camera zed;
        return zed.isZEDconnected();
    }

}
