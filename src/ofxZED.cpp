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

void ofxZED::Camera::draw(ofRectangle r) {

    int w = getWidth();
    int h = getHeight();


    stereoAlternate = !stereoAlternate;
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    if (leftTex.isAllocated() && rightTex.isAllocated()) {
        int offset = (float)(stereoOffset * (float)(w*0.01));
        ofRectangle lr = r;
        ofRectangle rr = r;
        lr.x += offset;
        rr.x -= offset;
        (stereoAlternate) ? leftTex.draw(lr) : rightTex.draw(rr);
    }
    if (leftTex.isAllocated() && !rightTex.isAllocated()) leftTex.draw(r);
    if (!leftTex.isAllocated() && rightTex.isAllocated()) rightTex.draw(r);
    ofEnableBlendMode(OF_BLENDMODE_SCREEN);
    if (depthTex.isAllocated()) depthTex.draw(r);
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}





namespace ofxZED {


    static string humanTimestamp(uint64_t & timestamp, string format) {

        std::time_t t = timestamp / 1000000000;
        char buff[255];
        strftime(buff, 255, format.c_str(),  localtime(&t));
        string time(buff);
        return time;
    }
    static std::time_t TimestampToTimeT(sl::timeStamp timestamp) {
        return timestamp / 1000000000;
    }

    static int TimestampToInt(uint64_t timestamp) {
        return timestamp / 1000000000;
    }

    static int TimestampToDurationSeconds(sl::timeStamp end, sl::timeStamp start) {
        std::time_t e = TimestampToTimeT(end);
        std::time_t s = TimestampToTimeT(start);
        double diff =  difftime(e, s);
        return diff;
    }
    static int getDurationInMillis(float fps_, float totalFrames_) {

        int totalMillis = (1000.0/(float)fps_)*(float)totalFrames_;
        return totalMillis;
    }


    bool sortSVO(SVO & a, SVO & b) {
        return (TimestampToTimeT(a.startTime) < TimestampToTimeT(b.startTime));
    }

    bool sortSVOPtrs(SVO * a, SVO * b)
    {
        return (TimestampToTimeT(a->startTime) < TimestampToTimeT(b->startTime) );
    }

    static float getDurationInSeconds(float fps_, float totalFrames_) {
        return (float)getDurationInMillis(fps_, totalFrames_)/1000.0;
    }
    static float getDurationInMinutes(float fps_, float totalFrames_) {
        return getDurationInSeconds(fps_, totalFrames_)/60.0;
    }
    static float getDurationInHours(float fps_, float totalFrames_) {
        return getDurationInMinutes(fps_, totalFrames_)/60.0;
    }
    static int getHumanSeconds(float fps_, float totalFrames_) {
        float d = getDurationInSeconds(fps_, totalFrames_);
        return fmod(d,60);
    }
    static int getHumanMinutes(float fps_, float totalFrames_) {
        float d = getDurationInMinutes(fps_, totalFrames_);
        return fmod(d,60);
    }
    static int getHumanHours(float fps_, float totalFrames_) {
        float d = getDurationInHours(fps_, totalFrames_);
        return fmod(d,60);
    }

    static string getDurationInHuman(float fps_, float totalFrames_) {

        int s_ = getHumanSeconds(fps_, totalFrames_);
        int m_ = getHumanMinutes(fps_, totalFrames_);
        int h_ = getHumanHours(fps_, totalFrames_);
        string human_ =  ( ofToString( h_ ) + " hours " + ofToString( m_ ) + " min " + ofToString( s_ ) + " secs " );
        return human_;
    }

    void SVO::init( string filename_, int fps_, ofVec2f resolution_, int totalFrames_, sl::timeStamp startTime_, sl::timeStamp endTime_) {
        filename = filename_;
        fps = fps_;
        resolution = resolution_;
        totalFrames = totalFrames_;
        startTime = startTime_;
        endTime = endTime_;

        startHuman = humanTimestamp(startTime);
        endHuman = humanTimestamp(endTime);

        durationSeconds =  TimestampToDurationSeconds(endTime, startTime);
        durationMinutes =  TimestampToDurationSeconds(endTime, startTime)/60;
        predictedFrames = ((float)durationSeconds * (float)fps);
        averageFPS = (float)totalFrames/(float)durationSeconds;
        droppedFrames = (predictedFrames - totalFrames);
        droppedAmount =  ofToString((int)(100-((100.0/predictedFrames) * totalFrames)))+ "%";
    }


    string SVO::getDescriptionStr() {

        string info = "\n";
        info += "Filename: " + filename;
        info += "\n";
        info += "FPS: " + ofToString( fps );
        info += "\n";
        info += "Resolution: " + ofToString( resolution[0] ) +" x "+ ofToString( resolution[1] );
        info += "\n";
        info += "Start time: " + startHuman;
        info += "\n";
        info += "End time: " + endHuman;
        info += "\n";
        info += "Predicted frames: " + ofToString( predictedFrames );
        info += "\n";
        info += "Total frames: " + ofToString( totalFrames );
        info += "\n";
        info += "Duration seconds: " + ofToString(durationSeconds);
        info += "\n";
        info += "Duration minutes: " + ofToString(durationMinutes);
        info += "\n";
        info += "Dropped frames: " + ofToString(droppedFrames);
        info += "\n";
        info += "Dropped amount: " + ofToString(droppedAmount);
        info += "\n";
        info += "Average FPS: " + ofToString(averageFPS);
        info += "\n";

        return info;
    }

    string SVO::getCSV() {

        string info = "";
        info += filename;
        info += ",";
        info += ofToString( fps );
        info += ",";
        info += ofToString( resolution[0] ) +" x "+ ofToString( resolution[1] );
        info += ",";
        info += ofToString( startTime );
        info += ",";
        info += ofToString( endTime );
        info += ",";
        info += startHuman;
        info += ",";
        info += endHuman;
        info += ",";
        info += ofToString(durationMinutes);
        info += ",";
        info += ofToString(averageFPS);
        info += ',';
        info += ofToString(totalFrames);
        info += ',';
        info += ofToString(predictedFrames);
        info += '\n';

        return info;
    }

    ofJson SVO::getJson() {
        ofJson j;
        j["filename"] = filename;
        j["fps"] = fps;
        j["resolution"]["width"] = (int)resolution[0];
        j["resolution"]["height"] = (int)resolution[1];
        j["startTime"] = startTime;
        j["endTime"] = endTime;
        j["startHuman"] = startHuman;
        j["endHuman"] = endHuman;
        j["predictedFrames"] = predictedFrames;
        j["totalFrames"] = totalFrames;
        j["durationSeconds"] = durationSeconds;
        j["durationMinutes"] = durationMinutes;
        j["droppedFrames"] = droppedFrames;
        j["droppedAmount"] = droppedAmount;
        j["averageFPS"] = averageFPS;
        return j;
    }
    void SVO::init( ofJson j ) {

        filename = j["filename"].get<string>();
        fps = j["fps"].get<int>();
        resolution[0] = j["resolution"]["width"].get<int>();
        resolution[1] = j["resolution"]["height"].get<int>();
        startHuman = j["startHuman"].get<string>();
        endHuman = j["endHuman"].get<string>();
        startTime = j["startTime"].get<uint64_t>();
        endTime = j["endTime"].get<uint64_t>();
        predictedFrames = j["predictedFrames"].get<int>();
        totalFrames = j["totalFrames"].get<int>();
        durationSeconds = j["durationSeconds"].get<int>();
        durationMinutes = j["durationMinutes"].get<int>();
        droppedFrames = j["droppedFrames"].get<int>();
        droppedAmount = j["droppedAmount"].get<string>();
        averageFPS = j["averageFPS"].get<float>();
    }


    bool Player::openSVO(string root, SVO * svo_) {
        svo = svo_;
        string p = ofFilePath::join(root, svo->filename);
        ofLog() << "path" << p;
        return ofxZED::Camera::openSVO(p);
    }
    void Player::setPositionFromTimestamp(int time) {

    }


    void Player::grab(bool left, bool right, bool depth) {



        frameNew = false;
        sl::RuntimeParameters runtime_parameters;
        runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL; // Use STANDARD sensing mode
        runtime_parameters.enable_depth = depth;
        sl::Camera::setSVOPosition(0);

        if (sl::Camera::grab(runtime_parameters) == sl::SUCCESS) {

            frameNew = true;

            int w = getWidth();
            int h = getHeight();
            uint64_t timestamp =  sl::Camera::getCameraTimestamp() ;
            ofLog() << "grabbing..." << sl::Camera::getSVOPosition() << humanTimestamp(timestamp);

            if (sl::Camera::getSVOPosition() == lastPosition) return;

            ofLog() << "retrieving..." << w << h;

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
    }
    void Database::init(string location, string fileName) {

        dir.allowExt("svo");
        dir.open(location);
        dir.listDir();

        totalFiles = dir.getFiles().size();
        currIndex = 0;
        totalFrames = 0;

        saveLocation = dir.getAbsolutePath() + "/" + fileName;
        json = ofLoadJson(saveLocation + ".json");


        for (auto f : dir.getFiles()) process(f);

        ofSort(data, sortSVO);
        write();
        ofLogNotice("ofxZED::Database") << "finished! total frames" << totalFrames;

        zed.close();
    }


    void Database::process(ofFile & f) {


        if (json["files"].find(f.getFileName()) != json["files"].end()) {

            ofLogNotice("ofxZED::Database") << "moving to next item, already in JSON" << currIndex << "of" << dir.getFiles().size();


            SVO svo;

            svo.init(json["files"][f.getFileName()]);
            data.push_back(svo);

        } else {


           ofLogNotice("ofxZED::Database") << "opening svo";


            if (zed.openSVO(f.getAbsolutePath())) {

                /* get start time */

                ofLogNotice("ofxZED::Database") << "waiting for start time";
                zed.setSVOPosition(0);
                while ( zed.grab() != sl::SUCCESS )  sl::sleep_ms(1);

                sl::timeStamp startTime = zed.getFrameTimestamp();

                /* get end time */

                ofLogNotice("ofxZED::Database") << "waiting for end time";
                zed.setSVOPosition( zed.getSVONumberOfFrames() - 2 );
                while ( zed.grab() != sl::SUCCESS )  sl::sleep_ms(1);

                sl::timeStamp endTime = zed.getFrameTimestamp();



                SVO svo;
                svo.init(f.getFileName(), zed.getCameraFPS(), ofVec2f( zed.getWidth(), zed.getHeight() ), zed.getSVONumberOfFrames(), startTime, endTime );
                data.push_back(svo);


            } else {
                ofLogError("ofxZED::Database") << "could not open" << f.getAbsolutePath();
                OF_EXIT_APP(0);
            }
        }
        currIndex += 1;


        SVO & svo = data.back();



        int backwards = 2;

        if (svo.endTime <= svo.startTime) {

            ofLogNotice("ofxZED::Database") << "attempting to fix broken end time" << svo.filename << svo.startTime << svo.endTime;

            if (zed.openSVO(f.getAbsolutePath())) {

                while (svo.endTime <= svo.startTime) {

                    ofLogNotice("zedinfo") << "fixing broken end time" << svo.startTime << svo.endTime << backwards;
                    zed.setSVOPosition( zed.getSVONumberOfFrames() - backwards );
                    while ( zed.grab() != sl::SUCCESS )  sl::sleep_ms(1);
                    svo.endTime = zed.getFrameTimestamp();

                }

                ofLogNotice("ofxZED::Database") << "fixed end and start to" << svo.startTime << svo.endTime;
            }
        }

        ofLogNotice("ofxZED::Database") << "\n" << svo.getDescriptionStr();

        totalFrames += svo.totalFrames;
        write();

    }
    void Database::write() {


        string csv = "Filename,FPS,Resolution,Start,Start Date,End,End Date,Duration (mins),Average FPS,Total Frames, Predicted Frames\n";

        json["info"]["totalFrames"] = totalFrames;
        json["info"]["progress"] = ofToString(currIndex) + " out of " + ofToString(totalFiles);
        json["info"]["totalDuration"] = getDurationInHuman(60, totalFrames);

        ofSort(data, sortSVO);
        for ( auto & d : data) {
            json["files"][d.filename] = d.getJson();
            csv += d.getCSV();
        }

        ofLogNotice("ofxZED::Database") << "moving to" << json["info"]["progress"];

        ofBuffer buff;
        buff.set(csv.c_str(), csv.size());
        ofSavePrettyJson(saveLocation + ".json" , json);
        ofBufferToFile(saveLocation + ".csv", buff);
    }

    std::map<string, vector<SVO *>> Database::getSortedByDay() {
        std::map<string, vector<SVO *>> db;
        for (auto & d : data) {
            string date = humanTimestamp(d.startTime, "%A %d %b");
            if (db.find(date) == db.end()) db[date] = {};
            db[date].push_back(&d);
        }
        for (auto & d : db) ofSort(d.second, sortSVOPtrs);
        return db;
    }
    vector<SVO *> Database::getPtrs() {
        vector<SVO *> db;
        for (auto & d : data) db.push_back(&d);
        return db;
    }

    vector<SVO *> Database::getPtrsInsideTimestamp(int time) {
        vector<SVO *> db;
        int DIV = 1000000000;
        for (auto & d : data) if (time >= d.startTime/DIV && time < d.endTime/DIV ) db.push_back(&d);
        return db;
    }

}
