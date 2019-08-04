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

    vector<ofxZEDDevice> getDeviceList();
    void printDeviceList();
    string getSDKVersion();
    int getNumDevices();
}
