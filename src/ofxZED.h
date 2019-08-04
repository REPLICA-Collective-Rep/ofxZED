#include "ofMain.h"
#include <sl/Camera.hpp>
#include "ofxZEDCamera.h"
using namespace sl;

struct ofxZEDDevice {
	int id;
	string path;
	int serial_number;
	ofxZEDDevice( int id_, string path_, int serial_number_) {
		id = id;
		path = path;
		serial_number = serial_number_;
	}
};

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