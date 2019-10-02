#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofxKinect4AzureSettings settings;
	settings.enable_imu = true;
	k4a.setup(settings);
	ofSetVerticalSync(true);
}

//--------------------------------------------------------------
void ofApp::update(){
	k4a.update();
	auto& imus = k4a.popImuSampleQueue();
	for (auto& value : imus) {
		imu.push_back(value);
	}
	if (imu.size() > imu_buffer_size) {
		int gap = imu.size() - imu_buffer_size;
		imu.erase(imu.begin(), imu.begin()+gap);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (imu.size()) {
		ofPushMatrix();
		ofTranslate(0, ofGetHeight() / 2);
		uint64_t first = imu.back().acc.first;
		for (int i = imu.size() - 1; i > 0; i--) {
			ofSetColor(ofColor::red);
			ofDrawLine((first - imu[i].acc.first) / 2000, imu[i].acc.second.x*10, (first - imu[i - 1].acc.first) / 2000, imu[i - 1].acc.second.x*10);
			ofSetColor(ofColor::green);
			ofDrawLine((first - imu[i].acc.first) / 2000, imu[i].acc.second.y * 10, (first - imu[i - 1].acc.first) / 2000, imu[i - 1].acc.second.y * 10);
			ofSetColor(ofColor::blue);
			ofDrawLine((first - imu[i].acc.first) / 2000, imu[i].acc.second.z * 10, (first - imu[i - 1].acc.first) / 2000, imu[i - 1].acc.second.z * 10);
		}
		ofPopMatrix();
	}
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
