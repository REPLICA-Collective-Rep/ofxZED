#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofLog::setAutoSpace(true);

	/*-- parse commandline arguments, ie. a folder directory --*/

    string currentRoot = "/home/autr/Desktop/bin/03_ZED";
    if (arguments.size() <= 1) {
        ofLogError() << "No arguments are set, set arguments with --args /path/to/directory";
        ofLogNotice() << "Defaulting to" << currentRoot;
    } else {
        currentRoot = arguments[1];
    }

    /*-- scan the folder for SVOs and create database entries into database.json --*/

    db.init(currentRoot);

    /*-- database.json is loaded into ofxZED::Database::data, as vector<ofxZED::SVO>  --*/


}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

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
