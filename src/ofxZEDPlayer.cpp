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
        left = true;
        right = false;
        depth = false;
        cloud = false;
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

    int Player::grab() {



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

//            ofLog() << "retrieving..." << w << h;

            if (left) {

                if (!leftPix.isAllocated()) {
                    leftPix.allocate(w, h, 3);
                    leftTex.allocate(w, h, GL_RGB, false);
                }
                sl::Camera::retrieveImage(leftMat, sl::VIEW_LEFT, sl::MEM_CPU, w,h);
                leftPix.setFromPixels( leftMat.getPtr<sl::uchar1>(), w, h, OF_PIXELS_BGRA );
                leftTex.loadData(leftPix);
            }

            if (right) {

                if (!rightPix.isAllocated()) {
                    rightPix.allocate(w, h, 3);
                    rightTex.allocate(w, h, GL_RGB, false);
                }
                sl::Camera::retrieveImage(rightMat, sl::VIEW_RIGHT, sl::MEM_CPU, w,h);
                rightPix.setFromPixels( rightMat.getPtr<sl::uchar1>(), w, h, OF_PIXELS_BGRA );
                rightTex.loadData(rightPix);
            }

            if (depth) {

                if (!depthPix.isAllocated()) {
                    depthPix.allocate(w, h, 3);
                    depthTex.allocate(w, h, GL_RGB, false);
                }
                sl::Camera::retrieveImage(depthMat, sl::VIEW_DEPTH, sl::MEM_CPU, w, h);
                depthPix.setFromPixels( depthMat.getPtr<sl::uchar1>(), w, h, OF_PIXELS_BGRA );
                depthTex.loadData(depthPix);
            }

            if (cloud) {
               sl::Camera::retrieveMeasure(cloudMat, sl::MEASURE_XYZRGBA, sl::MEM_CPU, w, h);
               mesh.clear();

               float *data = (cloudMat.getPtr<sl::float1>());
               unsigned char *data_char = cloudMat.getPtr<sl::uchar1>();
               vector<glm::vec3> points;
               vector<ofFloatColor> colors;

               for (int y = 0; y < h; y++) {
                   for (int x = 0; x < w; x++) {
                       int index = (x + w * y) * 4;
                       int index_color = (index + 3) *4;

//                       mesh.addVertex(ofVec3f(data[index], data[index + 1], data[index + 2]));
//                       mesh.addColor( ofFloatColor(data_char[index_color], data_char[index_color + 1], data_char[index_color + 2], data_char[index_color + 3])  );
                       points.push_back(glm::vec3(data[index], data[index + 1], data[index + 2]));
                       colors.push_back( ofFloatColor(data_char[index_color], data_char[index_color + 1], data_char[index_color + 2], data_char[index_color + 3])  );
                   }
               }

               mesh.addVertices(points);
               mesh.addColors(colors);
            }


















            lastPosition = sl::Camera::getSVOPosition();

            isSettingPosition = false;

        } else {

            ofLog() << "Did not grab";
        }

        return sl::Camera::getSVOPosition();

    }




}
