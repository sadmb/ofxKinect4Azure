#pragma once

#include "ofMain.h"
#include "ofxKinect4Azure.h"
#include "k4aThread.h"

class ofApp : public ofBaseApp{

public:
	ofxKinect4AzureManager manager;
	vector<ofPtr<k4aThread>> k4a;
	ofEasyCam cam;

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
};
