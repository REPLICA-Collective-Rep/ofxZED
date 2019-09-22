#include "ofMain.h"
#include "ofApp.h"
#include "ofAppNoWindow.h"

//========================================================================
int main(int argc, char *argv[] ){

    ofApp *app = new ofApp();
    app->arguments = vector<string>(argv, argv + argc);
    ofSetupOpenGL(1024,768, OF_WINDOW);
    ofRunApp(app);

}
