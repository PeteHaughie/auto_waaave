/*
 * Copyright (c) 2013 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/danomatika/ofxMidi for documentation
 *
 */
#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxProcessFFT.h"

class ofApp : public ofBaseApp, public ofxMidiListener
{

public:
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);

	void midibiz();
	void midibizOld();
	void newMidiMessage(ofxMidiMessage &eventArgs);
	void midiSetup();
	ofxMidiIn midiIn;
	std::vector<ofxMidiMessage> midiMessages;
	std::size_t maxMessages = 10; //< max number of messages to keep track of

	ofShader shaderMixer;
	ofShader shaderSharpen;

	void fbDeclareAndAllocate();
	ofFbo framebuffer0;
	ofFbo framebuffer1;
	ofFbo sharpenFramebuffer;

	void inputSetup();
	void inputUpdate();
	ofVideoGrabber cam;
	unsigned long int midiID = 0, prevMidiID = 0;

	void p_lockClear();
	void p_lockUpdate();

	ProcessFFT fft;
	void fftAssignValues();

	void parametersAssign();
	void midiLatchClear();
};
