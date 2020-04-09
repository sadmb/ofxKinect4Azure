#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	auto serial_list = manager.getDeviceSerialList();
	k4a.resize(serial_list.size());
	ofxKinect4AzureSettings settings;
	settings.make_pointcloud = true;
	settings.transform_type = DEPTH_TO_COLOR;

	for (int i = 0; i < serial_list.size(); i++) {
		k4a[i] = ofPtr<k4aThread>(new k4aThread);
		manager.setupWithSerial(serial_list[i], k4a[i]->getK4aRef(), settings);
		k4a[i]->startThread();
	}

	ofSetVerticalSync(false);
}

//--------------------------------------------------------------
void ofApp::update(){
	for (auto& k : k4a) {
		k->updatePointcloud();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	cam.begin();
	ofPushMatrix();
	ofScale(1, -1, -1);
	ofEnableDepthTest();
	for (auto& k : k4a) {
		auto& tex = k->getTexture();
		auto& pointcloud = k->getPointcloud();
		tex.bind();
		pointcloud.draw(GL_POINTS, 0,pointcloud.getNumVertices());
		tex.unbind();
	}
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
