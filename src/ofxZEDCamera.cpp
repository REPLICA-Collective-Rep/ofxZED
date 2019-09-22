#include "ofxZEDCamera.h"

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


ofxZED::Camera::Camera() {
    stereoOffset = 0;
}

int ofxZED::Camera::getSerialNumber() {
    if (!sl::Camera::isOpened()) return -1;
    return sl::Camera::getCameraInformation().serial_number;
}

void ofxZED::Camera::logSerial() {
    ofLogNotice( "ofxZED") << sl::Camera::getCameraInformation().serial_number << ((sl::Camera::isOpened()) ? "is Open" : "is Closed");
}

void ofxZED::Camera::processMatToPix(ofPixels & pix, sl::Mat & mat, bool psychedelic) {
    int w = mat.getWidth();
    int h = mat.getHeight();
    if (pix.getWidth() != w || pix.getHeight() != h || !pix.isAllocated()) {
        pix.allocate(w, h, 4);
    }
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            sl::uchar4  p;
            mat.getValue<sl::uchar4 >(x, y, &p);
            int index = 3 * (x + y * w);

            if (!psychedelic) {

                pix[index + 0] = p.b;
                pix[index + 1] = p.g;
                pix[index + 2] = p.r;
                pix[index + 3] = p.a;

            } else {

                pix[index + 0] = p.r;
                pix[index + 1] = (p.r*2 < 255) ? 255 - (p.r*2) : 0;
                pix[index + 2] = (p.r > 255/2) ? (255) - (p.r - (255/2)) : 255;
                pix[index + 3] = 255;
            }

        }
    }
}


string ofxZED::Camera::getTimestamp(string format) {
    string date =  ofToString(ofGetYear()) + "-" + ofToString(ofGetMonth()) + "-" + ofToString(ofGetDay()) ;
    date += "-";
    std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(p);
    char buff[255];
    strftime(buff, 255, format.c_str(), localtime(&t));
    string time(buff);
    return (time);
}

void ofxZED::Camera::record(string path, bool b, bool autoPath) {

    if (b) {
        frameCount = 0;
        if (autoPath) {
            path += "";
            path +=  ofToString( sl::Camera::getCameraInformation().serial_number );
            path += "_";
            path += getTimestamp();
            path += ".svo";
        }
        ofLogNotice("ofxZED") << "recording to" << path;
        sl::String p(path.c_str());
        auto error = sl::Camera::enableRecording(p, sl::SVO_COMPRESSION_MODE_HEVC);
        if (error != sl::SUCCESS) {
            ofLogError("ofxZED") << "recording initialisation error";
            if (error == sl::ERROR_CODE_SVO_RECORDING_ERROR)
               ofLogError("ofxZED") << "recording error - check path and write permissions";
            if (error == sl::ERROR_CODE_SVO_UNSUPPORTED_COMPRESSION)
                ofLogError("ofxZED") << "unsupported compression method - check your hardware";

            sl::Camera::close();

        } else {
            isRecording = b;
        }
    } else {
        isRecording = b;
        frameCount = 0;
        sl::Camera::disableRecording();
        ofLogNotice("ofxZED") << "stopped recording";
    }
}

void ofxZED::Camera::toggleRecording(string path, bool autoPath) {
    if (!isRecording) {
        record(path, true, autoPath);
    } else {
        record(path, false, autoPath);
    }
}
void ofxZED::Camera::updateRecording() {

    frameNew = false;
    sl::RuntimeParameters runtime_parameters;
    runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL; // Use STANDARD sensing mode
    runtime_parameters.enable_depth = true;


    if (sl::Camera::grab(runtime_parameters) == sl::SUCCESS) {

        frameNew = true;

        if (isRecording) {
            sl::RecordingState state = sl::Camera::record();
            if (state.status) frameCount++;
//                ofLogNotice("ofxZED") << "Frame count: " << frameCount;
        }
    }
}

bool ofxZED::Camera::isFrameNew() {
    return frameNew;
}
ofQuaternion ofxZED::Camera::eulerToQuat(const ofVec3f & rotationEuler) {
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

int ofxZED::Camera::getWidth() {
    return  sl::Camera::getResolution().width;
}

int ofxZED::Camera::getHeight() {
    return sl::Camera::getResolution().height;
}


bool ofxZED::Camera::openSVO(string svoPath) {

    lastPosition = -1;
    ofLogNotice("ofxZED") << "loading svo" << svoPath;
    init.svo_input_filename.set(svoPath.c_str());
//    init.svo_real_time_mode = true;

    return openWithParams();
}

bool ofxZED::Camera::openCamera(int i) {



    init.camera_resolution = sl::RESOLUTION_HD1080;
    init.depth_mode = sl::DEPTH_MODE_QUALITY; // Use PERFORMANCE depth mode
    init.sdk_verbose = true;
    init.coordinate_units = sl::UNIT_METER;
    init.coordinate_system = sl::COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP;

    if (i != -1) init.input.setFromCameraID(i);


    return openWithParams();
}

bool ofxZED::Camera::openWithParams() {



    if (sl::Camera::isOpened()) {
        sl::Camera::close();
    } else {
        ofAddListener(ofGetWindowPtr()->events().exit, this, &ofxZED::Camera::close);
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

uint64_t ofxZED::Camera::getFrameTimestamp() {
    return  sl::Camera::getTimestamp(sl::TIME_REFERENCE_IMAGE);
}
uint64_t ofxZED::Camera::getLastTimestamp() {
    return  sl::Camera::getTimestamp(sl::TIME_REFERENCE_LAST);
}

void ofxZED::Camera::close(ofEventArgs &args) {
    sl::Camera::close();
}

void ofxZED::Camera::close() {
    sl::Camera::close();
}

void ofxZED::Camera::draw(ofRectangle r, bool left, bool right, bool depth) {

    int w = getWidth();
    int h = getHeight();


    stereoAlternate = !stereoAlternate;
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    if (leftTex.isAllocated() && rightTex.isAllocated() && left && right) {
        int offset = (float)(stereoOffset * (float)(w*0.01));
        ofRectangle lr = r;
        ofRectangle rr = r;
        lr.x += offset;
        rr.x -= offset;
        (stereoAlternate) ? leftTex.draw(lr) : rightTex.draw(rr);
    }
    if (leftTex.isAllocated() && !rightTex.isAllocated() && left && !right) leftTex.draw(r);
    if (!leftTex.isAllocated() && rightTex.isAllocated() && !left && right) rightTex.draw(r);
    ofEnableBlendMode(OF_BLENDMODE_SCREEN);
    if (depthTex.isAllocated() && depth) depthTex.draw(r);
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}

