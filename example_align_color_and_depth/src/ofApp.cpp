#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowTitle("ofxKinect4Azure example");
	k4a.setup();
	k4a.setTransformType(ofxKinect4Azure::COLOR_TO_DEPTH);
	//setup with settings
	//
	//ofxKinect4AzureSettings settings;
	//settings.camera_fps = K4A_FRAMES_PER_SECOND_30;
	//settings.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	//settings.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	//settings.color_resolution = K4A_COLOR_RESOLUTION_720P;
	//settings.synchronized_images_only = true;
	//
	//k4a.setup(settings);

	ofSetVerticalSync(false);
}

//--------------------------------------------------------------
void ofApp::update(){
	k4a.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	k4a.draw(0, 0, 450, 450);
	k4a.drawColorizedDepth(0, 460,450,450);
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
