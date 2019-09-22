#include "ofxZEDSVO.h"

namespace ofxZED {


    void SVO::init( ofFile &f, int fps_) {
        filename = f.getFileName();
        path = f.getAbsolutePath();
        fps = fps_;
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
        info += ofToString( fps );
        info += ',';
        for (int i = 0; i < frames.size(); i++) info += ofToString(frames[i].timestamp) + " ";
        info += ',';
        for (int i = 0; i < lookup.size(); i++) info += ofToString(lookup[i]) + " ";
        info += '\n';

        return info;
    }

    ofJson SVO::getJson() {
        ofJson j;
        j["filename"] = filename;
        j["fps"] = fps;
        for (int i = 0; i < frames.size(); i++) j["timestamps"][i] = frames[i].timestamp;
        for (int i = 0; i < lookup.size(); i++) j["lookup"][i] = lookup[i];
        return j;
    }
    void SVO::init( ofJson j ) {

        frames.clear();
        lookup.clear();
        filename = j["filename"].get<string>();
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
