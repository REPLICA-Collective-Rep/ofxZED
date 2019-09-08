#pragma once

#include "ofMain.h"
#include <sl/Camera.hpp>

class ofxZED : public sl::Camera {
    ofTexture colorTexture, depthTexture;
    sl::Mat colorMat;
    sl::Mat depthMat;
//	sl::Pose camera_path;
//	sl::TRACKING_STATE tracking_state;
public:
//    sl::Camera z;
//	sl::float3 translation;
//	ofVec3f rotEuler;
//	ofVec3f rotAngle;
//	ofQuaternion rotQuat;

    sl::InitParameters init;
//    ofParameter<int> camera_fps;
//    ofParameter<int> depth_stabilisation;
//    ofParameter<int> depth_mode;
//    ofParameter<bool> camera_image_flip;
//    ofParameter<bool> camera_disable_imu;
//    ofParameter<bool> enable_right_side_measure;
//    ofParameter<string> svo_input_filename;

    int frameCount = 0;
    bool isRecording = false;

    int getSerialNumber() {
        if (!sl::Camera::isOpened()) return -1;
        return sl::Camera::getCameraInformation().serial_number;
    }

    void logSerial() {
        ofLogNotice( "ofxZED") << sl::Camera::getCameraInformation().serial_number << ((sl::Camera::isOpened()) ? "is Open" : "is Closed");
    }

    ofPixels processDepthMat(sl::Mat & mat) {
        ofPixels pix;
        ofColor c;
        int w = mat.getWidth();
        int h = mat.getHeight();
        pix.allocate(w, h, 4);

//        ofLog() << "process depth mat" << w << h;

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                sl::uchar4  p;
                mat.getValue<sl::uchar4 >(x, y, &p);
                int index = 3 * (x + y * w);

                pix[index + 0] = p.r;
                pix[index + 1] = (p.r*2 < 255) ? 255 - (p.r*2) : 0;
                pix[index + 2] = (p.r > 255/2) ? (255) - (p.r - (255/2)) : 255;
                pix[index + 3] = 255;
            }
        }
        return pix;
    }
    ofPixels processColorMat(sl::Mat & mat) {
        ofPixels pix;
        int w = mat.getWidth();
        int h = mat.getHeight();
        pix.allocate(w, h, 4);
//        ofLog() << "process color mat" << w << h;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                sl::uchar4  p;
                mat.getValue<sl::uchar4 >(x, y, &p);
                int index = 3 * (x + y * w);
                pix[index + 0] = p.b;
                pix[index + 1] = p.g;
                pix[index + 2] = p.r;
                pix[index + 3] = p.a;

            }
        }
        return pix;
    }


    string getTimestamp(string format = "%Y-%m-%d_%H:%M:%S") {
        string date =  ofToString(ofGetYear()) + "-" + ofToString(ofGetMonth()) + "-" + ofToString(ofGetDay()) ;
        date += "-";
        std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(p);
        char buff[255];
        strftime(buff, 255, format.c_str(), localtime(&t));
        string time(buff);
        return (time);
    }

    void record(string path, bool b, bool autoPath = true) {

        if (b) {
            frameCount = 0;
            //std::filesystem::last_write_time(file);
            if (autoPath) {
                path += "";
                path +=  ofToString( sl::Camera::getCameraInformation().serial_number );
                path += "_";
                path += getTimestamp();
    //            path += ofGetCurrentTime();
                path += ".svo";
            }
            ofLogNotice("ofxZED") << "recording to" << path;
            sl::String p(path.c_str());
            auto error = sl::Camera::enableRecording(p, sl::SVO_COMPRESSION_MODE_HEVC);
            if (error != sl::SUCCESS) {
                ofLogError("ofxZED") << "Recording initialisation error";
                if (error == sl::ERROR_CODE_SVO_RECORDING_ERROR)
                   ofLogError("ofxZED") << " Note : This error mostly comes from a wrong path or missing writing permissions.";
                if (error == sl::ERROR_CODE_SVO_UNSUPPORTED_COMPRESSION)
                    ofLogError("ofxZED") << " Note : This error mostly comes from a non-compatible graphic card. If you are using HEVC compression (H265), please note that most of the graphic card below pascal architecture will not support it. Prefer to use AVCHD compression which is supported on most of NVIDIA graphic cards";

                sl::Camera::close();

            } else {
                isRecording = b;
            }
        } else {
            isRecording = b;
            frameCount = 0;
            sl::Camera::disableRecording();
            ofLogNotice("ofxZED") << "Stopped recording";
        }
    }

    void toggleRecording(string path, bool autoPath = true) {
        if (!isRecording) {
            record(path, true, autoPath);
        } else {
            record(path, false, autoPath);
        }
    }
    void update() {

        // Set runtime parameters after opening the camera
        sl::RuntimeParameters runtime_parameters;
        runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL; // Use STANDARD sensing mode
        runtime_parameters.enable_depth = true;


        if (sl::Camera::grab(runtime_parameters) == sl::SUCCESS) {

            if (isRecording) {
                sl::RecordingState state = sl::Camera::record();
                if (state.status) frameCount++;
//                ofLogNotice("ofxZED") << "Frame count: " << frameCount;
            }

            sl::Camera::retrieveImage(colorMat, sl::VIEW_LEFT, sl::MEM_CPU, 320,180);
            sl::Camera::retrieveImage(depthMat, sl::VIEW_DEPTH, sl::MEM_CPU, 320, 180);
            //auto ptr = colorMat.getPtr<sl::uchar4>();

            int w = depthMat.getWidth();
            int h = depthMat.getHeight();


            colorTexture.loadData(processColorMat(colorMat).getData(), w, h, GL_RGB);

            depthTexture.loadData(processDepthMat(depthMat).getData(), w, h, GL_RGB);


//			tracking_state = z.getPosition(camera_path, sl::REFERENCE_FRAME_WORLD);

//			if (tracking_state == sl::TRACKING_STATE_OK) {
//				// Get rotation and translation
//				sl::float3 rot = camera_path.getEulerAngles();
//				rotEuler.set(rot.x, rot.y, rot.z);
//				rotQuat = eulerToQuat(rotEuler);
//				//rotQuat.getRotate(rotAngle, rotQuat);
//				translation = camera_path.getTranslation();

//			}
            //depthTexture.loadData(getDepthBuffer(), w, h, GL_LUMINANCE);
        }
    }
    ofQuaternion eulerToQuat(const ofVec3f & rotationEuler) {
        ofQuaternion rotation;
        float c1 = cos(rotationEuler[2] * 0.5);
        float c2 = cos(rotationEuler[1] * 0.5);
        float c3 = cos(rotationEuler[0] * 0.5);
        float s1 = sin(rotationEuler[2] * 0.5);
        float s2 = sin(rotationEuler[1] * 0.5);
        float s3 = sin(rotationEuler[0] * 0.5);

        rotation[0] = c1 * c2*s3 - s1 * s2*c3;
        rotation[1] = c1 * s2*c3 + s1 * c2*s3;
        rotation[2] = s1 * c2*c3 - c1 * s2*s3;
        rotation[3] = c1 * c2*c3 + s1 * s2*s3;

        return rotation;
    }

    int getWidth() {
        return  sl::Camera::getResolution().width;
    }

    int getHeight() {
        return sl::Camera::getResolution().height;
    }


    bool openSVO(string svoPath) {

        // ** Possible SVO related methods **

        // int getSVOPosition ()
        // void 	setSVOPosition (int frame_number)
        // int 	getSVONumberOfFrames ()
        // Resolution getResolution 	( 		) Attributes: 	size_t 	width size_t 	height
        // float getCameraFPS 	( 		)
        // timeStamp getTimestamp 	( 	sl::TIME_REFERENCE  	reference_time	) 	Attribute: typedef uint64_t timeStamp
        //          TIME_REFERENCE_IMAGE

        //        Defines the timestamp at the time the frame has been extracted from USB stream.
        //        TIME_REFERENCE_CURRENT

        //        Defines the timestamp at the time of the function call.
        //        TIME_REFERENCE_LAST

        // make a function "guessDroppedFrames()"

        ofLogNotice("ofxZED") << "loading svo" << svoPath;
        init.svo_input_filename.set(svoPath.c_str());

        return openWithParams();
    }

    bool openCamera(int i = -1) {



        init.camera_resolution = sl::RESOLUTION_HD1080;
        init.depth_mode = sl::DEPTH_MODE_QUALITY; // Use PERFORMANCE depth mode
        init.sdk_verbose = true;
        init.coordinate_units = sl::UNIT_METER;
        init.coordinate_system = sl::COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP;

        if (i != -1) init.input.setFromCameraID(i);


        return openWithParams();
    }

    bool openWithParams() {

        if (sl::Camera::isOpened()) {
            sl::Camera::close();
        } else {
            ofAddListener(ofGetWindowPtr()->events().exit, this, &ofxZED::close);
        }

        bool success = false;


        try {
            auto resp = sl::Camera::open(init);
            if (resp != sl::SUCCESS) {
                ofLogNotice("ofxZED") << "couldn't open ZED: ";
                std::cout << sl::toString(resp) << std::endl; // Display the error;
                sl::Camera::close();

            } else {

                ofLogNotice("ofxZED") << "successfully loaded";// << sl::Camera::getCameraInformation().serial_number;
                sl::Camera::enableTracking();

                int w = getWidth();
                int h = getHeight();


    //            colorTexture.allocate(w, h, GL_RGBA, false);
    //            depthTexture.allocate(w, h, GL_RGBA, false);
                success = true;
            }

        } catch (const std::exception& e) {

            ofLogNotice("ofxZED") << "couldn't open ZED from unknown error ";
        }

        return success;
    }

    uint64_t getFrameTimestamp() {
        return  sl::Camera::getTimestamp(sl::TIME_REFERENCE_IMAGE);
    }
    uint64_t getLastTimestamp() {
        return  sl::Camera::getTimestamp(sl::TIME_REFERENCE_LAST);
    }

    void close(ofEventArgs &args) {
        sl::Camera::close();
    }

    void close() {
        sl::Camera::close();
    }

    void draw(ofRectangle r) {
        ofSetColor(255);
        ofEnableAlphaBlending();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        if (colorTexture.isAllocated()) colorTexture.draw(r);
        ofEnableBlendMode(OF_BLENDMODE_SCREEN);
        if (depthTexture.isAllocated()) depthTexture.draw(r);
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    }

};
