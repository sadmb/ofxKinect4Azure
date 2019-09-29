#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofxKinect4AzureSettings settings;
	settings.use_tracker=true;
	settings.use_ir_image = true;
	settings.make_pointcloud = true;

	k4a.setup(settings);

	cam.setVFlip(true);
}

//--------------------------------------------------------------
void ofApp::update(){
	k4a.update();
	k4a.updatePointcloud();
}

//--------------------------------------------------------------
void ofApp::draw(){
	cam.begin();
	ofEnableDepthTest();
	auto& tex = k4a.getIRTexture();
	auto& pointcloud = k4a.getPointcloudVbo();
	tex.bind();
	pointcloud.draw(GL_POINTS, 0, pointcloud.getNumVertices());
	tex.unbind();
	ofDisableDepthTest();
	ofSetColor(ofColor::red);
	for (auto b : k4a.bones) {
		for (auto b_ : b) {
			ofDrawSphere(b_, 10);
		}
	}
	ofSetColor(255);
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
