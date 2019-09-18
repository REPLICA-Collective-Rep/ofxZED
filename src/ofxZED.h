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


    class Camera : public sl::Camera {
    public:

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

        void draw(ofRectangle r);

    };

    class SVO {
    public:
        string filename;
        int fps;
        ofVec2f resolution;
        uint64_t startTime;
        uint64_t endTime;
        int durationSeconds;
        int durationMinutes;
        int predictedFrames;
        int totalFrames;
        int droppedFrames;
        int averageFPS;

        string droppedAmount, startHuman, endHuman;

        SVO() { }

        void init( string filename_, int fps_, ofVec2f resolution_, int totalFrames_, sl::timeStamp startTime_, sl::timeStamp endTime_);
        void init( ofJson j );

        string getDescriptionStr();
        string getCSV();
        ofJson getJson();
    };

    class Database {
    private:
        Camera zed;
        string saveLocation;
        ofDirectory dir;
        int currIndex;

        void process(ofFile & f);
        void write();
    public:


        int totalFiles;
        int totalFrames;
        vector<SVO> data;
        ofJson json;
        string csv;
        Database() { };
        void init(string location, string fileName = "_database");
        std::map<string, vector<SVO *>> getSortedByDay();
        vector<SVO *> getPtrs();
        vector<SVO *> getPtrsInsideTimestamp(int time);
    };


    class Player : public ofxZED::Camera {
    public:
        SVO * svo;
        bool openSVO(string root, SVO * svo_);
        void setPositionFromTimestamp(int time);
        void grab(bool left, bool right, bool depth);
        void drawStereoscopic(ofRectangle r);
    };



    static string humanTimestamp(uint64_t & timestamp, string format = "Date %Y-%m-%d Time %H:%M:%S ");
    static std::time_t TimestampToTimeT(sl::timeStamp timestamp);

    static int TimestampToInt(uint64_t timestamp);
    static int TimestampToDurationSeconds(sl::timeStamp end, sl::timeStamp start);
    static int getDurationInMillis(float fps_, float totalFrames_);
    bool sortSVO(SVO & a, SVO & b);
    bool sortSVOPtrs(SVO * a, SVO * b);
    static float getDurationInSeconds(float fps_, float totalFrames_);
    static float getDurationInMinutes(float fps_, float totalFrames_);
    static float getDurationInHours(float fps_, float totalFrames_);
    static int getHumanSeconds(float fps_, float totalFrames_);
    static int getHumanMinutes(float fps_, float totalFrames_);
    static int getHumanHours(float fps_, float totalFrames_);
    static string getDurationInHuman(float fps_, float totalFrames_);


}

