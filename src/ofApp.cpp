#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  sender.setup("localhost", 9999);
    receiver.setup(7600);

  camWidth = 640;
  camHeight = 480;
  
  gui.setup();
  gui.addSlider("Min Bri", minBriFind, 0, 255);
  gui.addSlider("Max Bri", maxBriFind, 0, 255);
  gui.addSlider("Min Sat", minSatFind, 0, 255);
  gui.addSlider("Max Sat", maxSatFind, 0, 255);
  gui.addSlider("Min Hue", minHueFind, 0, 255);
  gui.addSlider("Max Hue", maxHueFind, 0, 255);

  gui.addSlider("Gain", gain, 0, 1);
  gui.addSlider("Shutter", shutter, 0, 1);
  gui.addSlider("Gamma", gamma, 0, 1);
  gui.addSlider("Brightness", brightness, 0, 1);
  gui.addSlider("Contrast", contrast, 0, 1);
  gui.addSlider("Hue", hue, 0, 1);
  gui.addSlider("Flicker", flicker, 1, 2);
  gui.addSlider("minContArea", minContArea, 0, 2000);
  gui.addSlider("maxContArea", maxContArea, 0, 20000);
  gui.addSlider("minVariationDistance", minVariationDistance, 0.01, 500.0);
  gui.addSlider("lifeTime", lifeTime, 0, 80);
  gui.addToggle("Configured", configured);
  gui.addToggle("Save Points", saveCorners);
  //gui.addToggle("Reset Pts", resetPts);
  gui.addToggle("Flip", flip);
  gui.loadFromXML();
  gui.show();

  selectedCorner = -1;
  ps3Eye.listDevices();
  ps3Eye.setDesiredFrameRate(60);
  ps3Eye.setWhiteBalance(4);
  ps3Eye.initGrabber(camWidth, camHeight, false);
  warpImg.allocate(camWidth, camHeight);

  iterateImg.allocate(camWidth, camHeight);
  transImg.allocate(camWidth, camHeight);
  diffImg.allocate(camWidth, camHeight);
  filtered.allocate(camWidth, camHeight);

  src[0] = ofPoint(0,0);
  src[1] = ofPoint(camWidth,0);
  src[2] = ofPoint(0,camHeight);
  src[3] = ofPoint(camWidth,camHeight);
  dest[0] = ofPoint(0,0);
  dest[1] = ofPoint(camWidth,0);
  dest[2] = ofPoint(0,camHeight);
  dest[3] = ofPoint(camWidth,camHeight);

  ballTracker.init(&lifeTime, &minVariationDistance, &minContArea, &maxContArea);

  autoConfigurator.init(camWidth, camHeight);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (!autoConfigurator.isConfigured()) {
        if (!ps3Eye.getAutoGainAndShutter()) {
            ps3Eye.setGain(gain);
            ps3Eye.setShutter(shutter);
        }
        else {
            ofLogWarning("Auto gain and shutter is true!!! SETTING FALSE");
            ps3Eye.setAutoGainAndShutter(false);
        }
        ps3Eye.setGamma(gamma);
        ps3Eye.setBrightness(brightness);
        ps3Eye.setContrast(contrast);
        ps3Eye.setHue(hue);
        ps3Eye.setFlicker(flicker);
        ps3Eye.update();
        if (ps3Eye.isFrameNew()) {
            unsigned char* pixels = ps3Eye.getPixels();
            warpImg.setFromPixels(pixels, camWidth, camHeight);
            warpImg.resize(camWidth, camHeight);
            
            autoConfigurator.configure(ofxCv::toCv(warpImg));

            if (autoConfigurator.isConfigured()) { // Config done
                SendMessage("/config/done");
                SaveCorners();
                autoConfigurator.getCorners(dest);
            }
        }
	} else {
		if (!ps3Eye.getAutoGainAndShutter()) {
			ps3Eye.setGain(gain);
			ps3Eye.setShutter(shutter);
		}
		else {
			ofLogWarning("Auto gain and shutter is true!!! SETTING FALSE");
			ps3Eye.setAutoGainAndShutter(false);
		}
		ps3Eye.setGamma(gamma);
		ps3Eye.setBrightness(brightness);
		ps3Eye.setContrast(contrast);
		ps3Eye.setHue(hue);
		ps3Eye.setFlicker(flicker);
		ps3Eye.update();

		if (ps3Eye.isFrameNew()){
			if (configured) {
				unsigned char* pixels = ps3Eye.getPixels();
				warpImg.setFromPixels(pixels, camWidth, camHeight);
				warpImg.resize(camWidth, camHeight);
				iterateImg.warpIntoMe(warpImg, dest, src);
				iterateImg.convertRgbToHsv();
				filtered.setFromPixels(pixels, camWidth, camHeight);

				ofPixels H = iterateImg.getPixelsRef().getChannel(0);
				ofPixels S = iterateImg.getPixelsRef().getChannel(1);
				ofPixels B = iterateImg.getPixelsRef().getChannel(2);
				for (int i = 0; i < iterateImg.getWidth()*iterateImg.getHeight(); i++){
					if (ofInRange(H.getPixels()[i], minHueFind, maxHueFind) &&
						ofInRange(S.getPixels()[i], minSatFind, maxSatFind) &&
						ofInRange(B.getPixels()[i], minBriFind, maxBriFind)){
						filtered.getPixels()[i] = 255;
					}
					else{
						filtered.getPixels()[i] = 0;
					}
				}

				labels.clear();
				rects.clear();
//                contours.findContours(filtered, minContArea, maxContArea, 8, true);
				ballTracker.track(filtered, &rects, &labels);

				for (int i = 0; i < labels.size(); i++) {
					if (!ballTracker.depthTracked(labels[i])) {
						SendHitMessage("/move", rects[i].getCenter());
					}
				}
			}
		}

		/*if (resetPts) {
			resetPts = false;
			ofxXmlSettings settings;
			settings.addTag("positions");
			settings.pushTag("positions");
			for (int i = 0; i < 4; i++){
				if (dest[i].x != -1 && dest[i].y != -1){
					settings.addTag("position");
					settings.pushTag("position", i);
					settings.setValue("X", i * 10);
					settings.setValue("Y", i * 10);
					settings.popTag();
				}
			}
		}*/

		if (saveCorners) {
			saveCorners = false;
			SaveCorners();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofSetColor(255,255,255);

  for (int j = 0; j < 4; j++){
	ofCircle(dest[j].x, dest[j].y, 3);
  }
  filtered.draw(0,camHeight);
  iterateImg.draw(camWidth,0);
  if(configured){
    contours.draw(0,0);
  } else {
    ps3Eye.draw(0,0);
    ofxXmlSettings settings;
    for(int j=0; j<4;j++){
      settings.loadFile("points.xml");
      settings.pushTag("positions");
      settings.pushTag("position", j);
      if(selectedCorner < 0){
        if(settings.getValue("X", -1) != -1){
          dest[j].x = settings.getValue("X", -1);
        }
        if(settings.getValue("Y", -1) != -1){
          dest[j].y = settings.getValue("Y", -1);
        }
      }
      settings.popTag();
      settings.popTag();
    }
    ofSetColor(0, 0, 255);
    ofCircle(dest[0].x, dest[0].y, 3);
    ofDrawBitmapString("TL", dest[0]);

    ofSetColor(255, 0, 0);
    ofCircle(dest[1].x, dest[1].y, 3);
    ofDrawBitmapString("TR", dest[1]);

    ofSetColor(255, 255, 0);
    ofCircle(dest[2].x, dest[2].y, 3);
    ofDrawBitmapString("BL", dest[2]);

    ofSetColor(0, 255, 0);
    ofCircle(dest[3].x, dest[3].y, 3);
    ofDrawBitmapString("BR", dest[3]);
  }
  gui.draw();
}

//--------------------------------------------------------------
void ofApp::CheckOSCMessage() {
	while (receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		if (receiver.getNextMessage(&m)) {
			string addr = m.getAddress();
			if (addr == "/configure") {
				autoConfigurator.reconfigure();
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::SendMessage(string message) {
	ofxOscMessage m;
	m.setAddress(message);
	sender.sendMessage(m);
}

//--------------------------------------------------------------
void ofApp::SendHitMessage(string message, ofPoint pos) {
	ofxOscMessage m;
	m.setAddress(message);
	float x = pos.x / camWidth;
	float y = pos.y / camHeight;
	if (flip) {
		x = 1 - x;
		y = 1 - y;
	}
	m.addFloatArg(x);
	m.addFloatArg(y);
	sender.sendMessage(m);
}

void ofApp::SaveCorners() {
	ofxXmlSettings settings;
	settings.addTag("positions");
	settings.pushTag("positions");
	for (int i = 0; i < 4; i++){
		settings.addTag("position");
		settings.pushTag("position", i);
		settings.setValue("X", dest[i].x);
		settings.setValue("Y", dest[i].y);
		settings.popTag();
	}
	settings.popTag();
	settings.saveFile("points.xml");
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  switch(key){
    case OF_KEY_LEFT:
      gui.toggleDraw();
      break;
    case 'c':
      drawCams = !drawCams;
      break;
  }


}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
  if (selectedCorner > -1){
      dest[selectedCorner].x = x;
      dest[selectedCorner].y = y;
  }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

    //this is not the best way
  selectedCorner       = -1;

  float smallestDist  = 999999;
	float clickRadius = 10;

	for (int j = 0; j < 4; j++){
		ofPoint inputPt;
		inputPt.x = dest[j].x;
		inputPt.y = dest[j].y;
		inputPt.z = 0;
		float len = sqrt( (inputPt.x - x) * (inputPt.x - x) +
							(inputPt.y - y) * (inputPt.y - y));
		if (len < clickRadius && len < smallestDist){
			selectedCorner  = j;
			smallestDist = len;
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
}
