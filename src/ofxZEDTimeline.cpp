#include "ofxZEDTimeline.h"

ofRectangle ofTextBounds(string text) {
    vector<string> lines = ofSplitString(text, "\n");
    int maxLineLength = 0;
    for(int i = 0; i < (int)lines.size(); i++) {
        // tabs are not rendered
        const string & line(lines[i]);
        int currentLineLength = 0;
        for(int j = 0; j < (int)line.size(); j++) {
            if (line[j] == '\t') {
                currentLineLength += 8 - (currentLineLength % 8);
            } else {
                currentLineLength++;
            }
        }
        maxLineLength = MAX(maxLineLength, currentLineLength);
    }

    int padding = 4;
    int fontSize = 8;
    float leading = 1.7;
    int height = lines.size() * fontSize * leading - 1;
    int width = maxLineLength * fontSize;
    return ofRectangle(0,0,width, height);
}
namespace ofxZED {

    Timeline::Timeline() {
        isStereoscopic = false;
        isPlaying = false;
        isPsychedelic = false;
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
        for (auto & s : svos) mapped[s->getSVOPath()] = s;
        ofSort(svos, ofxZED::SVO::sortSVOPtrs);
    }
    void Timeline::draw() {



    }

    void Timeline::update() {

        for (auto & p : players) {

            if (isPlaying || grabOnce) {
                p.second->grab(true, isStereoscopic, isPsychedelic);
            }
        }
        if (grabOnce) grabOnce = false;
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
        ofDrawBitmapString( startStr, 10, 20 );
        ofDrawBitmapString( endStr, w - ofTextBounds(endStr).width - 10, 20 );


        ofPopMatrix();
    }
    uint64_t Timeline::getStart() {
        return svos.front()->getStart();
    }
    uint64_t Timeline::getEnd() {
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
        int xx = ofxZED::SVO::mapFromTimestamp(currentTime, getStart(), getEnd(), 0, w, true);
        ofSetLineWidth(2);
        ofNoFill();
        ofSetColor(255,0,255);
        ofRectangle r( xx - 2, 0, 4, h );
        ofFill();
        ofDrawRectangle(r);

        if (players.size() > 0) {

            int yy = 0;
            int hh = h / players.size();
            for (auto & p : players) {
                ofxZED::SVO * svo = mapped[p.first];
                uint64_t t = svo->frames[p.second->getSVOPosition()].timestamp;
                ofSetColor(255);
                ofDrawBitmapString( ofxZED::SVO::getHumanTimestamp( t, "%H:%M:%S:%." ), xx + 10, yy + 20 );
                int xx = ofxZED::SVO::mapFromTimestamp(t, getStart(), getEnd(), 0, w, true );
                ofDrawRectangle( xx -1, yy + 2, 2, hh - 4 );
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

    void Timeline::drawPlayers(ofRectangle bounds) {

        int total = players.size();
        if (svos.size() <= 0 || total <= 0) return;

        int x = bounds.x;
        int y = bounds.y;
        int w = bounds.width;
        int h = bounds.height;

        ofPushMatrix();
        ofTranslate(x,y);
        float xx = 0;
        float ww = w / total;
        float hh = ( ww / 1280.0 ) * 720.0;
        ofSetColor(255);
        for (auto & p : players) {
            ofRectangle r( xx, 0, ww, hh );


            if(p.second->leftTex.isAllocated()) p.second->leftTex.draw(r);
            r.y += 100;
            if(p.second->depthTex.isAllocated()) p.second->depthTex.draw(r);
//            if(p.second->measureTex.isAllocated()) p.second->measureTex.draw(r);


//                glPointSize(3);
//                ofPushMatrix();
                // the projected points are 'upside down' and 'backwards'
//                ofScale(1, -1, -1);
//                ofTranslate(0, 0, -1000); // center the points a bit
//                ofEnableDepthTest();
//                p.second->mesh.drawVertices();
//                ofDisableDepthTest();
//                ofPopMatrix();


            xx += ww;
        }
        ofPopMatrix();
        ofSetColor(255);

    }


    void Timeline::setSVOFromXY(int x, int y) {


    }


    //--------------------------------------------------------------
    void Timeline::mouseMoved( ofMouseEventArgs & e ){

    }

    void Timeline::setTimeFromXY(int x, int y) {

        currentTime = ofxZED::SVO::mapToTimestamp(x, timelineRect.getLeft(), timelineRect.getRight(), getStart(), getEnd(), true);

        for (auto & p : players) {
            ofxZED::Player * player = p.second;
            ofxZED::SVO * svo = mapped[p.first];

            int lookupIdx = ofxZED::SVO::mapFromTimestamp(currentTime, svo->getStart(), svo->getEnd(), 0, svo->lookup.size(), true);
            int frame = svo->lookup[lookupIdx];
            player->setSVOPosition(frame);

            if (!isPlaying) grabOnce = true;
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
        ofLog() << "NUDGE" << frames;
        for (auto & p : players) p.second->nudge(frames);
        if (!isPlaying) grabOnce = true;
    }

}
