#include "ofxZEDSVO.h"

namespace ofxZED {

    void SVO::scrape(ofxZED::Camera & zed) {
        frames.clear();
        lookup.clear();

        ofLogNotice("ofxZED::SVO") << "beginning scrape";

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

    bool SVO::hasPoseIdx(int i ) {
        return i < poses.frames.size();
    }
    bool SVO::hasLookupIdx(int i ) {
        return i < lookup.size();
    }
    bool SVO::hasTimestampIdx(int i ) {
        return i < frames.size();
    }
    string SVO::getName() {
        return filename.substr(0, path.size() - 4);
    }
    string SVO::getLookupPath() {
         return path.substr(0, path.size() - 4) + ".lookup";
    }
    string SVO::getPosesPath() {
         return path + ".poses";
    }
    string SVO::getSVOPath() {
         return path;
    }

    bool SVO::hasLookupFile() {

        return ofFile::doesFileExist(getLookupPath(), false);
    }
    bool SVO::hasPosesFile() {

        return ofFile::doesFileExist(getPosesPath(), false);

    }
    void SVO::checkForPoses() {
        if (poses.frames.size() <= 3 && hasPosesFile()) {
            ofLogNotice("ofxZED::SVO") << "poses not loaded yet";
            loadPoses();
        }

    }
    void SVO::checkForLookup() {
        if (lookup.size() <= 3 && hasLookupFile()) {

            ofLogNotice("ofxZED::SVO") << "lookup not loaded yet";
            loadLookup();
        }

    }

    int SVO::getLookupIndex(int i) {
        if (!hasLookupIdx(i)) {
            ofLogError("ofxZED::SVO") << "no lookup at this index";
            return 0;
        }
        return lookup[i];
    }

    void SVO::loadPoses() {
        ofLogNotice("ofxZED::SVO") << "loading .svo.poses" << getPosesPath();

        ofJson j = ofLoadJson(getPosesPath());
        ofLogNotice("ofxZED::SVO") << "parsing .svo.poses" << j["frames"].size() << "frames";

        for (auto & frame : j["frames"]) {
            ofxPose::Frame frame_;

            if (!frame.is_null()) {
                for (auto & person : frame) {
                    ofxPose::Person person_;
                    for (auto & joint : person.items()) {

                        try  {

                            int key = ofToInt( joint.key() );
                            float x = joint.value()[0].get<float>();
                            float y = joint.value()[1].get<float>();
                            float z = joint.value()[2].get<float>();
                            float weight = joint.value()[3].get<float>();

                            /*-- if is centre of gravity --*/

                            if (key == -1) person_.center = ofVec3f(x,y,z);

                            /*-- if is joint --*/

                            if (key != -1)  {
                                int to = joint.value()[4].get<int>();
                                person_.add( key, ofxPose::Joint(key, ofVec3f(x,y,z), to, weight) );
                                frame_.raw.push_back( ofVec3f(x,y,z) );
                            }
                        } catch (int e) {
                            ofLogError("ofxZED::SVO") << "pose error on joint" << joint;
                        }

                    }
                    frame_.add( person_ );
                }
            } else {

            }

            poses.add( frame_ );
        }
        ofLogNotice("ofxZED::SVO") << "success .svo.poses" << j["frames"].size() << "frames";
    }

    void SVO::loadLookup() {
        ofLogNotice("ofxZED::SVO") << "loading lookup table" << getLookupPath();
        ofJson j = ofLoadJson(getLookupPath());
        init(j);
    }

    void SVO::init( ofFile &f, int fps_) {
        filename = f.getFileName();
        path = f.getAbsolutePath();
        fps = fps_;
    }

    int SVO::getTotalLookupFrames() {
        return lookup.size();
    }

    int SVO::getLookupIndexFromTimestamp(uint64_t time) {
        checkForLookup();
        int idx = mapFromTimestamp(time, getStart(), getEnd(), 0, getTotalLookupFrames(), true);
        return lookup[idx];
    }

    int SVO::getTotalFrames() {
        return frames.size();
    }
    string SVO::getDroppedPercent() {
        return ofToString((int)(100-((100.0/getPredictedFrames()) * getTotalFrames())))+ "%";
    }
    uint64_t SVO::getStart() {
        if (frames.size() <= 0) {
            ofLogError("ofxZED::SVO") << "no frames to get start time";
            return 0;
        }
        return frames.front().timestamp;
    }
    uint64_t SVO::getEnd() {
        if (frames.size() <= 0) {
            ofLogError("ofxZED::SVO") << "no frames to get end time";
            return 0;
        }
        if ( frames.back().timestamp < frames.front().timestamp ) {
            ofLogError("ofxZED::SVO") << "end timestamp is incorrect";
        }
        return frames.back().timestamp;
    }
    int SVO::getPredictedFrames() {
        return  ((float)(getDurationMillis(getStart(), getEnd())/1000.0) * (float)fps);
    }

    std::chrono::system_clock::time_point SVO::getTimePoint( uint64_t timestamp ) {

        typedef std::chrono::milliseconds milliseconds;
        typedef std::chrono::system_clock::time_point time_point;
        typedef std::chrono::system_clock::time_point::duration duration;
        typedef std::chrono::nanoseconds nano_seconds;
        time_point tp{std::chrono::duration_cast<duration>(nano_seconds(timestamp))};
        return tp;
    }

    /*-- get timestamp from string, default format as "21/06/2019 14:24:00" --*/

    uint64_t SVO::getTimestampFromStr(string date, string format) {

        typedef std::chrono::system_clock::time_point time_point;
        typedef std::chrono::nanoseconds nano_seconds;
        typedef std::chrono::milliseconds milliseconds;
        typedef std::chrono::microseconds microseconds;
        typedef std::chrono::seconds seconds;

        /*-- account for daylight saving w. tm_isdst --*/

        std::tm tm = {0};
        std::stringstream ss(date);
        ss >> std::get_time(&tm, format.c_str());
        tm.tm_isdst = -1;

        /*-- convert to chrono --*/

        time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

        /*-- count to uint64 --*/

        auto out = std::chrono::duration_cast<nano_seconds>(tp.time_since_epoch()).count();
        return out;
    }

    /*-- increment seconds to a timestamp --*/

    void SVO::incrementSeconds(uint64_t & timestamp, float seconds) {
        timestamp += (1000000000.0 * seconds);
    }

    /*-- returns milliseconds between two timestamps --*/

    int SVO::getDurationMillis(uint64_t start, uint64_t end) {

        typedef std::chrono::milliseconds milliseconds;
        typedef std::chrono::system_clock::time_point time_point;
        typedef std::chrono::system_clock::time_point::duration duration;
        typedef std::chrono::nanoseconds nano_seconds;

        time_point ee{std::chrono::duration_cast<duration>(nano_seconds(end))};
        time_point ss{std::chrono::duration_cast<duration>(nano_seconds(start))};
        milliseconds ms = std::chrono::duration_cast<milliseconds>(ee - ss);
        return ms.count();
    }


    /*-- maps a float range into a timestamp range, preserving fidelity --*/

    uint64_t SVO::mapToTimestamp(float value, float from, float to, uint64_t start, uint64_t end, bool constrain) {

        typedef std::chrono::milliseconds milliseconds;
        typedef std::chrono::system_clock::time_point time_point;
        typedef std::chrono::system_clock::time_point::duration duration;
        typedef std::chrono::nanoseconds nano_seconds;
        typedef std::chrono::system_clock system_clock;

        int dur = getDurationMillis(start,end);
        int milli = ofMap(value, from, to, 0, dur, constrain);

        time_point ss{std::chrono::duration_cast<duration>(nano_seconds(start))};
        ss += milliseconds(milli);
        auto out = std::chrono::duration_cast<nano_seconds>(ss.time_since_epoch()).count();
        return out;
    }

    /*-- maps a timestamp into a float range --*/

    float SVO::mapFromTimestamp(uint64_t timestamp, uint64_t start, uint64_t end, float from, float to, bool constrain) {
        int value = getDurationMillis(start, timestamp);
        int range = getDurationMillis(start, end);
        return ofMap(value, 0, range, from, to, constrain);
    }

    /*-- returns human-readable duration between two timestamps --*/

    string SVO::getHumanDuration(uint64_t start, uint64_t end, string format) {
       std::time_t t = getDurationMillis(start, end);
       char buff[255];
       strftime(buff, 255, format.c_str(),  localtime(&t));
       string time(buff);
       return time;
    }

    /*-- formats timestamp into a human-readable string --*/

    string SVO::getHumanTimestamp(uint64_t timestamp, string format) {

        typedef std::chrono::milliseconds milliseconds;
        typedef std::chrono::system_clock::time_point time_point;
        typedef std::chrono::system_clock::time_point::duration duration;
        typedef std::chrono::nanoseconds nano_seconds;
        typedef std::chrono::system_clock system_clock;

        time_point tt{std::chrono::duration_cast<duration>(nano_seconds(timestamp))};
        std::time_t t = system_clock::to_time_t(tt);
        char buff[255];
        strftime(buff, 255, format.c_str(),  localtime(&t));
        string time(buff);

        /*-- NOTE, milliseconds are appended if the last two format chars are "%." --*/

        if ( format.substr(format.size() - 2, format.size()) == "%." ) {
            time.erase(time.size() - 2, time.size());
            auto millis = std::chrono::duration_cast<milliseconds>(nano_seconds(timestamp)).count();
            time += ofToString( (millis%1000)/10 );
        }
        return time;
    }

    float SVO::getAverageFPS() {
        float seconds = getDurationMillis(getStart(), getEnd())*1000;
        return (float)getTotalFrames()/seconds;
    }
    int SVO::getLookupLength() {
        return lookup.size();
    }
    string SVO::printInfo() {

        string info = "\n";
        info += "File: " + filename;
        info += "\n";
        info += "Path: " + path;
        info += "\n";
        info += "FPS: " + ofToString( fps );
        info += "\n";
        info += "Start time: " + getHumanTimestamp(getStart());
        info += "\n";
        info += "End time: " + getHumanTimestamp(getEnd());
        info += "\n";
        info += "Predicted size: " + ofToString( getPredictedFrames() );
        info += "\n";
        info += "Actual size: " + ofToString( getTotalFrames() );
        info += "\n";
        info += "Lookup size: " + ofToString( getLookupLength() );
        info += "\n";
        info += "Duration: " + ofToString(getHumanDuration(getStart(), getEnd()));
        info += "\n";
        info += "Dropped amount: " + getDroppedPercent();
        info += "\n";
        info += "Average FPS: " + ofToString(getAverageFPS());
        info += "\n";

        ofLogNotice("ofxZED::SVO") << info;

        return info;
    }

    string SVO::getCSV() {

        string info = "";
        info += filename;
        info += ",";
        info += path;
        info += ",";
        info += ofToString( fps );
//        info += ',';
//        for (int i = 0; i < frames.size(); i++) info += ofToString(frames[i].timestamp) + " ";
//        info += ',';
//        for (int i = 0; i < lookup.size(); i++) info += ofToString(lookup[i]) + " ";
        info += '\n';

        return info;
    }


    ofJson SVO::getJson(bool withTables) {
        ofJson j;
        j["filename"] = filename;
        j["path"] = path;
        j["fps"] = fps;
        if (withTables) {
            for (int i = 0; i < frames.size(); i++) j["timestamps"][i] = frames[i].timestamp;
            for (int i = 0; i < lookup.size(); i++) j["lookup"][i] = lookup[i];
        } else {
            j["timestamps"][0] = frames[0].timestamp;
            j["timestamps"][1] = frames[frames.size()-1].timestamp;
        }
        return j;
    }
    void SVO::init( ofJson j ) {

        frames.clear();
        lookup.clear();
        filename = j["filename"].get<string>();
        path = j["path"].get<string>();
        fps = j["fps"].get<int>();
        for (int i = 0; i < j["timestamps"].size(); i++) frames.push_back( Frame(i, j["timestamps"][i].get<uint64_t>()));
        for (int i = 0; i < j["lookup"].size(); i++) lookup.push_back( j["lookup"][i].get<int>() );
    }

    bool SVO::sortSVO(ofxZED::SVO & a, ofxZED::SVO & b) {
       return ((a.getStart()) < (b.getStart()));
    }

    bool SVO::sortSVOPtrs(ofxZED::SVO * a, ofxZED::SVO * b)
    {
       return ((a->getStart()) < (b->getStart()) );
    }



}
