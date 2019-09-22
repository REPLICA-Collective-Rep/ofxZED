#pragma once

#include "ofMain.h"
#include <sl/Camera.hpp>


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



}

