#pragma once

#include "ofMain.h"
#include <sl/Camera.hpp>

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

    struct Frame {
    public:
        uint64_t timestamp;
        int frame;
        Frame( int f, uint64_t t) {
            timestamp = t;
            frame = f;
        }
    };

    class SVO {
    public:

        vector<int> lookup;
        vector<Frame> frames;
        string filename;
        string path;
        int fps;

        SVO() { }

        void init( ofFile & f, int fps_);
        void init( ofJson j );

        /*-- util --*/

        uint64_t getStart();
        uint64_t getEnd();

        /*-- time util --*/

        /*-- increment seconds to a timestamp --*/
        static void incrementSeconds(uint64_t & timestamp, float seconds);

        /*-- returns milliseconds between two timestamps --*/
        static int getDurationMillis(uint64_t start, uint64_t end);

        /*-- maps a float range into a timestamp range, preserving fidelity --*/
        static uint64_t mapToTimestamp(float value, float from, float to, uint64_t start, uint64_t end, bool constrain = false);

        /*-- maps a timestamp into a float range --*/
        float mapFromTimestamp(uint64_t timestamp, uint64_t start, uint64_t end, float from, float to, bool constrain);

        /*-- returns human-readable duration between two timestamps --*/
        static string getHumanDuration(uint64_t start, uint64_t end, string format = "%H-%M-%S-%.");

        /*-- formats timestamp into a human-readable string --*/
        static string getHumanTimestamp(uint64_t timestamp, string format = "%Y-%m-%d_%H-%M-%S-%.");


        static bool sortSVO(SVO & a, SVO & b);
        static bool sortSVOPtrs(SVO * a, SVO * b);


        /*-- info --*/

        int getPredictedFrames();
        int getTotalFrames();
        int getLookupLength();
        float getAverageFPS();
        string getDroppedPercent();
        string printInfo();

        /*-- formats --*/

        string getCSV();
        ofJson getJson();
    };

    class Camera : public sl::Camera {
    public:


        static string humanTimestamp(uint64_t & timestamp, string format = "Date %Y-%m-%d Time %H:%M:%S ");
        static std::time_t TimestampToTimeT(sl::timeStamp timestamp);


        int lastPosition;
        float stereoOffset;
        bool stereoAlternate;
        ofTexture leftTex, rightTex, depthTex;
        ofPixels leftPix, rightPix, depthPix;
        sl::Mat leftMat, rightMat, depthMat;
        sl::InitParameters init;
        int frameCount = 0;
        bool isRecording = false;
        bool frameNew = false;

        Camera();

        int getSerialNumber();

        void logSerial();

        void processMatToPix(ofPixels & pix, sl::Mat & mat, bool psychedelic = false);


        string getTimestamp(string format = "%Y-%m-%d_%H:%M:%S");

        void record(string path, bool b, bool autoPath = true);

        void toggleRecording(string path, bool autoPath = true);
        void updateRecording();
        bool isFrameNew();

        ofQuaternion eulerToQuat(const ofVec3f & rotationEuler);

        int getWidth();
        int getHeight();


        bool openSVO(string svoPath);

        bool openCamera(int i = -1);

        bool openWithParams();

        uint64_t getFrameTimestamp();
        uint64_t getLastTimestamp();

        void close(ofEventArgs &args);
        void close();

        void draw(ofRectangle r, bool left = true, bool right = false, bool depth = false);

    };
    class Player : public ofxZED::Camera {
    public:
        SVO * svo;
        bool openSVO(string root, SVO * svo_);
        int grab(bool left, bool right, bool depth);
        void drawStereoscopic(ofRectangle r);
    };

    class Database {
    private:


        Camera zed;
        string saveLocation;
        ofDirectory dir;
        int currIndex;

        void scrape(ofFile & f, vector<Frame> & frames, vector<int> & lookup);
        void process(ofFile & f);
        void write();
    public:

        bool isCreatingBins;
        int totalFiles;
        int totalFrames;
        vector<SVO> data;
        ofJson json;
        string csv;
        Database() { };

        void init(string location, bool recreate, string fileName = "_database");

        std::map<string, vector<SVO *>> getSortedByDay(vector<SVO *> svos);
        std::map<string, vector<SVO *>> getSortedBySerialNumber(vector<SVO *> svos);
        vector<SVO *> getPtrs();
        vector<SVO *> getPtrsInsideTimestamp(uint64_t time);
    };





}

