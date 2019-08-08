#pragma once

#include "ofMain.h"
#include <sl/Camera.hpp>

const int MAX_CHAR = 128;

class ofxZEDCamera {
	ofTexture colorTexture, depthTexture;
	sl::Mat colorMat;
	sl::Mat depthMat;
	sl::Pose camera_path;
	sl::TRACKING_STATE tracking_state;
public:
    sl::Camera z;

	sl::float3 translation;
	ofVec3f rotEuler;
	ofVec3f rotAngle;
	ofQuaternion rotQuat;

    ofParameter<int> camera_fps;
    ofParameter<int> depth_stabilisation;
    ofParameter<int> depth_mode;
    ofParameter<bool> camera_image_flip;
    ofParameter<bool> camera_disable_imu;
    ofParameter<bool> enable_right_side_measure;
    ofParameter<string> svo_input_filename;

	int getSerialNumber() {
		if (!z.isOpened()) return -1;
		return z.getCameraInformation().serial_number;
	}

    sl::CameraInformation getCameraInformation() {
		sl::CameraInformation i;
		if (!z.isOpened()) {
			ofLogError("ofxZED") << "Cannot get CameraInformation on closed Camera";
			return i;
		}
		return z.getCameraInformation();
	}

	int getCameraFps() {
		return z.getCameraFPS();
	}

	void log() {
		ofLogNotice( "ofxZED") << z.getCameraInformation().serial_number << ((z.isOpened()) ? "is Open" : "is Closed");
	}

	ofPixels processDepthMat(sl::Mat & mat) {
		ofPixels pix;
		ofColor c;
		int w = mat.getWidth();
		int h = mat.getHeight();
		pix.allocate(w, h, 4);

		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				sl::uchar4  p;
				mat.getValue<sl::uchar4 >(x, y, &p);
				int index = 3 * (x + y * w);

				//c.setHsb(p.r/2, 255, ((int)p.r == 0) ? 0 : 255, 255);

				pix[index + 0] = 255 - p.r;
				pix[index + 1] = 255 + p.r;
				pix[index + 2] = 255;
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

    int frameCount = 0;

    bool isRecording = false;

    string getTimestamp() {
        string date =  ofToString(ofGetYear()) + "-" + ofToString(ofGetMonth()) + "-" + ofToString(ofGetDay()) ;
        date += "-";
        std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(p);
//        std::stringstream time;
        string format =  "%a %b %d %y %H:%M:%S ";
//        time << std::put_time(&t, format.c_str());

        char buff[255];
        strftime(buff, 255, "%Y-%m-%d_%H:%M:%S", localtime(&t));
        string time(buff);

        return (time);
    }

    void record(bool b, string path, bool autoPath = true) {

        if (b) {
            frameCount = 0;
            //std::filesystem::last_write_time(file);
            if (autoPath) {
                path += "";
                path +=  ofToString( z.getCameraInformation().serial_number );
                path += "_";
                path += getTimestamp();
    //            path += ofGetCurrentTime();
                path += ".svo";
            }
            ofLogNotice("ofxZED") << "recording to" << path;
            sl::String p(path.c_str());
            auto error = z.enableRecording(p, sl::SVO_COMPRESSION_MODE_HEVC);
            if (error != sl::SUCCESS) {
                ofLogError("ofxZED") << "Recording initialisation error";
                if (error == sl::ERROR_CODE_SVO_RECORDING_ERROR)
                   ofLogError("ofxZED") << " Note : This error mostly comes from a wrong path or missing writing permissions.";
                if (error == sl::ERROR_CODE_SVO_UNSUPPORTED_COMPRESSION)
                    ofLogError("ofxZED") << " Note : This error mostly comes from a non-compatible graphic card. If you are using HEVC compression (H265), please note that most of the graphic card below pascal architecture will not support it. Prefer to use AVCHD compression which is supported on most of NVIDIA graphic cards";

                z.close();

            } else {
                isRecording = b;
            }
        } else {
            isRecording = b;
            frameCount = 0;
            z.disableRecording();
            ofLogNotice("ofxZED") << "Stopped recording";
        }
    }

    void toggleRecording(string path, bool autoPath = true) {
        if (!isRecording) {
            record(true, path, autoPath);
        } else {
            record(false, path, autoPath);
        }
    }
	void update() {

		// Set runtime parameters after opening the camera
		sl::RuntimeParameters runtime_parameters;
		runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL; // Use STANDARD sensing mode
		runtime_parameters.enable_depth = true;


		if (z.grab(runtime_parameters) == sl::SUCCESS) {

            if (isRecording) {
                sl::RecordingState state = z.record();
                if (state.status) frameCount++;
//                ofLogNotice("ofxZED") << "Frame count: " << frameCount;
            }

			z.retrieveImage(colorMat, sl::VIEW_LEFT, sl::MEM_CPU, 320,180);
			z.retrieveImage(depthMat, sl::VIEW_DEPTH, sl::MEM_CPU, 320, 180);
			//auto ptr = colorMat.getPtr<sl::uchar4>();

			int w = depthMat.getWidth();
			int h = depthMat.getHeight();
			

			colorTexture.loadData(processColorMat(colorMat).getData(), w, h, GL_RGB);

			depthTexture.loadData(processDepthMat(depthMat).getData(), w, h, GL_RGB);


			tracking_state = z.getPosition(camera_path, sl::REFERENCE_FRAME_WORLD);

			if (tracking_state == sl::TRACKING_STATE_OK) {
				// Get rotation and translation
				sl::float3 rot = camera_path.getEulerAngles();
				rotEuler.set(rot.x, rot.y, rot.z);
				rotQuat = eulerToQuat(rotEuler);
				//rotQuat.getRotate(rotAngle, rotQuat);
				translation = camera_path.getTranslation();

			}
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
		return  z.getResolution().width;
	}

	int getHeight() {
		return z.getResolution().height;
	}

    int open(int i = -1) {

        //camera_fps.set(p.camera_fps);
        //depth_stabilisation.set(p.depth_stabilization);
        //depth_mode.set(p.depth_mode);
        //camera_image_flip.set(p.camera_image_flip);
        //camera_disable_imu.set(p.camera_disable_imu);
        //enable_right_side_measure.set(p.enable_right_side_measure);
        //svo_input_filename.set(p.svo_input_filename.get());

        ofAddListener(ofGetWindowPtr()->events().exit, this,&ofxZEDCamera::close);

        sl::InitParameters p;

		p.depth_mode = sl::DEPTH_MODE_QUALITY; // Use PERFORMANCE depth mode
        p.sdk_verbose = true;
		p.coordinate_units = sl::UNIT_METER;
		p.coordinate_system = sl::COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP;

		if (i != -1) p.input.setFromCameraID(i);

		auto isOpened = z.open(p);
		if (isOpened != sl::SUCCESS) {
			z.close();
			ofLogNotice("ofxZED") << "Couldn't open ZED Camera ";
		}
		else {
			ofLogNotice("ofxZED") << "Serial number" << z.getCameraInformation().serial_number;
			z.enableTracking();

			int w = getWidth();
			int h = getHeight();

			colorTexture.allocate(w, h, GL_RGBA, false);
			depthTexture.allocate(w, h, GL_RGBA, false);

			
		}

		return isOpened;



    }


	sl::Camera & getCamera() {
		return z;
	}

    void close() {
        z.close();
    }
    void close(ofEventArgs &args) {
        z.close();
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
