#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){

    ofLog::setAutoSpace(true);

	/*-- parse commandline arguments, ie. a folder directory --*/

    string currentRoot = "/home/autr/Desktop/bin/03_ZED";
    if (arguments.size() <= 1) {
        ofLogError() << "no arguments are set, set arguments with --args /path/to/directory";
        ofLogNotice() << "defaulting to" << currentRoot;
    } else {
        currentRoot = arguments[1];
    }

    /*-- Database Initialisation
     * scan the folder for SVOs then;
        * a) if an entry isn't present in database.json, create one
        * b) if an entry is available in database.json, load it
     * an unpopulated database might take some time as it scans all frames for their timestamps
     * the database.json is loaded as vector<ofxZED::SVO> data  --*/

    db.build(currentRoot);

    /*-- search for these time ranges --*/

    ranges.push_back( Range("Chairs w. Ditte", "14/08/2019 11:24:00", "14/08/2019 11:28:10") );
    ranges.push_back( Range("Shadowing w. Matej", "12/08/2019 16:47:00", "12/08/2019 16:50:00") );
    ranges.push_back( Range("Jungle Run w. Ditte", "14/08/2019 10:59:00", "14/08/2019 11:04:00") );
    ranges.push_back( Range("Stretching in Line w. Matej", "14/08/2019 15:33:52", "14/08/2019 15:52:37") );

    print = "";

    ofxZED::Database newDb;
    string newDbPath = ofFilePath::join(currentRoot, "/_presentation/");

    for (auto & range : ranges) {
        print += range.name + "\n";
        print += range.start + "\n";
        print += range.end + "\n";

        /*-- convert time string into timestamps --*/

        uint64_t start = ofxZED::SVO::getTimestampFromStr( range.start );
        uint64_t end = ofxZED::SVO::getTimestampFromStr( range.end );

        /*-- find SVOs filtered by start and end time --*/

        vector<ofxZED::SVO *> svos = db.getFilteredByRange( start, end );
        for (auto & svo : svos) {
            print += svo->filename + "\n";
            ofFile file( ofFilePath::join(currentRoot, svo->filename) );

            /*-- copy SVOs to new folder and new database --*/

            file.copyTo( ofFilePath::join(newDbPath, svo->filename));
            newDb.data.push_back( *svo );
        }
        print += "\n";

    }

    /*-- save new Database for only SVOs in ranges --*/

    newDb.write( newDbPath, "database" );

    ofLog() << print;
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(20);
    ofSetColor(255);
    ofDrawBitmapString( print , 20, 20 );
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::exit() {

}
