#pragma once

#include "ofMain.h"
#include "ofxKinect4Azure.h"

class ofApp : public ofBaseApp {
public:
	ofxKinect4Azure k4a;
	ofEasyCam cam;
	vector<vector<ofConePrimitive>> bones;
	vector<vector<ofSpherePrimitive>> axises;
	int num_bodies = 0;

	//for joint index
	vector<k4abt_joint_id_t> pelvis_to_head = {
		K4ABT_JOINT_PELVIS,
		K4ABT_JOINT_SPINE_NAVEL,
		K4ABT_JOINT_SPINE_CHEST,
		K4ABT_JOINT_NECK,
		K4ABT_JOINT_HEAD,
		K4ABT_JOINT_NOSE
	};

	vector<string> left_right = {
		"LEFT",
		"RIGHT"
	};

	vector<string> clavicle_to_wrist{
		"CLAVICLE_",
		"SHOULDER_",
		"ELBOW_",
		"WRIST_"
	};

	vector<string> hip_to_foot = {
		"HIP_",
		"KNEE_",
		"ANKLE_",
		"FOOT_"
	};

	vector<string> eye_ear = {
		"EYE_",
		"EAR_"
	};

	vector<ofColor> id_to_color = {
		ofColor::red,
		ofColor::blue,
		ofColor::green,
		ofColor::skyBlue,
		ofColor::orange,
		ofColor::yellow,
		ofColor::purple,
		ofColor::pink,
		ofColor::lime,
		ofColor::crimson,
		ofColor::red,
		ofColor::blue,
		ofColor::green,
		ofColor::skyBlue,
		ofColor::orange,
		ofColor::yellow,
		ofColor::purple,
		ofColor::pink,
		ofColor::lime,
		ofColor::crimson
	};
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
