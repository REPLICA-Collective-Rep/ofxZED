#include "ofxZEDTimeline.h"


namespace ofxZED {

    Timeline::Timeline() {
        isPlaying = false;
        grabOnce = false;
    }

    void Timeline::init() {

        ofRegisterMouseEvents(this);
    }
    void Timeline::load(vector<SVO *> svos_) {
        vector<string> names;
        for (auto & s : svos_) names.push_back(s->getName());
        for (auto & p : players) {
            if ( std::find(names.begin(), names.end(), p.first) == names.end()) {
                ofLog() << "deleting" << p.first;
                p.second->close();
                players.erase( p.first );
            }
        }

        for (auto & s : svos_) {
            string name = s->getName();
            if ( players.find(name) == players.end()) {
                string path = s->getSVOPath();
                ofLog() << "loading" << path;
//                ofxZED::Player player;
                players[path] = new ofxZED::Player();
                players[path]->openSVO( s );
            }
        }

    }

    void Timeline::set(vector<SVO *> svos_, bool load) {
        svos = svos_;
        mapped.clear();
        for (auto & s : svos) {
            mapped[s->getSVOPath()] = s;
        }
        ofSort(svos, ofxZED::SVO::sortSVOPtrs);
    }


    bool Timeline::doesIntersect(ofRectangle & rect, vector<ofRectangle> & rects) {
        bool b = false;
        for (auto & r : rects) if (rect.intersects(r)) return true;
        return false;
    }
    void Timeline::drawBlocks(ofRectangle bounds) {



        typedef ofxZED::SVO SVO;
        string format = "%D %H:%M:%S";
        string startStr = SVO::getHumanTimestamp(getStart(), format);
        string endStr = SVO::getHumanTimestamp(getEnd(), format);
        blocksRect = bounds;
        int x = blocksRect.x;
        int y = blocksRect.y;
        int w = blocksRect.width;
        int h = blocksRect.height;
        ofPushMatrix();
        ofTranslate(x,y);
        vector<ofRectangle> rects;
        float multi = 1;
        for (auto & s : svos) {
            int yy = 0;
            int xx = SVO::mapFromTimestamp(s->getStart(), getStart(), getEnd(), 0, w, false);
            int ww = ofMap( SVO::getDurationMillis(s->getStart(), s->getEnd()), 0.0, SVO::getDurationMillis(getStart(), getEnd()), 0, w, false);
            int hh = h - 10;
            ofRectangle rect(xx,yy,ww,hh);
            int count = 1;
            while ( doesIntersect(rect, rects) ) {
                rect.y += h;
                count += 1;
            }
            if (count > multi) multi = count;
            rects.push_back(rect);
        }

//        ofLog() << multi;

        ofPushMatrix();
        ofScale(1, 1.0/multi);

        for (auto & r : rects) {
            ofFill();
            ofSetColor(ofMap(r.x, 0, w, 0, 255), ofMap(r.x, 0, w, 255, 0), 255);
            ofDrawRectangle( r );
            ofNoFill();
        }
        ofPopMatrix();

        ofSetColor(255);


        drawString( startStr, 10, 20 );
        drawString( endStr, w - getStringWidth(endStr,0,0) - 10, 20 );


        ofPopMatrix();
    }

    int Timeline::getStringWidth(string txt, int x, int y) {

        return theme->font.ptr.get()->width(txt, x, y);
    }
    void Timeline::drawString(string txt, int x, int y) {
        theme->font.ptr.get()->draw(txt, x, y);
    }
    uint64_t Timeline::getStart() {
        if (svos.size() <=0) return 0;
        return svos.front()->getStart();
    }
    uint64_t Timeline::getEnd() {
        if (svos.size() <=0) return 0;
        return svos.back()->getEnd();
    }

    void Timeline::drawTimeline(ofRectangle bounds) {


        timelineRect = bounds;


        int x = bounds.x;
        int y = bounds.y;
        int w = bounds.width;
        int h = bounds.height;

        ofPushMatrix();
        ofTranslate(x,y);


        ofFill();

        for (int i = 0; i < 100; i++) {
            float xx = ((float)(w-10)/99.0)*(float)i;
            ofSetColor(80);
            ofDrawRectangle( xx + 5, 5, 0.5, h-10 );
        }

        vector<int> playheads = {};
        if (players.size() > 0) {
            for (auto & player : players) {
                ofxZED::SVO * svo = mapped[player.first];
                ofxZED::Player * p = player.second;
                if (p->left || p->right || p->depth || p->cloud) {
                    uint64_t t = svo->frames[player.second->getSVOPosition()].timestamp;
                    float xx = ofxZED::SVO::mapFromTimestamp(t, getStart(), getEnd(), 0, w, true );
                    playheads.push_back((int)xx);
                }
            }

            ofSetColor(120);
            int xx =  ofxZED::SVO::mapFromTimestamp(currentTime, getStart(), getEnd(), 0, w, true);
            ofRectangle r( xx - 3, 5, 6, h - 10 );
            ofDrawRectangle(r);
        }


        if (playheads.size() > 0) {

            int yy = 0;
            int hh = h / playheads.size();
            for (auto & xx : playheads) {
                ofSetColor(255);
                ofDrawRectangle( xx -1, yy + 10, 2, hh - 20 );
                yy += hh;
            }
        }


        ofPopMatrix();
        ofSetColor(255);

    }

    void Timeline::drawButtons(ofRectangle bounds) {

        int x = bounds.x;
        int y = bounds.y;
        int w = bounds.width;
        int h = bounds.height;
        float bit = 0.4;
        ofPushMatrix();
        ofTranslate(x,y);
        if (!isPlaying) {
            ofDrawTriangle( ofVec2f(w * bit, h * bit), ofVec2f(w * (1-bit), h/2), ofVec2f( w * bit, h * (1-bit) ));
        } else {
            int ww = (w * bit) * 0.2;
            ofDrawRectangle( w * bit, h * bit, ww, h - (h * bit * 2) );
            ofDrawRectangle( ( w * (1-bit) ) - ww, h * bit,ww, h - (h * bit * 2) );
        }
        ofPopMatrix();
    }

    bool Timeline::update() {

        bool setViaPlayer = false;


//        ofLog() << "grab video" << p->left << left;
        if (isPlaying || grabOnce) {
            uint64_t t = 0;
            uint64_t total = 0;
            if (players.size() > 0) {

                for (auto & player : players) {
                    ofxZED::Player * p = player.second;
                    if (p->left || p->right || p->depth || p->cloud) {
//                        ofLog() << "grabbing video";
                        p->grab();
                        setViaPlayer = true;
                        ofxZED::SVO * svo = mapped[player.first];
                        uint64_t t = svo->frames[p->getSVOPosition()].timestamp;
                        total += t;


                        currentTime = t;
                    }
                }
            }

//            if (setViaPlayer) currentTime = total/players.size();
        }



        if (grabOnce) grabOnce = false;

        if (!setViaPlayer && isPlaying) return true;
        return false;
    }


    void Timeline::setSVOFromXY(int x, int y) {


    }


    //--------------------------------------------------------------
    void Timeline::mouseMoved( ofMouseEventArgs & e ){

    }

    void Timeline::setTimeFromXY(int x, int y) {


        currentTime = ofxZED::SVO::mapToTimestamp(x, timelineRect.getLeft(), timelineRect.getRight(), getStart(), getEnd(), true);

        for (auto & player : players) {

            ofxZED::Player * p = player.second;

            if (p->left || p->right || p->depth || p->cloud) {
                ofxZED::SVO * svo = mapped[player.first];
                svo->checkForLookup();
                int lookupIdx = ofxZED::SVO::mapFromTimestamp(currentTime, svo->getStart(), svo->getEnd(), 0, svo->getTotalLookupFrames(), true);
                int frame = svo->getLookupIndex(lookupIdx);
                p->setSVOPosition(frame);

                if (!isPlaying) grabOnce = true;
            }
        }
    }

    //--------------------------------------------------------------
    void Timeline::mouseDragged(ofMouseEventArgs & e){

        if (isTimeline) setTimeFromXY(e.x, e.y);
    }

    //--------------------------------------------------------------
    void Timeline::mousePressed(ofMouseEventArgs & e){
        if ( timelineRect.inside(e.x,e.y) ) {
            isTimeline = true;
            setTimeFromXY(e.x, e.y);
        }
        if ( blocksRect.inside(e.x,e.y) ) isBlocks = true;
        if ( zoomRect.inside(e.x,e.y) ) isZoom = true;

    }

    //--------------------------------------------------------------
    void Timeline::mouseReleased(ofMouseEventArgs & e){

         isTimeline = false;
         isBlocks = false;
         isZoom = false;
    }

    //--------------------------------------------------------------
    void Timeline::mouseEntered(ofMouseEventArgs & e){

    }

    //--------------------------------------------------------------
    void Timeline::mouseExited(ofMouseEventArgs & e){

    }
    void Timeline::mouseScrolled(ofMouseEventArgs & e) {

    }

    void Timeline::nudge( int frames ) {
        for (auto & p : players) p.second->nudge(frames);
        if (!isPlaying) grabOnce = true;
    }

}
