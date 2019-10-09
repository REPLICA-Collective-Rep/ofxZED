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

    void Database::load(string databaseLocation, string databaseName, bool withLookup) {

        string loadPath = ofFilePath::join(databaseLocation, databaseName);
        ofLogNotice("ofxZED::Database") << "loading json database" << loadPath;
        json = ofLoadJson( loadPath + ".json");
        isForcingRecreate = false;

        dir.allowExt("svo");
        dir.open(databaseLocation);
        dir.listDir();

        ofLogNotice("ofxZED::Database") << "loading database";
        for (auto f : dir.getFiles()) {
            process(f, withLookup);
        }

        ofLogNotice("ofxZED::Database") << "finished initing database" << data.size();
        ofLogNotice("ofxZED::Database") << "sorting by date";

        ofSort(data, SVO::sortSVO);
        if (zed.isOpened()) zed.close();
    }

    void Database::finish() {


    }


    void Database::build(string location, string fileName, bool withLookup, bool forceRecreate) {

        ofLogNotice("ofxZED::Database") << "opening json database";
        isForcingRecreate = forceRecreate;
        dir.allowExt("svo");
        dir.open(location);
        dir.listDir();

        totalFiles = dir.getFiles().size();
        currIndex = 0;
        totalFrames = 0;

        directoryPath = dir.getAbsolutePath();
        databaseName = fileName;
        string loadPath = ofFilePath::join(directoryPath, databaseName);
        json = ofLoadJson(loadPath + ".json");

        ofLogNotice("ofxZED::Database") << "initing database";
        for (auto f : dir.getFiles()) process(f, withLookup);

        ofLogNotice("ofxZED::Database") << "finished initing database" << data.size();
        ofLogNotice("ofxZED::Database") << "sorting by date";

        ofSort(data, SVO::sortSVO);
        if (zed.isOpened()) zed.close();
        write(directoryPath, databaseName);

    }


    void Database::scrape(ofFile & f, vector<Frame> & frames, vector<int> & lookup) {

        frames.clear();
        lookup.clear();

        ofLogNotice("ofxZED::Database") << "beginning scrape";

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
                float fps = zed.getCameraFPS();

                if (frameIdx != i) {
                    ofLogError("ofxZED::Database") << "something went wrong:" << frameIdx << "does not equal" << i;
                }
               frames.push_back( Frame(frameIdx, timestamp) );
               int repetitions = 0;
               if (i > 0) {
                   float millis = ofxZED::SVO::getDurationMillis(frames[i-1].timestamp, timestamp);
                   float fpsMillis = 1000.0/fps;
                   float effectiveFps = 1000.0/millis;
                   repetitions =  round(millis/fpsMillis);
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

    void Database::process(ofFile & f, bool withLookup) {

        bool recreateLookups = false;


        if (json["files"].find(f.getFileName()) != json["files"].end() && !isForcingRecreate) {

            ofLogNotice("ofxZED::Database") << "loading svo entry with lookup:" << withLookup;

            SVO svo;
            svo.init(json["files"][f.getFileName()]);
            data.push_back(svo);
            string lookupPath = data.back().getLookupPath();
            ofFile file(lookupPath);
            bool hasLookup = file.exists();
            if (!hasLookup && withLookup) {



                ofLogNotice("ofxZED::Database") << "lookup table not found, creating new..." << svo.getLookupPath();
                recreateLookups = true;
                if (zed.openSVO(f.getAbsolutePath())) {
                    ofLogNotice("ofxZED::Database") << "creating lookup table for entry" << svo.getLookupPath();
                    data.back().scrape(zed);
                } else {
                    ofLogError("ofxZED::Database") << "could not open" << f.getAbsolutePath();
                    OF_EXIT_APP(0);
                }
            } else if (withLookup) {

                ofLogNotice("ofxZED::Database") << "loading lookup table" << svo.getLookupPath();

                svo.loadLookup();
            }


        } else {


           ofLogNotice("ofxZED::Database") << "creating database entry from scratch";


            if (zed.openSVO(f.getAbsolutePath())) {

                SVO svo;
                svo.init(f, zed.getCameraFPS());
                svo.scrape(zed);
                svo.printInfo();
                data.push_back(svo);
                totalFrames += svo.getTotalFrames();
                recreateLookups = true;

            } else {
                ofLogError("ofxZED::Database") << "could not open" << f.getAbsolutePath();
                OF_EXIT_APP(0);
            }


            write(directoryPath, databaseName);
        }

        if (recreateLookups) {
            ofJson lookupJson = data.back().getJson(true);
            ofSaveJson(data.back().getLookupPath(), lookupJson);
        }
        currIndex += 1;


    }
    void Database::write(string dirPath, string dbName) {


        float ts = ofGetElapsedTimef();

        string csv = "Filename,Path,FPS,Timestamps,Lookup\n";


        ofSort(data, SVO::sortSVO);
        for ( auto & d : data) {
            json["files"][d.filename] = d.getJson(false);
            csv += d.getCSV();
        }

        ofBuffer buff;
        buff.set(csv.c_str(), csv.size());
        string savePath = ofFilePath::join(dirPath, dbName);
        ofSaveJson(savePath + ".json" , json);
        ofBufferToFile(savePath + ".csv", buff);

        ofLogNotice("ofxZED::Database") << "writing db took" << ofGetElapsedTimef() - ts << "seconds to" << savePath;

    }


    vector<SVO *> Database::getFilteredByRange( uint64_t start, uint64_t end) {

        if (data.size() <= 0) {
            ofLogError("ofxZED::Database") << "database is not loaded or is empty";
        }

        typedef std::chrono::system_clock::time_point time_point;
        typedef ofxZED::SVO S;
        string format = "%H:%M";
        vector<SVO *> db;
        for (auto & d : data) {


            bool hasStartIn = (d.getStart() >= start  && d.getStart() <= end);
            bool hasEndIn = (d.getEnd() >= start && d.getEnd() <= end);
            bool hasWrapped = (d.getStart() < start  && d.getEnd() > end);
            if (hasStartIn || hasEndIn || hasWrapped) {
                db.push_back(&d);
            }
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
        ofSort(db, ofxZED::SVO::sortSVOPtrs);
        return db;
    }

    vector<SVO *> Database::getPtrsInsideTimestamp(uint64_t time) {
        vector<SVO *> db;
//        int DIV = 1000000000;
        for (auto & d : data) if (time >= d.getStart() && time < d.getEnd() ) db.push_back(&d);
        return db;
    }



}
