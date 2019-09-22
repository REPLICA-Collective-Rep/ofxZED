#pragma once

#include "ofMain.h"
#include "ofxZEDSVO.h"
#include "ofxZEDCamera.h"


namespace ofxZED {

    class Database {
    private:


        Camera zed;
        string directoryPath;
        string databaseName;
        ofDirectory dir;
        int currIndex;

        void finish();
        void scrape(ofFile & f, vector<Frame> & frames, vector<int> & lookup);
        void process(ofFile & f);
    public:

        bool isForcingRecreate;
        int totalFiles;
        int totalFrames;
        vector<SVO> data;
        ofJson json;
        string csv;
        Database() { };

        void build(string location, bool forceRecreate = false, string fileName = "_database");
        void load(string databaseLocation, string databaseName);
        void write(string dirPath, string dbName);

        vector<SVO *> getFilteredByRange( uint64_t start, uint64_t end);
        std::map<string, vector<SVO *>> getSortedByDay(vector<SVO *> svos);
        std::map<string, vector<SVO *>> getSortedBySerialNumber(vector<SVO *> svos);
        vector<SVO *> getPtrs();
        vector<SVO *> getPtrsInsideTimestamp(uint64_t time);
    };


}

