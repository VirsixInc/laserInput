#pragma once

#include "ofMain.h"

#include "ofxOsc.h"
#include "ofxMacamPs3Eye.h"
#include "ofxOpenCv.h"
#include "ofxXmlSettings.h"
#include "ofxSimpleGuiToo.h"

using namespace cv;

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);

    ofxCvColorImage iterateImg, warpImg, diffImg;
    ofxCvColorImage transImg;
    ofxCvGrayscaleImage filtered;
    ofxCvContourFinder contours;

    ofxMacamPs3Eye ps3Eye;

    float gain, shutter, gamma, brightness, contrast, hue, flicker;

    unsigned char * videoMirror;
    int camWidth, camHeight, range;
    int minContArea, maxContArea;
    int minBriFind, maxBriFind, minSatFind, maxSatFind, minHueFind, maxHueFind;
    int valRange, satRange, hueRange;

    bool drawCams, configured, savePts, saveBk, resetPts;

    ofPoint dest[4];
    ofPoint src[4];
    int selectedCorner;

    ofxSimpleGuiToo gui;
    ofxOscSender sender;
};