#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowTitle("ofxKinect4Azure example");
	ofxKinect4AzureSettings settings;
	settings.transform_type = DEPTH_TO_COLOR;
	settings.make_colorize_depth = true;
	k4a.setup(settings);
	

	ofSetVerticalSync(false);
}

//--------------------------------------------------------------
void ofApp::update(){
	k4a.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	k4a.draw(0, 0, 800, 450);
	k4a.drawColorizedDepth(0, 460,800,450);
	ofDrawBitmapString(ofGetFrameRate(), 10, 10);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
