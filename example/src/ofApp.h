#pragma once
#include "ofMain.h"
#include "ofxZED.h"
#include "ofxAssimpModelLoader.h"

struct Txt {
    string s;
    bool b;
    ofColor c;
    Txt( string s_, ofColor c_, bool b_ = false) {
        s = s_;
        c = c_;
        b = b_;
    }
};



class Cam {
public:
    ofxZEDCamera camera;
    vector<Txt *> text;

    ofxAssimpModelLoader model;
    ofLight	light;
    ofFbo uiFbo;

    ofEasyCam cam;
    float orbX, orbY, orbZ;
    ofRectangle bounds;

	void setup() {

        model.loadModel("zed_model.dae", 20);
		// these rotation set the model so it is oriented correctly
        model.setRotation(0, 90, 1, 0, 0);
		model.setScale(0.9, 0.9, 0.9);
	    light.setPosition(0, 0, 500);

        orbX = ofRandom(0, 999);
        orbY = ofRandom(0, 999);
        orbZ = ofRandom(0, 999);
		cam.setDistance(700);

        setupText();
        drawFbo();

	}

    void setupText() {
        ofColor w(255);
        ofColor y(0,255,255);
        ofColor b(0,255,0);
        ofColor g(0,0,255);

        text.push_back( new Txt("Framerate", w) );
        text.push_back( new Txt("Something", w) );
        text.push_back( new Txt("A", w) );
        text.push_back( new Txt("B", w) );
        text.push_back( new Txt("C", w, true) );


    }

    void drawFbo() {
        uiFbo.allocate(800,800);
        uiFbo.begin();
        ofSetColor(255);
        int y = 20;
        int x = 20;
        for(int i = 0; i < text.size(); i++) {

            ofSetColor( text[i]->c );

            if (!text[i]->b) ofDrawBitmapString( text[i]->s , x, y);

            y += 30;
        }


        uiFbo.end();
    }

    void update(){
        orbX += ofGetLastFrameTime() * 11.11;
//        orbY += ofGetLastFrameTime() * 6.66;
        orbY = ofMap( sin(ofGetElapsedTimef() * 0.2), -1, 1, -10, 10 );
        orbY = 0;
        cam.orbitDeg(orbX, orbY, cam.getDistance(), {0., 0., 0.});
	}

	void draw() {
		
        ofEnableDepthTest();
        cam.begin();
		ofColor(255,255);
		// draws all the other file types which are loaded into model.
//        model.drawFaces();
        model.drawWireframe();
        cam.end();

		ofDisableDepthTest();

        text[4]->s = ofToString( ofRandom(1,10));
        int y = 20;
        int x = 20;
        for(int i = 0; i < text.size(); i++) {

            ofSetColor( text[i]->c );

            if (text[i]->b) ofDrawBitmapString( text[i]->s , x, y);

            y += 30;
        }
	}

};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);


        Cam * c = new Cam();
        std::map<string, Cam *> cameras;
		
};
