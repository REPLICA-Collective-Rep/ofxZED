#include "ofMain.h"
#include <sl/Camera.hpp>


class ofxZEDCamera {
public:
    sl::Camera z;

    ofParameter<int> camera_fps;
    ofParameter<int> depth_stabilisation;
    ofParameter<int> depth_mode;
    ofParameter<bool> camera_image_flip;
    ofParameter<bool> camera_disable_imu;
    ofParameter<bool> enable_right_side_measure;
    ofParameter<string> svo_input_filename;

    void setup() {

        sl::InitParameters p;
        p.sdk_verbose = true;

        camera_fps.set(p.camera_fps);
        depth_stabilisation.set(p.depth_stabilization);
        depth_mode.set(p.depth_mode);
        camera_image_flip.set(p.camera_image_flip);
        camera_disable_imu.set(p.camera_disable_imu);
        enable_right_side_measure.set(p.enable_right_side_measure);
        svo_input_filename.set(p.svo_input_filename.get());


        sl::ERROR_CODE err = z.open(p);

        if (err != sl::SUCCESS) {
            ofLogError( "ofxZED") << "Could not connect";
        }

    }

    bool isOpened() {
        return z.isOpened();
    }

    unsigned long getTimestampFromSensor() {
//        uint64_t
        return z.getTimestamp(sl::TIME_REFERENCE_IMAGE);
    }
    unsigned long getTimestampFromFunction() {
        return z.getTimestamp(sl::TIME_REFERENCE_IMAGE);
    }
    unsigned long getLastTimestamp() {
        return z.getTimestamp(sl::TIME_REFERENCE_LAST);
    }
    void update() {

    }
    void draw() {

    }

    void save() {

    }
};
