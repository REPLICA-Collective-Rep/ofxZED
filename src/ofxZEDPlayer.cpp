#include "ofxZEDPlayer.h"

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


namespace ofxZED {

    Player::Player() {
        isSettingPosition = false;
    }

    void Player::init() {

//#ifdef TARGET_OPENGLES
//    shader.load("shadersES2/shader");
//#else
//    if(ofIsGLProgrammableRenderer()){
//        shader.load("shadersGL3/shader");
//    }else{
//        shader.load("shadersGL2/shader");
//    }
//#endif

    }


    void Player::nudge( int frames ) {
        sl::Camera::setSVOPosition( sl::Camera::getSVOPosition() + frames );
    }

    void Player::setSVOPosition(int i ) {
        if (!isSettingPosition) {
            isSettingPosition = true;
            sl::Camera::setSVOPosition(i);
        }


    }

    bool Player::openSVO(SVO * svo_) {
        svo = svo_;
        return ofxZED::Camera::openSVO(svo->getSVOPath());
    }

    int Player::grab(bool left, bool right, bool depth) {


        if (!sl::Camera::isOpened()) return  sl::Camera::getSVOPosition();

        frameNew = false;

        sl::RuntimeParameters runtime_parameters;
        runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL; // Use STANDARD sensing mode
        runtime_parameters.enable_depth = depth;

        if (sl::Camera::grab(runtime_parameters) == sl::SUCCESS) {

            frameNew = true;

            int w = getWidth();
            int h = getHeight();

            if (sl::Camera::getSVOPosition() == lastPosition)  sl::Camera::getSVOPosition();

            sl::Camera::retrieveImage(leftMat, sl::VIEW_LEFT, sl::MEM_CPU, w,h);
            sl::Camera::retrieveImage(depthMat, sl::VIEW_DEPTH, sl::MEM_CPU, w, h);


//            sl::Camera::retrieveMeasure(measureMat, sl::MEASURE::MEASURE_DEPTH, sl::MEM_CPU, w, h);
//            depthMat.normalizeMeasure(sl::MEASURE::MEASURE_DEPTH, 0, 20000);

            if (!leftPix.isAllocated()) {
                //leftPix.allocate(w, h, 3);
                //leftTex.allocate(w, h, GL_RGB, false);
            }
            if (!depthPix.isAllocated()) {
//                depthPix.allocate(w, h, 4);
//                depthTex.allocate(w, h, GL_RGB, false);
            }
//            if (!measurePix.isAllocated()) {
//                measurePix.allocate(w, h, 1);
//                measureTex.allocate(w, h, GL_LUMINANCE, false);
//            }


//            leftPix.setFromPixels( leftMat.getPtr<sl::uchar1>(), w, h, OF_PIXELS_BGRA );
//            depthPix.setFromPixels( depthMat.getPtr<sl::uchar1>(), w, h, OF_PIXELS_BGRA);



            processMatToPix( leftPix, leftMat, false);
            processMatToPix( depthPix, depthMat, false );



//            measurePix.setFromPixels( measureMat.getPtr<sl::uchar1>(), w, h, OF_PIXELS_GRAY);


            leftTex.loadData(leftPix.getData(), w, h, GL_RGB);
            depthTex.loadData(depthPix.getData(), w, h, GL_RGB);




//            measureTex.loadData(measurePix);


//            sl::Mat zedView;
//            retrieveMeasure(zedView, sl::MEASURE_XYZRGBA);
//            int step = zedView.getStep() / 4;
//            int step_char = zedView.getStep();
//            pointCloud_.resize(w*h);
//            pointCloudColors_.resize(w*h);
//            pointCloudFloatColors_.resize(w*h);
//            mesh.clear();

//            float *data = (zedView.getPtr<sl::float1>());
//            unsigned char *data_char = zedView.getPtr<sl::uchar1>();

//            for (int y = 0; y < h; y++) {
//                for (int x = 0; x < w; x++) {
//                    int index = (x + w*y) * 4; //formerly step * y
//                    int index_color = (index + 3) *4;

//                    pointCloud_[x + w*y] = ofVec3f(data[index], data[index + 1], data[index + 2]);
//                    mesh.addVertex( pointCloud_[x + w*y ]);
//                    pointCloudColors_[x + w*y] = ofColor(data_char[index_color], data_char[index_color + 1], data_char[index_color + 2], data_char[index_color + 3]);
//                    mesh.addColor( ofFloatColor(data_char[index_color], data_char[index_color + 1], data_char[index_color + 2], data_char[index_color + 3])  );
//                }
//            }

//            size_t n = pointCloudColors_.size();
//            pointCloudFloatColors_.resize(n);
//            for (size_t i = 0; i < n; i++) {
//                pointCloudFloatColors_[i] = pointCloudColors_[i];
//            }
//            lastPosition = sl::Camera::getSVOPosition();
//            mesh.addVertices(pointCloud_);
//            if (pointCloudFloatColors_.size() == pointCloud_.size()) mesh.addColors(pointCloudFloatColors_);



            if (isSettingPosition) isSettingPosition = false;

        } else {

            ofLog() << "Did not grab";
        }

        return sl::Camera::getSVOPosition();
    }




}
