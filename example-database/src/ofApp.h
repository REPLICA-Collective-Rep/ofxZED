#pragma once

#include "ofMain.h"
#include "ofxZEDSVO.h"
#include "ofxZEDDatabase.h"

struct Range {
public:
    string name;
    string start;
    string end;
    Range(string name_, string start_, string end_) {
        name = name_;
        start = start_;
        end = end_;
    }
};

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void exit();

        vector<string> arguments;
        ofxZED::Database db;
        vector<Range> ranges;
        string print;
};
