#pragma once

#include "ofMain.h"
#include "ofxZEDSVO.h"
#include "ofxZEDDatabase.h"
#include "ofxZEDPlayer.h"
#include <sl/Camera.hpp>

static  ofRectangle ofTextBounds(string text);

namespace ofxZED {

    struct Range {
    public:
        string name;
        uint64_t start;
        uint64_t end;
        string startStr;
        string endStr;
        Range(string name_, uint64_t start_, uint64_t end_) {
            name = name_;
            start = start_;
            end = end_;
        }
        Range(string name_, string start_, string end_, string format = "%d/%m/%Y %H:%M:%S") {
            name = name_;
            start = SVO::getTimestampFromStr(start_, format);
            end =  SVO::getTimestampFromStr(end_, format);
        }
    };

    struct Position {
    public:
       uint64_t timestamp;
       int frame;
       Position(uint64_t timestamp_, int frame_) {
           timestamp = timestamp_;
           frame = frame_;
       }
    };


    class Timeline {
    public:


        Range * range;
        Timeline();

        uint64_t currentTime;
        std::map<string, ofxZED::Player *> players;
        std::map<string, ofxZED::SVO *> mapped;

        /*-- ui vars --*/

//        string currentRoot;
        bool isPlaying;
        bool isStereoscopic;
        bool isPsychedelic;

        bool isTimeline;
        bool isBlocks;
        bool isZoom;

        bool grabOnce;

        void mouseMoved(ofMouseEventArgs & e );
        void mouseDragged(ofMouseEventArgs & e);
        void mousePressed(ofMouseEventArgs & e);
        void mouseReleased(ofMouseEventArgs & e);
        void mouseEntered(ofMouseEventArgs & e);
        void mouseExited(ofMouseEventArgs & e);
        void mouseScrolled(ofMouseEventArgs & e);

        ofRectangle blocksRect, timelineRect, zoomRect;

        vector<ofxZED::SVO *> svos;

        /*-- database and players --*/

        ofxZED::Database * db;


        /*-- methods --*/

        void init();
        void load(vector<SVO *> svos_);
        void set(vector<SVO *> svos_, bool load = false);
//        ofRectangle txtBounds(string text);
        void draw();

        void update();

        uint64_t getStart();
        uint64_t getEnd();
        void drawBlocks(ofRectangle bounds);
        void drawTimeline(ofRectangle bounds);
        void drawPlayers(ofRectangle bounds);
        void drawButtons(ofRectangle bounds);

        void setTimeFromXY(int x, int y);
        void setSVOFromXY(int x, int y);
        bool doesIntersect(ofRectangle & rect, vector<ofRectangle> & rects);
        void nudge( int frames );
    };

}
