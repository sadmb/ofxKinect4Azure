#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofxKinect4AzureSettings settings;
	settings.make_pointcloud = true;
	settings.transform_type = DEPTH_TO_COLOR;
	k4a.setup(settings);
	cam.setVFlip(true);

	ofSetVerticalSync(false);

}

//--------------------------------------------------------------
void ofApp::update(){
	k4a.update();
	k4a.updatePointcloud();
}

//--------------------------------------------------------------
void ofApp::draw(){
	
	cam.begin();
	ofPushMatrix();
	ofScale(1, 1, -1);
	ofEnableDepthTest();

	auto& pointcloud = k4a.getPointcloudVbo();
	pointcloud.draw(GL_POINTS, 0, pointcloud.getNumVertices());

	ofDisableDepthTest();
	ofPopMatrix();
	cam.end();

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
