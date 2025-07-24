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
#include "VideoInputManager.h"

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

  vector<string> midiPortNames;
  int currentMidiPortIndex = 0;

  bool midiDeviceConnected = false;
  uint64_t lastMidiPollTime = 0;
  const uint64_t midiPollInterval = 2000; // ms

  ofxMidiIn midiIn;
  std::vector<ofxMidiMessage> midiMessages;
  std::size_t maxMessages = 10; //< max number of messages to keep track of
  void incrementMidiDeviceID();

  ofShader shaderMixer;
  ofShader shaderSharpen;

  void fbDeclareAndAllocate();
  ofFbo framebuffer0;
  ofFbo framebuffer1;
  ofFbo sharpenFramebuffer;

  void inputSetup();
  void inputUpdate();
  // ofVideoGrabber cam;
  VideoInputManager videoInputManager;
  unsigned long int camID = 0, prevCamID = 0;
  unsigned long int midiID = 0, prevMidiID = 0;

  void p_lockClear();
  void p_lockUpdate();

  ProcessFFT fft;
  void fftAssignValues();

  void parametersAssign();
  void midiLatchClear();
};
