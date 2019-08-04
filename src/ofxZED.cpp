#include "ofxZED.h"
namespace ofxZED {
    vector<ofxZEDDevice> getDeviceList() {
        sl::Camera zed;
        // Set configuration parameters
        std::string sdk(zed.getSDKVersion().get());
        ofLogNotice("ofxZED") << "Running SDK: " << sdk;
        ofLogNotice("ofxZED") << "Number of Cameras: " << zed.isZEDconnected();

        vector<ofxZEDDevice> l;

        for (auto & d : zed.getDeviceList() ) {

           int id = d.id;
           string p = d.path.get();
           int sn = d.serial_number;

           ofxZEDDevice dd(id, p, sn);
           l.push_back(dd);
           ofLogNotice("ofxZED") << "ID: " << id << " PATH: " << p << " SN: " << sn;
        }

        return l;
    }

    void printDeviceList() {
        getDeviceList();
    }

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
