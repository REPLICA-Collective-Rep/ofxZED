#pragma once

#include "ofMain.h"
#include <sl/Camera.hpp>
#include "ofxZEDCamera.h"
#include "ofxPose.h"


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
    private:
        vector<int> lookup;
    public:


        ofxPose::Animation poses;
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

        /*-- convert timestamp to chrono time_point --*/

        static std::chrono::system_clock::time_point getTimePoint( uint64_t timestamp );

        /*-- get timestamp from string, default format as "21/06/2019 14:24:00" --*/
        static uint64_t getTimestampFromStr(string date, string format = "%d/%m/%Y %H:%M:%S");

        /*-- increment seconds to a timestamp --*/
        static void incrementSeconds(uint64_t & timestamp, float seconds);

        /*-- returns milliseconds between two timestamps --*/
        static int getDurationMillis(uint64_t start, uint64_t end);

        /*-- maps a float range into a timestamp range, preserving fidelity --*/
        static uint64_t mapToTimestamp(float value, float from, float to, uint64_t start, uint64_t end, bool constrain = false);

        /*-- maps a timestamp into a float range --*/
        static float mapFromTimestamp(uint64_t timestamp, uint64_t start, uint64_t end, float from, float to, bool constrain);

        /*-- returns human-readable duration between two timestamps --*/
        static string getHumanDuration(uint64_t start, uint64_t end, string format = "%H-%M-%S-%.");

        /*-- formats timestamp into a human-readable string --*/
        static string getHumanTimestamp(uint64_t timestamp, string format = "%Y/%m/%d %H:%M:%S:%.");


        static bool sortSVO(SVO & a, SVO & b);
        static bool sortSVOPtrs(SVO * a, SVO * b);


        /*-- info --*/

        int getPredictedFrames();
        int getTotalFrames();
        int getTotalLookupFrames();
        int getLookupLength();
        int getLookupIndexFromTimestamp(uint64_t time);
        int getLookupIndex(int i);


        bool threadPoses_, threadLookup_;

        void checkForLookup();
        void checkForPoses();

        bool hasPoseIdx(int i );
        bool hasLookupIdx(int i );
        bool hasTimestampIdx(int i );

        bool hasLookupFile();
        bool hasPosesFile();

        void scrape(ofxZED::Camera & zed);

        float getAverageFPS();
        string getDroppedPercent();
        string printInfo();
        string getLookupPath();
        string getPosesPath();
        string getSVOPath();
        string getName();

        void loadPoses();
        void loadLookup();

        /*-- formats --*/

        string getCSV();
        ofJson getJson(bool withTables);
    };



}

