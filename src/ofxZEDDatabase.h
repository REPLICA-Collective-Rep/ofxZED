#pragma once

#include "ofMain.h"
#include "ofxZEDSVO.h"
#include "ofxZEDCamera.h"


namespace ofxZED {

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

        void init(string location, bool recreate = false, string fileName = "_database");

        vector<SVO *> getFilteredByRange( uint64_t start, uint64_t end);
        std::map<string, vector<SVO *>> getSortedByDay(vector<SVO *> svos);
        std::map<string, vector<SVO *>> getSortedBySerialNumber(vector<SVO *> svos);
        vector<SVO *> getPtrs();
        vector<SVO *> getPtrsInsideTimestamp(uint64_t time);
    };


}

