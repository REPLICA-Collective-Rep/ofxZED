#include "ofxZEDDatabase.h"

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


    /*-- Databse --*/



    void Database::init(string location, bool recreate, string fileName) {

        ofLogNotice("ofxZED::Database") << "opening json database";
        isCreatingBins = recreate;
        dir.allowExt("svo");
        dir.open(location);
        dir.listDir();

        totalFiles = dir.getFiles().size();
        currIndex = 0;
        totalFrames = 0;

        saveLocation = dir.getAbsolutePath() + "/" + fileName;
        json = ofLoadJson(saveLocation + ".json");


        for (auto f : dir.getFiles()) process(f);

        ofLogNotice("ofxZED::Database") << "finished initing database";
        ofLogNotice("ofxZED::Database") << "sorting by date";

        ofSort(data, SVO::sortSVO);
        if (zed.isOpened()) zed.close();
    }


    void Database::scrape(ofFile & f, vector<Frame> & frames, vector<int> & lookup) {

        frames.clear();
        lookup.clear();

        zed.setSVOPosition(0);
        int total = zed.getSVONumberOfFrames();

        for (int i = 0; i < total; i++) {

            float tt = ofGetElapsedTimef();
            bool timeout = false;
            while ( zed.grab() != sl::SUCCESS && !timeout )  {
                if (ofGetElapsedTimef() > tt + 2) {
                    ofLogError("ofxZED::Database") << "timout";
                    timeout = true;
                }
                sl::sleep_ms(1);
            }

            if (!timeout) {

                uint64_t timestamp = zed.getFrameTimestamp();
                int frameIdx = zed.getSVOPosition()-1;
                int fps = zed.getCameraFPS();

                if (frameIdx != i) {
                    ofLogError("ofxZED::Database") << "something went wrong:" << frameIdx << "does not equal" << i;
                }
               frames.push_back( Frame(frameIdx, timestamp) );
               int repetitions = 0;
               if (i > 0) {
                   int millis = ofxZED::SVO::getDurationMillis(frames[i-1].timestamp, timestamp);
                   int fpsMillis = 1000/fps;
                   int effectiveFps = 1000/millis;
                   repetitions =  millis/fpsMillis;
                   for (auto _ = repetitions; _--;) lookup.push_back(i);
               }

               bool printProgress = false;

               if (printProgress) {
                string actualStr = ofToString(i)+"/"+ofToString(total);
                std::cout.flush();
                std::cout << "... lookup table: "+ofToString(lookup.size())+" "+actualStr+"";
                std::cout.flush();
               }
            }
        }


    }

    void Database::process(ofFile & f) {


        if (json["files"].find(f.getFileName()) != json["files"].end() && !isCreatingBins) {

            SVO svo;
            svo.init(json["files"][f.getFileName()]);
            data.push_back(svo);

        } else {


           ofLogNotice("ofxZED::Database") << "opening svo";


            if (zed.openSVO(f.getAbsolutePath())) {

                SVO svo;
                scrape(f, svo.frames, svo.lookup);
                svo.init(f, zed.getCameraFPS());
                svo.printInfo();
                data.push_back(svo);
                totalFrames += svo.getTotalFrames();


            } else {
                ofLogError("ofxZED::Database") << "could not open" << f.getAbsolutePath();
                OF_EXIT_APP(0);
            }
            write();
        }

        ofLogNotice("ofxZED::Database") << "loading" << currIndex+1 << "/" << dir.getFiles().size();

        currIndex += 1;


    }
    void Database::write() {


        float ts = ofGetElapsedTimef();

        string csv = "Filename,FPS,Timestamps,Lookup\n";


        ofSort(data, SVO::sortSVO);
        for ( auto & d : data) {
            json["files"][d.filename] = d.getJson();
            csv += d.getCSV();
        }

        ofBuffer buff;
        buff.set(csv.c_str(), csv.size());
        ofSaveJson(saveLocation + ".json" , json);
        ofBufferToFile(saveLocation + ".csv", buff);

        ofLogNotice("ofxZED::Database") << "writing db took" << ofGetElapsedTimef() - ts << "seconds";

    }


    vector<SVO *> Database::getFilteredByRange( uint64_t start, uint64_t end) {

        if (data.size() <= 0) {
            ofLogError("ofxZED::Database") << "database is not loaded or is empty";
        }

        typedef std::chrono::system_clock::time_point time_point;

        time_point tps = ofxZED::SVO::getTimePoint(start);
        time_point tpe = ofxZED::SVO::getTimePoint(end);

        vector<SVO *> db;
        for (auto & d : data) {

            time_point tpstart = ofxZED::SVO::getTimePoint(d.getStart());
            time_point tpend = ofxZED::SVO::getTimePoint(d.getEnd());

            bool hasStart = (tps > tpstart  && tps < tpend);
            bool hasEnd = (tpe > tpstart && tpe < tpend);
            if (hasStart || hasEnd) db.push_back(&d);
        }
        return db;

    }

    std::map<string, vector<SVO *>> Database::getSortedByDay(vector<SVO *> svos) {
        std::map<string, vector<SVO *>> db;
        for (auto & d : svos) {
            string date = SVO::getHumanTimestamp(d->getStart(), "%A %d %b");
            if (db.find(date) == db.end()) db[date] = {};
            db[date].push_back(d);
        }
        for (auto & d : db) ofSort(d.second, SVO::sortSVOPtrs);
        return db;
    }
    std::map<string, vector<SVO *>> Database::getSortedBySerialNumber(vector<SVO *> svos) {
        std::map<string, vector<SVO *>> db;
        for (auto & d : svos) {
            string serial = d->filename.substr(0, d->filename.find("_"));
            if (db.find(serial) == db.end()) db[serial] = {};
            db[serial].push_back(d);
        }
        for (auto & d : db) ofSort(d.second, SVO::sortSVOPtrs);
        return db;
    }
    vector<SVO *> Database::getPtrs() {
        vector<SVO *> db;
        for (auto & d : data) db.push_back(&d);
        return db;
    }

    vector<SVO *> Database::getPtrsInsideTimestamp(uint64_t time) {
        vector<SVO *> db;
//        int DIV = 1000000000;
        for (auto & d : data) if (time >= d.getStart() && time < d.getEnd() ) db.push_back(&d);
        return db;
    }



}
