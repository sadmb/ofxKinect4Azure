#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofxKinect4AzureSettings settings;
	settings.make_pointcloud = true;
	settings.transform_type = DEPTH_TO_COLOR;
	k4a.setup(settings);

	ofSetVerticalSync(false);
}

//--------------------------------------------------------------
void ofApp::update(){
	k4a.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	auto& pointcloud = k4a.getPointcloudPix();
	auto data = pointcloud.getData();
	auto color = k4a.getPixels().getData();
	cam.begin();
	glBegin(GL_POINTS);
	ofPushStyle();
	int w = pointcloud.getWidth();
	int h = pointcloud.getHeight();
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			int id = j * w * 3 + i * 3;
			int c_id= j * pointcloud.getWidth() * 4 + i * 4;
			glColor3f(color[c_id + 2] / 255.f, color[c_id + 1] / 255.f, color[c_id] / 255.f);
			glVertex3f(data[id], data[id + 1], -data[id + 2]);
		}
	}
	ofPopStyle();
	glEnd();
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
