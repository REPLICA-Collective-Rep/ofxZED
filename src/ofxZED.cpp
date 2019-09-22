#include "ofxZED.h"

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


ofxZED::Camera::Camera() {
    stereoOffset = 0;
}

int ofxZED::Camera::getSerialNumber() {
    if (!sl::Camera::isOpened()) return -1;
    return sl::Camera::getCameraInformation().serial_number;
}

void ofxZED::Camera::logSerial() {
    ofLogNotice( "ofxZED") << sl::Camera::getCameraInformation().serial_number << ((sl::Camera::isOpened()) ? "is Open" : "is Closed");
}

void ofxZED::Camera::processMatToPix(ofPixels & pix, sl::Mat & mat, bool psychedelic) {
    int w = mat.getWidth();
    int h = mat.getHeight();
    if (pix.getWidth() != w || pix.getHeight() != h || !pix.isAllocated()) {
        pix.allocate(w, h, 4);
    }
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            sl::uchar4  p;
            mat.getValue<sl::uchar4 >(x, y, &p);
            int index = 3 * (x + y * w);

            if (!psychedelic) {

                pix[index + 0] = p.b;
                pix[index + 1] = p.g;
                pix[index + 2] = p.r;
                pix[index + 3] = p.a;

            } else {

                pix[index + 0] = p.r;
                pix[index + 1] = (p.r*2 < 255) ? 255 - (p.r*2) : 0;
                pix[index + 2] = (p.r > 255/2) ? (255) - (p.r - (255/2)) : 255;
                pix[index + 3] = 255;
            }

        }
    }
}


string ofxZED::Camera::getTimestamp(string format) {
    string date =  ofToString(ofGetYear()) + "-" + ofToString(ofGetMonth()) + "-" + ofToString(ofGetDay()) ;
    date += "-";
    std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(p);
    char buff[255];
    strftime(buff, 255, format.c_str(), localtime(&t));
    string time(buff);
    return (time);
}

void ofxZED::Camera::record(string path, bool b, bool autoPath) {

    if (b) {
        frameCount = 0;
        if (autoPath) {
            path += "";
            path +=  ofToString( sl::Camera::getCameraInformation().serial_number );
            path += "_";
            path += getTimestamp();
            path += ".svo";
        }
        ofLogNotice("ofxZED") << "recording to" << path;
        sl::String p(path.c_str());
        auto error = sl::Camera::enableRecording(p, sl::SVO_COMPRESSION_MODE_HEVC);
        if (error != sl::SUCCESS) {
            ofLogError("ofxZED") << "recording initialisation error";
            if (error == sl::ERROR_CODE_SVO_RECORDING_ERROR)
               ofLogError("ofxZED") << "recording error - check path and write permissions";
            if (error == sl::ERROR_CODE_SVO_UNSUPPORTED_COMPRESSION)
                ofLogError("ofxZED") << "unsupported compression method - check your hardware";

            sl::Camera::close();

        } else {
            isRecording = b;
        }
    } else {
        isRecording = b;
        frameCount = 0;
        sl::Camera::disableRecording();
        ofLogNotice("ofxZED") << "stopped recording";
    }
}

void ofxZED::Camera::toggleRecording(string path, bool autoPath) {
    if (!isRecording) {
        record(path, true, autoPath);
    } else {
        record(path, false, autoPath);
    }
}
void ofxZED::Camera::updateRecording() {

    frameNew = false;
    sl::RuntimeParameters runtime_parameters;
    runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL; // Use STANDARD sensing mode
    runtime_parameters.enable_depth = true;


    if (sl::Camera::grab(runtime_parameters) == sl::SUCCESS) {

        frameNew = true;

        if (isRecording) {
            sl::RecordingState state = sl::Camera::record();
            if (state.status) frameCount++;
//                ofLogNotice("ofxZED") << "Frame count: " << frameCount;
        }
    }
}

bool ofxZED::Camera::isFrameNew() {
    return frameNew;
}
ofQuaternion ofxZED::Camera::eulerToQuat(const ofVec3f & rotationEuler) {
    ofQuaternion rotation;
    float c1 = cos(rotationEuler[2] * 0.5);
    float c2 = cos(rotationEuler[1] * 0.5);
    float c3 = cos(rotationEuler[0] * 0.5);
    float s1 = sin(rotationEuler[2] * 0.5);
    float s2 = sin(rotationEuler[1] * 0.5);
    float s3 = sin(rotationEuler[0] * 0.5);

    rotation[0] = c1 * c2*s3 - s1 * s2*c3;
    rotation[1] = c1 * s2*c3 + s1 * c2*s3;
    rotation[2] = s1 * c2*c3 - c1 * s2*s3;
    rotation[3] = c1 * c2*c3 + s1 * s2*s3;

    return rotation;
}

int ofxZED::Camera::getWidth() {
    return  sl::Camera::getResolution().width;
}

int ofxZED::Camera::getHeight() {
    return sl::Camera::getResolution().height;
}


bool ofxZED::Camera::openSVO(string svoPath) {

    lastPosition = -1;
    ofLogNotice("ofxZED") << "loading svo" << svoPath;
    init.svo_input_filename.set(svoPath.c_str());
//    init.svo_real_time_mode = true;

    return openWithParams();
}

bool ofxZED::Camera::openCamera(int i) {



    init.camera_resolution = sl::RESOLUTION_HD1080;
    init.depth_mode = sl::DEPTH_MODE_QUALITY; // Use PERFORMANCE depth mode
    init.sdk_verbose = true;
    init.coordinate_units = sl::UNIT_METER;
    init.coordinate_system = sl::COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP;

    if (i != -1) init.input.setFromCameraID(i);


    return openWithParams();
}

bool ofxZED::Camera::openWithParams() {



    if (sl::Camera::isOpened()) {
        sl::Camera::close();
    } else {
        ofAddListener(ofGetWindowPtr()->events().exit, this, &ofxZED::Camera::close);
    }

    bool success = false;


    try {
        auto resp = sl::Camera::open(init);
        if (resp != sl::SUCCESS) {
            ofLogNotice("ofxZED") << "couldn't open ZED: ";
            std::cout << sl::toString(resp) << std::endl; // Display the error;
            sl::Camera::close();

        } else {

            ofLogNotice("ofxZED") << "successfully loaded";// << sl::Camera::getCameraInformation().serial_number;
            sl::Camera::enableTracking();

            int w = getWidth();
            int h = getHeight();


//            colorTexture.allocate(w, h, GL_RGBA, false);
//            depthTexture.allocate(w, h, GL_RGBA, false);
            success = true;
        }

    } catch (const std::exception& e) {

        ofLogNotice("ofxZED") << "couldn't open ZED from unknown error ";
    }

    return success;
}

uint64_t ofxZED::Camera::getFrameTimestamp() {
    return  sl::Camera::getTimestamp(sl::TIME_REFERENCE_IMAGE);
}
uint64_t ofxZED::Camera::getLastTimestamp() {
    return  sl::Camera::getTimestamp(sl::TIME_REFERENCE_LAST);
}

void ofxZED::Camera::close(ofEventArgs &args) {
    sl::Camera::close();
}

void ofxZED::Camera::close() {
    sl::Camera::close();
}

void ofxZED::Camera::draw(ofRectangle r, bool left, bool right, bool depth) {

    int w = getWidth();
    int h = getHeight();


    stereoAlternate = !stereoAlternate;
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    if (leftTex.isAllocated() && rightTex.isAllocated() && left && right) {
        int offset = (float)(stereoOffset * (float)(w*0.01));
        ofRectangle lr = r;
        ofRectangle rr = r;
        lr.x += offset;
        rr.x -= offset;
        (stereoAlternate) ? leftTex.draw(lr) : rightTex.draw(rr);
    }
    if (leftTex.isAllocated() && !rightTex.isAllocated() && left && !right) leftTex.draw(r);
    if (!leftTex.isAllocated() && rightTex.isAllocated() && !left && right) rightTex.draw(r);
    ofEnableBlendMode(OF_BLENDMODE_SCREEN);
    if (depthTex.isAllocated() && depth) depthTex.draw(r);
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}





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
        return frames.back().timestamp;
    }
    int SVO::getPredictedFrames() {
        return  ((float)(getDurationMillis(getStart(), getEnd())/1000.0) * (float)fps);
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
        return ofMap(value, 0, range, from, to, constrain);)
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


    bool Player::openSVO(string root, SVO * svo_) {
        svo = svo_;
        string p = ofFilePath::join(root, svo->filename);
        ofLog() << "path" << p;
        return ofxZED::Camera::openSVO(p);
    }

    int Player::grab(bool left, bool right, bool depth) {


        if (!sl::Camera::isOpened()) return  sl::Camera::getSVOPosition();

        frameNew = false;
        sl::RuntimeParameters runtime_parameters;
        runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL; // Use STANDARD sensing mode
        runtime_parameters.enable_depth = depth;

        if (sl::Camera::grab(runtime_parameters) == sl::SUCCESS) {

            frameNew = true;

            int w = getWidth();
            int h = getHeight();

            if (sl::Camera::getSVOPosition() == lastPosition)  sl::Camera::getSVOPosition();

//            ofLog() << "retrieving..." << w << h;

            if (left) sl::Camera::retrieveImage(leftMat, sl::VIEW_LEFT, sl::MEM_CPU, w,h);
            if (right) sl::Camera::retrieveImage(rightMat, sl::VIEW_RIGHT, sl::MEM_CPU, w,h);
            if (depth) sl::Camera::retrieveImage(depthMat, sl::VIEW_DEPTH, sl::MEM_CPU, w, h);


            if (left) processMatToPix(leftPix, leftMat);
            if (right) processMatToPix(rightPix, rightMat);
            if (depth) processMatToPix(depthPix, depthMat, true);


            if (left) leftTex.loadData(leftPix.getData(), w, h, GL_RGB);
            if (right) rightTex.loadData(rightPix.getData(), w, h, GL_RGB);
            if (depth) depthTex.loadData(depthPix.getData(), w, h, GL_RGB);

            lastPosition = sl::Camera::getSVOPosition();

        } else {

            ofLog() << "Did not grab";
        }

        return sl::Camera::getSVOPosition();
    }









    /*-- Databse --*/








    void Database::init(string location, bool recreate, string fileName) {

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

        ofSort(data, SVO::sortSVO);
        write();
        ofLogNotice("ofxZED::Database") << "finished initing database" << data.size() << "/" << totalFiles;

        zed.close();
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

        ofLogNotice("ofxZED::Database") << "loading" << currIndex << "/" << dir.getFiles().size();

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

    bool SVO::sortSVO(ofxZED::SVO & a, ofxZED::SVO & b) {
       return ((a.getStart()) < (b.getStart()));
    }

    bool SVO::sortSVOPtrs(ofxZED::SVO * a, ofxZED::SVO * b)
    {
       return ((a->getStart()) < (b->getStart()) );
    }



}
