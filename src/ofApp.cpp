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
#include "ofApp.h"

#include "iostream"

#define MIDI_MAGIC 63.50f

#define CONTROL_THRESHOLD .035f

// flip this switch to try different scalings
// 0 is 320 1 is 640
// if you reduce scale to 320 you can up the total delay time to
// about 4 seconds/ 120 frames
// so try that out sometime and see how that feels!
bool scaleswitch = 1;
const int framebufferLength = 60;
// const int framebufferLength=120;

// 0 is wet (framebuffer fed from final output, internal
// feedback mode
// 1 is dry (framebuffer fed direct from camera input,
// traditional video delay mode
bool wet_dry_switch = 1;

// 0 is sd aspect ratio
// use definitely with all of the VSERPI devices
// and anything else doing like 480i/p over hdmi
// 1 is corner cropping to fill the screen
int hdmi_aspect_ratio_switch = 0;

float az = 1.0;
float sx = 0;
float dc = 0;
float fv = 1;
float gb = 1;
float hn = 1;
float jm = 0.0;
float kk = 0.0;
float ll = 0.0;
float qw = 0.0;
float er = 1.0;
float ty = 0.0;
float ui = 0.0;
float op = 0.0;

float fb1_brightx = 0.0;
bool toroidSwitch = 0;
bool y_skew_switch = false;
bool x_skew_switch = false;
bool lumakeyInvertSwitch = false;
bool x_mirrorSwitch = false;
bool y_mirrorSwitch = false;
float y_skew = 0;
float x_skew = 0;
bool mirrorSwitch = false;

int fb0_delayamount = 0;

// dummy variables for midi control

int width = 640;
int height = 480;

// dummy variables for midi to audio attenuatiors
// 0 is direct midi, 1 is low_x, 2 is mid_x, 3 is high_x
int audioReactiveControlSwitch = 0;

float lowC1 = 0.0;
float lowC2 = 0.0;
float lowC3 = 0.0;
float lowC4 = 0.0;
float lowC5 = 0.0;
float lowC6 = 0.0;
float lowC7 = 0.0;
float lowC8 = 0.0;
float lowC9 = 0.0;
float lowC10 = 0.0;
float lowC11 = 0.0;
float lowC12 = 0.0;
float lowC13 = 0.0;
float lowC14 = 0.0;
float lowC15 = 0.0;
float lowC16 = 0.0;

float midC1 = 0.0;
float midC2 = 0.0;
float midC3 = 0.0;
float midC4 = 0.0;
float midC5 = 0.0;
float midC6 = 0.0;
float midC7 = 0.0;
float midC8 = 0.0;
float midC9 = 0.0;
float midC10 = 0.0;
float midC11 = 0.0;
float midC12 = 0.0;
float midC13 = 0.0;
float midC14 = 0.0;
float midC15 = 0.0;
float midC16 = 0.0;

float highC1 = 0.0;
float highC2 = 0.0;
float highC3 = 0.0;
float highC4 = 0.0;
float highC5 = 0.0;
float highC6 = 0.0;
float highC7 = 0.0;
float highC8 = 0.0;
float highC9 = 0.0;
float highC10 = 0.0;
float highC11 = 0.0;
float highC12 = 0.0;
float highC13 = 0.0;
float highC14 = 0.0;
float highC15 = 0.0;
float highC16 = 0.0;

bool clear_switch = 0;
// toggle switch variables
bool hueInvert = false;
bool saturationInvert = false;
bool brightInvert = false;

bool cam_h_invert = false;
bool cam_s_invert = false;
bool cam_b_invert = false;

bool horizontalMirror = false;
bool verticalMirror = false;

bool clear_flip = false;

bool x_2 = false;
bool x_5 = false;
bool x_10 = false;

bool y_2 = false;
bool y_5 = false;
bool y_10 = false;

bool z_2 = false;
bool z_5 = false;
bool z_10 = false;

bool r_2 = false;
bool r_5 = false;
bool r_10 = false;

bool huexx_0 = false;
bool huexx_1 = false;
bool huexx_2 = false;

bool huexy_0 = false;
bool huexy_1 = false;
bool huexy_2 = false;

bool huexz_0 = false;
bool huexz_1 = false;
bool huexz_2 = false;

// framebuffer management biziness

// framebufferLength is the total number of frames that will be stored.  setting
// framebufferLength to 30 while fps is set to 30 means 1 second of video will
// be stored in memory 75 seems to work ok with usb cams but the capture device
// might need a little more memory

int framedelayoffset =
    0;                       // this is used as an index to the circular framebuffers eeettt
unsigned int framecount = 0; // framecounter used to calc offset eeettt

// this is an array of framebuffer objects
ofFbo pastFrames[framebufferLength];

void incIndex() // call this every frame to calc the offset eeettt
{
  framecount++;
  framedelayoffset = framecount % framebufferLength;
}

// p_lock biz
// maximum total size of the plock array
const int p_lock_size = 240;

bool p_lock_switch = 0;

bool p_lock_erase = 0;

// maximum number of p_locks available...maybe there can be one for every knob
// for whatever wacky reason the last member of this array of arrays has a
// glitch so i guess just make an extra array and forget about it for now
const int p_lock_number = 17;

// so how we will organize the p_locks is in multidimensional arrays
// to access the data at timestep x for p_lock 2 (remember counting from 0) we
// use p_lock[2][x]
float p_lock[p_lock_number][p_lock_size];

// smoothing parameters(i think for all of these we can array both the arrays
// and the floats for now let us just try 1 smoothing parameter for everything.
float p_lock_smooth = .5;

// and then lets try an array of floats for storing the smoothed values
float p_lock_smoothed[p_lock_number];

// turn on and off writing to the array
bool p_lock_0_switch = 1;

// global counter for all the locks
int p_lock_increment = 0;

float my_normalize = 0;

float low_value_smoothed = 0.0;
float mid_value_smoothed = 0.0;
float high_value_smoothed = 0.0;
float smoothing_rate = .8;

ofFbo aspect_fix_fbo;
ofFbo dry_framebuffer;

bool videoReactiveSwitch = false;

float vLumakeyValue = 0.0;
float vMix = 0.0;
float vHue = 0.0;
float vSaturation = 0.0;
float vBright = 0.0;
float vTemporalFilterMix = 0.0;
float vTemporalFilterResonance = 0.0;
float vSharpenAmount = 0.0;
float vX = 0.0;
float vY = 0.0;
float vZ = 0.0;
float vRotate = 0.0;
float vHuexMod = 0.0;
float vHuexOff = 0.0;
float vHuexLfo = 0.0;

// latching
bool midiActiveFloat[17];
bool vmidiActiveFloat[17];
bool midiLowActiveFloat[17];
bool midiMidActiveFloat[17];
bool midiHighActiveFloat[17];

float range = 200;
float fftLow;
float fftMid;
float fftHigh;

float c_lumakey_value = 1.01;
float c_mix = 2;
float c_hue = .25;
float c_sat = .2;
float c_bright = .2;
float c_temporalFilterMix = 1.1;
float c_sharpenAmount = .5;
float c_x = .01;
float c_y = .01;
float c_z = .05;
float c_rotate = .314159265;
float c_huex_off = .25;
float c_huex_lfo = .25;

int d_delay;
float d_lumakey_value;
float d_mix;
float d_hue;
float d_sat;
float d_bright;
float d_temporalFilterMix;
float d_temporalFilterResonance;
float d_sharpenAmount;
float d_x;
float d_y;
float d_z;
float d_rotate;
float d_huex_mod;
float d_huex_off;
float d_huex_lfo;
//--------------------------------------------------------------
void ofApp::setup()
{
  // ofSetVerticalSync(true);
  ofSetFrameRate(30);
  ofBackground(ofColor::red);
  ofDisableArbTex();
  // ofToggleFullscreen();
  ofHideCursor();
  inputSetup();
  midiSetup();
  fbDeclareAndAllocate();
  shaderMixer.load("shadersES2/shaderMixer");
  shaderSharpen.load("shadersES2/shaderSharpen");
  // fft biz
  fft.setup();
  fft.setNormalize(false);
  // fft.setVolumeRange(1.0f);

  p_lockClear();
  midiLatchClear();
}
//-------------------------------------------------------------
void ofApp::midiLatchClear()
{
  for (int i = 0; i < 17; i++)
  {
    vmidiActiveFloat[i] = 0;
    midiActiveFloat[i] = 0;
    midiLowActiveFloat[i] = 0;
    midiMidActiveFloat[i] = 0;
    midiHighActiveFloat[i] = 0;
  }
}
//--------------------------------------------------------------
void ofApp::update()
{

  fft.update();
  inputUpdate();

  if (midiID != prevMidiID)
  {
    midiIn.closePort();
    midiIn.openPort(midiID);
    ofLog() << "midiID: " << midiID;
    prevMidiID = midiID;
  }

  midibiz();
  p_lockUpdate();

  if (fft.getLowVal() > my_normalize)
  {
    my_normalize = fft.getLowVal();
  }
}

//--------------------------------------------------------------
void ofApp::draw()
{

  // the range changes depending on what yr using for sound input.  just usb
  // cams have shitter ranges so 50 works float range=200;

  fftAssignValues();

  parametersAssign();

  framebuffer0.begin();
  shaderMixer.begin();

  // videoGrabber.getTextureReference().draw(0, 0, 320, 640);
  if (scaleswitch == 0)
  {
    cam.draw(0, 0, 320, 240);
  }
  if (scaleswitch == 1)
  {
    if (hdmi_aspect_ratio_switch == 0)
    {
      cam.draw(0, 0, 640, 480);
    }
    if (hdmi_aspect_ratio_switch == 1)
    {
      aspect_fix_fbo.draw(0, 0, 640, 480);
    }
  }

  /*
   //textures
   shaderMixer.setUniformTexture("fb0", pastFrames[d_delay].getTexture(),1);

   if(wet_dry_switch==1){
   shaderMixer.setUniformTexture("fb1",
   pastFrames[(abs(framedelayoffset-framebufferLength)-1)%framebufferLength].getTexture(),2);
   }
   if(wet_dry_switch==0){
   shaderMixer.setUniformTexture("fb1", dry_framebuffer.getTexture(),2);
   }
   //continuous variables
   shaderMixer.setUniform1f("fb0_lumakey_value",d_lumakey_value);
   shaderMixer.setUniform1f("fb0_mix",d_mix);
   shaderMixer.setUniform3f("fb0_hsbx",ofVec3f(d_hue,d_sat,d_bright));
   shaderMixer.setUniform1f("temporalFilterMix",d_temporalFilterMix);
   shaderMixer.setUniform1f("fb1_brightx",d_temporalFilterResonance );
   shaderMixer.setUniform1f("cam_brightx",d_cam_x);
   shaderMixer.setUniform1f("fb0_xdisplace",d_x);
   shaderMixer.setUniform1f("fb0_ydisplace",d_y);
   shaderMixer.setUniform1f("fb0_zdisplace",d_z);
   shaderMixer.setUniform1f("fb0_rotate",d_rotate);
   shaderMixer.setUniform3f("fb0_huex",ofVec3f(d_huex_mod,d_huex_off,d_huex_lfo));

   if(wet_dry_switch==1){
   shaderMixer.setUniform1i("toroidSwitch",toroidSwitch);
   shaderMixer.setUniform1i("mirrorSwitch",0);
   }

   if(wet_dry_switch==0){
   shaderMixer.setUniform1i("toroidSwitch",0);
   shaderMixer.setUniform1i("mirrorSwitch",toroidSwitch);
   }


   shaderMixer.setUniform1i("brightInvert",brightInvert);
   shaderMixer.setUniform1i("hueInvert",hueInvert);
   shaderMixer.setUniform1i("saturationInvert",saturationInvert);

   shaderMixer.setUniform1i("horizontalMirror",horizontalMirror);
   shaderMixer.setUniform1i("verticalMirror",verticalMirror);


   shaderMixer.setUniform1i("lumakeyInvertSwitch",lumakeyInvertSwitch);
   */

  // send the textures
  shaderMixer.setUniformTexture("fb", pastFrames[d_delay].getTexture(), 1);
  if (wet_dry_switch == 1)
  {
    shaderMixer.setUniformTexture(
        "temporalFilter",
        pastFrames[(abs(framedelayoffset - framebufferLength) - 1) %
                   framebufferLength]
            .getTexture(),
        2);
  }
  if (wet_dry_switch == 0)
  {
    shaderMixer.setUniformTexture("temporalFilter",
                                  dry_framebuffer.getTexture(), 2);
  }
  // send the continuous variables
  shaderMixer.setUniform2f("resolution",
                             ofGetWidth(), ofGetHeight());
  shaderMixer.setUniform1f("lumakey", d_lumakey_value);
  shaderMixer.setUniform1f("fbMix", d_mix);
  shaderMixer.setUniform1f("fbHue", d_hue);
  shaderMixer.setUniform1f("fbSaturation", d_sat);
  shaderMixer.setUniform1f("fbBright", d_bright);
  shaderMixer.setUniform1f("temporalFilterMix", d_temporalFilterMix);
  shaderMixer.setUniform1f("temporalFilterResonance",
                           d_temporalFilterResonance);
  shaderMixer.setUniform1f("fbXDisplace", d_x);
  shaderMixer.setUniform1f("fbYDisplace", d_y);
  shaderMixer.setUniform1f("fbZDisplace", d_z);
  shaderMixer.setUniform1f("fbRotate", d_rotate);
  shaderMixer.setUniform3f("fb_huex",
                           ofVec3f(d_huex_mod, d_huex_off, d_huex_lfo));
  shaderMixer.setUniform1f("fbHuexMod", d_huex_mod);
  shaderMixer.setUniform1f("fbHuexOff", d_huex_off);
  shaderMixer.setUniform1f("fbHuexLfo", d_huex_lfo);

  // send the switches
  shaderMixer.setUniform1i("toroidSwitch", toroidSwitch);
  shaderMixer.setUniform1i("mirrorSwitch", mirrorSwitch);
  shaderMixer.setUniform1i("brightInvert", brightInvert);
  shaderMixer.setUniform1i("hueInvert", hueInvert);
  shaderMixer.setUniform1i("saturationInvert", saturationInvert);
  shaderMixer.setUniform1i("horizontalMirror", horizontalMirror);
  shaderMixer.setUniform1i("verticalMirror", verticalMirror);
  shaderMixer.setUniform1i("lumakeyInvertSwitch", lumakeyInvertSwitch);

  // send the videoreactive controls
  shaderMixer.setUniform1f("vLumakey", vLumakeyValue * c_lumakey_value);
  shaderMixer.setUniform1f("vMix", vMix * c_mix);
  shaderMixer.setUniform1f("vHue", 2.0f * vHue * c_hue);
  shaderMixer.setUniform1f("vSat", 2.0f * vSaturation * c_sat);
  shaderMixer.setUniform1f("vBright", 2.0f * vBright * c_bright);
  shaderMixer.setUniform1f("vtemporalFilterMix",
                           vTemporalFilterMix * c_temporalFilterMix);
  shaderMixer.setUniform1f("vFb1X", vTemporalFilterResonance);
  shaderMixer.setUniform1f("vX", .01 * vX);
  shaderMixer.setUniform1f("vY", .01 * vY);
  shaderMixer.setUniform1f("vZ", vZ * c_z);
  shaderMixer.setUniform1f("vRotate", vRotate * c_rotate);
  shaderMixer.setUniform1f("vHuexMod", 1.0f - vHuexMod);
  shaderMixer.setUniform1f("vHuexOff", vHuexOff * c_huex_off);
  shaderMixer.setUniform1f("vHuexLfo", vHuexLfo * c_huex_lfo);
  shaderMixer.end();
  framebuffer0.end();

  /**sharpening mode**/
  sharpenFramebuffer.begin();
  shaderSharpen.begin();
  framebuffer0.draw(0, 0, ofGetWidth(), ofGetHeight());
  shaderSharpen.setUniform2f("resolution",
                             ofGetWidth(), ofGetHeight());
  shaderSharpen.setUniform1f("sharpenAmount", d_sharpenAmount);
  shaderSharpen.setUniform1f("vSharpenAmount",
                             vSharpenAmount * c_sharpenAmount);
  shaderSharpen.end();
  sharpenFramebuffer.end();

  // framebuffer0.draw(0,0,ofGetWidth(),ofGetHeight());
  sharpenFramebuffer.draw(0, 0, ofGetWidth(), ofGetHeight());

  pastFrames[abs(framebufferLength - framedelayoffset) - 1].begin(); // eeettt
  if (wet_dry_switch == 0)
  {
    if (hdmi_aspect_ratio_switch == 0)
    {
      cam.draw(0, 0, 640, 480);
    }
    if (hdmi_aspect_ratio_switch == 1)
    {
      aspect_fix_fbo.draw(0, 0, 640, 480);
    }

    dry_framebuffer.begin();
    // framebuffer0.draw(0,0);
    sharpenFramebuffer.draw(0, 0);
    dry_framebuffer.end();

  } // endifwetdry0

  if (wet_dry_switch == 1)
  {
    // framebuffer0.draw(0,0);
    sharpenFramebuffer.draw(0, 0);
  } // endifwetdry1

  pastFrames[abs(framebufferLength - framedelayoffset) - 1].end(); // eeettt
  incIndex();

  // p_lock biz
  if (p_lock_switch == 1)
  {
    p_lock_increment++;
    p_lock_increment = p_lock_increment % p_lock_size;
  }

  // ofDrawBitmapString("fps =" + ofToString(ofGetFrameRate())+"fft low =" +
  // ofToString(fftLow)+"fft low_smoothed =" +
  // ofToString(low_value_smoothed)+"fft mid =" + ofToString(fftLow)+"fft
  // mid_smoothed =" + ofToString(mid_value_smoothed), 10, ofGetHeight() - 5 );
  /*ofDrawBitmapString("fft high =" + ofToString(fftHigh,2)+
   "fft high_smoothed =" + ofToString(high_value_smoothed,2)+
   "fft mid =" + ofToString(fftMid,2)+
   "fft mid_smoothed =" + ofToString(mid_value_smoothed,2)+
   "fft low =" + ofToString(fftLow,2)+
   "fft low_smoothed =" + ofToString(low_value_smoothed,2)
   , 10, ofGetHeight() - 5 );
   */

  /*
   ofDrawBitmapString(
   "fft high =" + ofToString(high_value_smoothed,2)+
   "fft mid =" + ofToString(mid_value_smoothed,2)+
   "fft low =" + ofToString(low_value_smoothed,2)
   , 10, ofGetHeight() - 5 );
   */

  // i use this block of code to print out like useful information for debugging
  // various things and/or just to keep the framerate displayed to make sure i'm
  // not losing any frames while testing out new features.  uncomment the
  // ofDrawBitmap etc etc to print the stuff out on screen
  ofSetColor(255);
  string msg = "fps=" + ofToString(ofGetFrameRate(), 2) + " clear switch" +
               ofToString(clear_switch, 5); //+" z="+ofToString(az,5);
  /*
  ofDrawBitmapString(msg, 10, ofGetHeight() - 20);

  cam.draw(0, 0, ofGetWidth() / 4, ofGetHeight() / 4);
  framebuffer0.draw(ofGetWidth() / 4, 0, ofGetWidth() / 4,
                          ofGetHeight() / 4);
  framebuffer1.draw(ofGetWidth() / 2, 0, ofGetWidth() / 4,
                          ofGetHeight() / 4);
  sharpenFramebuffer.draw(ofGetWidth() * 3 / 4, 0, ofGetWidth() / 4,
                          ofGetHeight() / 4);
  */
}

//--------------------------------------------------------------
void ofApp::exit()
{

  // clean up
  midiIn.closePort();
  midiIn.removeListener(this);
}

//--------------------------------------------------------------
void ofApp::fftAssignValues()
{
  fftLow = fft.getLowVal();
  fftLow = ofClamp(fftLow, 1.0f, range);
  fftLow = fftLow / range;
  low_value_smoothed =
      fftLow * smoothing_rate + (1.0f - smoothing_rate) * low_value_smoothed;

  fftMid = fft.getMidVal();
  fftMid = ofClamp(fftMid, 1.0f, range);
  fftMid = fftMid / range;
  mid_value_smoothed =
      fftMid * smoothing_rate + (1.0f - smoothing_rate) * mid_value_smoothed;

  fftHigh = fft.getHighVal();
  fftHigh = ofClamp(fftHigh, 1.0f, range);
  fftHigh = fftHigh / range;
  high_value_smoothed =
      fftHigh * smoothing_rate + (1.0f - smoothing_rate) * high_value_smoothed;
}

//-------------------------------------------------------------
void ofApp::parametersAssign()
{
  if (wet_dry_switch == 0)
  {
    c_hue = 1.0;
    c_sat = 1.0;
    c_bright = 1.0;
    c_x = .1;
    c_y = .1;
    c_z = .5;
  }

  d_delay =
      (abs(framedelayoffset - framebufferLength - fb0_delayamount -
           int((p_lock_smoothed[15] + lowC16 * low_value_smoothed +
                midC16 * mid_value_smoothed + highC16 * high_value_smoothed) *
               (framebufferLength - 1.0))) -
       1) %
      framebufferLength;
  d_lumakey_value =
      kk + c_lumakey_value * p_lock_smoothed[0] +
      4.0f * (lowC1 * low_value_smoothed + midC1 * mid_value_smoothed +
              highC1 * high_value_smoothed);
  d_mix = jm + c_mix * p_lock_smoothed[1] +
          4.0f * (lowC2 * low_value_smoothed + midC2 * mid_value_smoothed +
                  highC2 * high_value_smoothed);
  d_hue = fv * (1.0f + c_hue * p_lock_smoothed[2]) +
          4.0f * (lowC3 * low_value_smoothed + midC3 * mid_value_smoothed +
                  highC3 * high_value_smoothed);
  d_sat = gb * (1.0f + c_sat * p_lock_smoothed[3]) +
          4.0f * (lowC4 * low_value_smoothed + midC4 * mid_value_smoothed +
                  highC4 * high_value_smoothed);
  d_bright = hn * (1.0f + c_bright * p_lock_smoothed[4]) +
             4.0f * (lowC5 * low_value_smoothed + midC5 * mid_value_smoothed +
                     highC5 * high_value_smoothed);
  d_temporalFilterMix =
      op + c_temporalFilterMix * p_lock_smoothed[5] +
      4.0f * (lowC6 * low_value_smoothed + midC6 * mid_value_smoothed +
              highC6 * high_value_smoothed);
  d_temporalFilterResonance =
      fb1_brightx + p_lock_smoothed[6] +
      4.0 * (lowC7 * low_value_smoothed + midC7 * mid_value_smoothed +
             highC7 * high_value_smoothed);
  d_sharpenAmount =
      ll + c_sharpenAmount * p_lock_smoothed[7] +
      8.0f * (lowC8 * low_value_smoothed + midC8 * mid_value_smoothed +
              highC8 * high_value_smoothed);
  d_x = sx + c_x * p_lock_smoothed[8] +
        4.0f * (c_x * (lowC9 * low_value_smoothed + midC9 * low_value_smoothed +
                       highC9 * high_value_smoothed));
  d_y =
      dc + c_y * p_lock_smoothed[9] +
      4.0f * (c_y * (lowC10 * low_value_smoothed + midC10 * mid_value_smoothed +
                     highC10 * high_value_smoothed));
  d_z =
      az * (1.0f + c_z * p_lock_smoothed[10]) +
      4.0f * (c_z * (lowC11 * low_value_smoothed + midC11 * mid_value_smoothed +
                     highC11 * high_value_smoothed));
  d_rotate = qw + c_rotate * p_lock_smoothed[11] +
             4.0 * (c_rotate *
                    (lowC12 * low_value_smoothed + midC12 * mid_value_smoothed +
                     highC12 * high_value_smoothed));
  d_huex_mod =
      er * (1.0f - p_lock_smoothed[12]) +
      4.0f * (lowC13 * low_value_smoothed + midC13 * mid_value_smoothed +
              highC13 * high_value_smoothed);
  d_huex_off = ty + c_huex_off * p_lock_smoothed[13] +
               4.0f * (c_huex_off * (lowC14 * low_value_smoothed +
                                     midC14 * mid_value_smoothed +
                                     highC14 * high_value_smoothed));
  d_huex_lfo = ui + c_huex_lfo * p_lock_smoothed[14] +
               4.0f * (c_huex_lfo * (lowC15 * low_value_smoothed +
                                     midC15 * mid_value_smoothed +
                                     highC15 * high_value_smoothed));
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage &msg)
{
  // add the latest message to the message queue
  midiMessages.push_back(msg);
  // remove any old messages if we have too many
  while (midiMessages.size() > 2)
  {
    midiMessages.erase(midiMessages.begin());
  }
}

//------------------------------------------------------------
void ofApp::p_lockUpdate()
{
  for (int i = 0; i < p_lock_number; i++)
  {
    p_lock_smoothed[i] = p_lock[i][p_lock_increment] * (1.0f - p_lock_smooth) +
                         p_lock_smoothed[i] * p_lock_smooth;
    if (abs(p_lock_smoothed[i]) < .05)
    {
      p_lock_smoothed[i] = 0;
    }
  }
}
//-------------------------------------------------------------
void ofApp::inputUpdate()
{
  cam.update();
  // corner crop and stretch to preserve hd aspect ratio
  if (hdmi_aspect_ratio_switch == 1)
  {
    aspect_fix_fbo.begin();
    cam.draw(0, 0, 853, 480);
    aspect_fix_fbo.end();
  }
}
//------------------------------------------------------------
void ofApp::p_lockClear()
{
  for (int i = 0; i < p_lock_number; i++)
  {
    for (int j = 0; j < p_lock_size; j++)
    {
      p_lock[i][j] = 0;
    }
  }
}

//------------------------------------------------------------
void ofApp::midiSetup()
{
  // print input ports to console
  midiIn.listInPorts();

  // open port by number (you may need to change this)
  midiIn.openPort(midiID);
  // midiIn.openPort("IAC Pure Data In");	// by name
  // midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port

  // don't ignore sysex, timing, & active sense messages,
  // these are ignored by default
  midiIn.ignoreTypes(false, false, false);

  // add ofApp as a listener
  midiIn.addListener(this);

  // print received messages to the console
  midiIn.setVerbose(true);
}

//-------------------------------------------------------------
void ofApp::fbDeclareAndAllocate()
{
  framebuffer0.allocate(width, height);
  framebuffer0.begin();
  ofClear(0, 0, 0, 255);
  framebuffer0.end();

  aspect_fix_fbo.allocate(width, height);
  aspect_fix_fbo.begin();
  ofClear(0, 0, 0, 255);
  aspect_fix_fbo.end();

  dry_framebuffer.allocate(width, height);
  dry_framebuffer.begin();
  ofClear(0, 0, 0, 255);
  dry_framebuffer.end();

  sharpenFramebuffer.allocate(width, height);
  sharpenFramebuffer.begin();
  ofClear(0, 0, 0, 255);
  sharpenFramebuffer.end();

  for (int i = 0; i < framebufferLength; i++)
  {
    pastFrames[i].allocate(width, height);
    pastFrames[i].begin();
    ofClear(0, 0, 0, 255);
    pastFrames[i].end();
  }
}

//-------------------------------------------------------------
void ofApp::inputSetup()
{
  auto devices = cam.listDevices();
  for (auto &dev : devices)
  {
    ofLogNotice() << "Found device: " << dev.deviceName;
  }
  // pass in the settings and it will start the camera
  cam.setDeviceID(camID);
  cam.setDesiredFrameRate(30);
  cam.setup(width, height);
  if (!cam.isInitialized())
  {
    ofLogError() << "Camera failed to initialize!";
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
  if (key == ')')
  {
    fft.setNormalize(true);
  }

  // error-if i decrement fb0_delayamount it always crashes...
  if (key == '[')
  {
    fb0_delayamount += 1;
  }
  if (key == ']')
  {
    fb0_delayamount = fb0_delayamount - 1;
    if (fb0_delayamount < 0)
    {
      fb0_delayamount = framebufferLength - fb0_delayamount;
    } // endiffb0
  } // endifkey

  // fb1 mix
  if (key == 'o')
  {
    op += .01;
  }
  if (key == 'p')
  {
    op -= .01;
  }

  // fb0 z displace
  if (key == 'a')
  {
    az += .0001;
  }
  if (key == 'z')
  {
    az -= .0001;
  }

  // fb0 x displace
  if (key == 's')
  {
    sx += .0001;
  }
  if (key == 'x')
  {
    sx -= .0001;
  }

  // fb0 y displace
  if (key == 'd')
  {
    dc += .0001;
  }
  if (key == 'c')
  {
    dc -= .0001;
  }

  // fb0 hue attenuate
  if (key == 'f')
  {
    fv += .001;
  }
  if (key == 'v')
  {
    fv -= .001;
  }

  // fb0 saturation attenuate
  if (key == 'g')
  {
    gb += .001;
  }
  if (key == 'b')
  {
    gb -= .001;
  }

  // fb0 brightness attenuate
  if (key == 'h')
  {
    hn += .001;
  }
  if (key == 'n')
  {
    hn -= .001;
  }

  // fb0 mix
  if (key == 'j')
  {
    jm += .01;
  }
  if (key == 'm')
  {
    jm -= .01;
  }

  // fb0 lumakey value
  if (key == 'k')
  {
    kk = ofClamp(kk + .01, 0.0, 1.0);
  }
  if (key == ',')
  {
    kk = ofClamp(kk - .01, 0.0, 1.0);
  }

  if (key == 'l')
  {
    ll += .01;
  }
  if (key == '.')
  {
    ll -= .01;
  }

  if (key == ';')
  {
    fb1_brightx += .01;
  }
  if (key == '\'')
  {
    fb1_brightx -= .01;
  }

  // fb0 rotation
  if (key == 'q')
  {
    qw += .0001;
  }
  if (key == 'w')
  {
    qw -= .0001;
  }

  // hue chaos1
  if (key == 'e')
  {
    er += .001;
  }
  if (key == 'r')
  {
    er -= .001;
  }

  // hue chaos2
  if (key == 't')
  {
    ty += .01;
  }
  if (key == 'y')
  {
    ty -= .01;
  }

  // hue chaos3
  if (key == 'u')
  {
    ui += .01;
  }
  if (key == 'i')
  {
    ui -= .01;
  }

  if (key == '1')
  {
    // clear the framebuffer if thats whats up
    framebuffer0.begin();
    ofClear(0, 0, 0, 255);
    framebuffer0.end();
    for (int i = 0; i < framebufferLength; i++)
    {
      pastFrames[i].begin();
      ofClear(0, 0, 0, 255);
      pastFrames[i].end();

    } // endifor
  }

  if (key == '2')
  {
    brightInvert = !brightInvert;
  }
  if (key == '3')
  {
    hueInvert = !hueInvert;
  }
  if (key == '4')
  {
    saturationInvert = !saturationInvert;
  }

  if (key == '5')
  {
    verticalMirror = !verticalMirror;
  }
  if (key == '6')
  {
    horizontalMirror = !horizontalMirror;
  }

  if (key == '7')
  {
    toroidSwitch = !toroidSwitch;
  }

  if (key == '-')
  {
    y_skew += .01;
  }
  if (key == '=')
  {
    y_skew -= .01;
  }
  if (key == '9')
  {
    x_skew += .01;
  }
  if (key == '0')
  {
    x_skew -= .01;
  }

  if (key == ' ')
  {
    // increment the cam id
    vector<string> devices;
    for (int i = 0; i < cam.listDevices().size(); i++)
    {
      devices.push_back(cam.listDevices()[i].deviceName);
    }
    if (devices.empty())
    {
      cout << "no camera devices found" << endl;
      return;
    }
    else
    {
      for (unsigned long int i = 0; i < devices.size(); i++)
      {
        cout << devices[i] << endl;
      }
      camID = (camID + 1) % devices.size(); // wrap camID correctly
      ofLog() << "camID: " << camID << " device name: " << devices[camID];
      cam.close();
      cam.setDeviceID(camID);
      cam.setup(width, height);
    }
  }

  if (key == '8')
  {
    vector<string> devices;
    devices = midiIn.getInPortList();
    if (devices.empty())
    {
      cout << "no midi devices found" << endl;
      return;
    }
    else
    {
      for (unsigned long int i = 0; i < devices.size(); i++)
      {
        cout << devices[i] << endl;
      }
      if (midiID < devices.size())
      {
        midiID++;
      }
      else
      {
        midiID = 0;
      }
    }
  }

  if (key == '@')
  {
    if (audioReactiveControlSwitch < 3)
    {
      audioReactiveControlSwitch++;
    }
    else
    {
      audioReactiveControlSwitch = 0;
    }
    ofLog() << "audioReactiveControlSwitch: "
             << audioReactiveControlSwitch;
  }

  // reset button
  if (key == '!')
  {
    az = 1.0;
    sx = 0;
    dc = 0;
    fv = 1;
    gb = 1;
    hn = 1;
    jm = 0.0;
    kk = 0.0;
    ll = 0.0;
    qw = 0.0;

    er = 1.0;
    ty = 0.0;
    ui = 0.0;

    op = 0.0;
    fb0_delayamount = 0;

    brightInvert = 0;
    hueInvert = 0;
    saturationInvert = 0;

    verticalMirror = 0;
    horizontalMirror = 0;

    x_skew = 0;
    y_skew = 0;

    framebuffer0.begin();
    ofClear(0, 0, 0, 255);
    framebuffer0.end();

    for (int i = 0; i < framebufferLength; i++)
    {
      pastFrames[i].begin();
      ofClear(0, 0, 0, 255);
      pastFrames[i].end();
    } // endifor
  }
}
//------------------------------------------------------------------
void ofApp::midibiz()
{

  // lets figure out the hd switch thing here
  // bool cc_aspect_switch=0;
  // int cc_aspect_int=0;

  for (unsigned int i = 0; i < midiMessages.size(); ++i)
  {

    ofxMidiMessage &message = midiMessages[i];

    if (message.status < MIDI_SYSEX)
    {
      // text << "chan: " << message.channel;
      if (message.status == MIDI_CONTROL_CHANGE)
      {

        // How to Midi Map
        // uncomment the line that says cout<<message control etc
        // run the code and look down in the console
        // when u move a knob on yr controller keep track of the number that
        // shows up that is the cc value of the knob then go down to the part
        // labled 'MIDIMAPZONE' and change the numbers for each if
        // message.control== statement to the values on yr controller

        // cout << "message.control"<< message.control<< endl;
        // cout << "message.value"<< message.value<< endl;

        // MIDIMAPZONE
        // these are mostly all set to output bipolor controls at this moment
        // (ranging from -1.0 to 1.0) if u uncomment the second line on each of
        // these if statements that will switch thems to unipolor controls
        // (ranging from 0.0to 1.0) if  you prefer

        // videoreactive
        if (message.control == 39)
        {
          if (message.value == 127)
          {
            videoReactiveSwitch = 1;
            p_lock_0_switch = 0;
          }
          if (message.value == 0)
          {
            videoReactiveSwitch = 0;
            p_lock_0_switch = 1;
          }
        }
        // audioreactive
        if (message.control == 43)
        {
          if (message.value == 127)
          {
            audioReactiveControlSwitch = 1;
          }
          if (message.value == 0)
          {
            audioReactiveControlSwitch = 0;
          }
        }
        if (message.control == 44)
        {
          if (message.value == 127)
          {
            audioReactiveControlSwitch = 2;
          }
          if (message.value == 0)
          {
            audioReactiveControlSwitch = 0;
          }
        }
        if (message.control == 42)
        {
          if (message.value == 127)
          {
            audioReactiveControlSwitch = 3;
          }
          if (message.value == 0)
          {
            audioReactiveControlSwitch = 0;
          }
        }
        if (message.control == 55)
        {
          if (message.value == 127)
          {
            p_lock_switch = 1;
            for (int i = 0; i < p_lock_number; i++)
            {
              p_lock_smoothed[i] = 0;
              for (int j = 0; j < p_lock_size; j++)
              {
                p_lock[i][j] = p_lock[i][p_lock_increment];
              }
            }
          }
          if (message.value == 0)
          {
            p_lock_switch = 0;
          }
        }
        if (message.control == 32)
        {
          if (message.value == 127)
          {
            x_2 = true;
          }
          if (message.value == 0)
          {
            x_2 = false;
          }
        }
        if (message.control == 48)
        {
          if (message.value == 127)
          {
            x_5 = true;
          }
          if (message.value == 0)
          {
            x_5 = false;
          }
        }
        if (message.control == 64)
        {
          if (message.value == 127)
          {
            x_10 = true;
          }
          if (message.value == 0)
          {
            x_10 = false;
          }
        }
        if (message.control == 33)
        {
          if (message.value == 127)
          {
            y_2 = true;
          }
          if (message.value == 0)
          {
            y_2 = false;
          }
        }
        if (message.control == 49)
        {
          if (message.value == 127)
          {
            y_5 = true;
          }
          if (message.value == 0)
          {
            y_5 = false;
          }
        }
        if (message.control == 65)
        {
          if (message.value == 127)
          {
            y_10 = true;
          }
          if (message.value == 0)
          {
            y_10 = false;
          }
        }
        if (message.control == 34)
        {
          if (message.value == 127)
          {
            z_2 = true;
          }
          if (message.value == 0)
          {
            z_2 = false;
          }
        }
        if (message.control == 50)
        {
          if (message.value == 127)
          {
            z_5 = true;
          }
          if (message.value == 0)
          {
            z_5 = false;
          }
        }
        if (message.control == 66)
        {
          if (message.value == 127)
          {
            z_10 = true;
          }
          if (message.value == 0)
          {
            z_10 = false;
          }
        }
        if (message.control == 35)
        {
          if (message.value == 127)
          {
            r_2 = true;
          }
          if (message.value == 0)
          {
            r_2 = false;
          }
        }
        if (message.control == 51)
        {
          if (message.value == 127)
          {
            r_5 = true;
          }
          if (message.value == 0)
          {
            r_5 = false;
          }
        }
        if (message.control == 67)
        {
          if (message.value == 127)
          {
            r_10 = true;
          }
          if (message.value == 0)
          {
            r_10 = false;
          }
        }
        if (message.control == 36)
        {
          if (message.value == 127)
          {
            huexx_0 = true;
          }
          if (message.value == 0)
          {
            huexx_0 = false;
          }
        }
        if (message.control == 52)
        {
          if (message.value == 127)
          {
            huexx_1 = true;
          }
          if (message.value == 0)
          {
            huexx_1 = false;
          }
        }
        if (message.control == 68)
        {
          if (message.value == 127)
          {
            huexx_2 = true;
          }
          if (message.value == 0)
          {
            huexx_2 = false;
          }
        }
        if (message.control == 46)
        {
          if (message.value == 127)
          {
            toroidSwitch = true;
          }
          if (message.value == 0)
          {
            toroidSwitch = false;
          }
        }
        if (message.control == 61)
        {
          if (message.value == 127)
          {
            mirrorSwitch = true;
          }
          if (message.value == 0)
          {
            mirrorSwitch = false;
          }
        }
        if (y_skew_switch == true)
        {
          y_skew += .00001;
        }
        if (x_skew_switch == true)
        {
          x_skew += .00001;
        }
        if (message.control == 71)
        {
          if (message.value == 127)
          {
            wet_dry_switch = false;
          }
          if (message.value == 0)
          {
            wet_dry_switch = true;
          }
        }
        if (message.control == 37)
        {
          if (message.value == 127)
          {
            huexy_0 = true;
          }
          if (message.value == 0)
          {
            huexy_0 = false;
          }
        }
        if (message.control == 53)
        {
          if (message.value == 127)
          {
            huexy_1 = true;
          }
          if (message.value == 0)
          {
            huexy_1 = false;
          }
        }
        if (message.control == 69)
        {
          if (message.value == 127)
          {
            huexy_2 = true;
          }
          if (message.value == 0)
          {
            huexy_2 = false;
          }
        }
        if (message.control == 38)
        {
          if (message.value == 127)
          {
            huexz_0 = true;
          }
          if (message.value == 0)
          {
            huexz_0 = false;
          }
        }
        if (message.control == 54)
        {
          if (message.value == 127)
          {
            huexz_1 = true;
          }
          if (message.value == 0)
          {
            huexz_1 = false;
          }
        }
        if (message.control == 70)
        {
          if (message.value == 127)
          {
            huexz_2 = true;
          }
          if (message.value == 0)
          {
            huexz_2 = false;
          }
        }
        if (message.control == 60)
        {
          if (message.value == 127)
          {
            lumakeyInvertSwitch = true;
          }
          if (message.value == 0)
          {
            lumakeyInvertSwitch = false;
          }
        }
        // CONTINUOUS CONTROLS
        if (message.control == 16)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[0] = 0;
            midiMidActiveFloat[0] = 0;
            midiHighActiveFloat[0] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[0] = 0;
              if (abs(message.value / 127.0f - p_lock[0][p_lock_increment]) <
                  CONTROL_THRESHOLD)
              {
                midiActiveFloat[0] = 1;
              }
              if (midiActiveFloat[0] == 1)
              {
                p_lock[0][p_lock_increment] = message.value / 127.0f;
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[0] = 0;
              if (abs(message.value / 127.0f - vLumakeyValue) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[0] = 1;
              }
              if (vmidiActiveFloat[0] == 1)
              {
                vLumakeyValue = message.value / 127.0f;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[0] = 0;
            vmidiActiveFloat[0] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[0] = 0;
            midiHighActiveFloat[0] = 0;
            if (abs(message.value / 127.0f - lowC1) < CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[0] = 1;
            }
            if (midiLowActiveFloat[0] == 1)
            {
              lowC1 = message.value / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[0] = 0;
            midiHighActiveFloat[0] = 0;
            if (abs(message.value / 127.0f - midC1) < CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[0] = 1;
            }
            if (midiMidActiveFloat[0] == 1)
            {
              midC1 = message.value / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[0] = 0;
            midiMidActiveFloat[0] = 0;
            if (abs(message.value / 127.0f - highC1) < CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[0] = 1;
            }
            if (midiHighActiveFloat[0] == 1)
            {
              highC1 = message.value / 127.0f;
            }
          }
        }
        if (message.control == 17)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[1] = 0;
            midiMidActiveFloat[1] = 0;
            midiHighActiveFloat[1] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[1] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[1][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[1] = 1;
              }
              if (midiActiveFloat[1] == 1)
              {
                p_lock[1][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[1] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vMix) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[1] = 1;
              }
              if (vmidiActiveFloat[1] == 1)
              {
                vMix = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[1] = 0;
            vmidiActiveFloat[1] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[1] = 0;
            midiHighActiveFloat[1] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC2) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[1] = 1;
            }
            if (midiLowActiveFloat[1] == 1)
            {
              lowC2 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[1] = 0;
            midiHighActiveFloat[1] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC2) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[1] = 1;
            }
            if (midiMidActiveFloat[1] == 1)
            {
              midC2 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[1] = 0;
            midiMidActiveFloat[1] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC2) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[1] = 1;
            }
            if (midiHighActiveFloat[1] == 1)
            {
              highC2 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
        }
        // c3 maps to fb hue
        if (message.control == 18)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[2] = 0;
            midiMidActiveFloat[2] = 0;
            midiHighActiveFloat[2] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[2] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[2][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[2] = 1;
              }
              if (midiActiveFloat[2] == 1)
              {
                p_lock[2][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[2] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vHue) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[2] = 1;
              }
              if (vmidiActiveFloat[2] == 1)
              {
                vHue = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[2] = 0;
            vmidiActiveFloat[2] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[2] = 0;
            midiHighActiveFloat[2] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC3) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[2] = 1;
            }
            if (midiLowActiveFloat[2] == 1)
            {
              lowC3 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[2] = 0;
            midiHighActiveFloat[2] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC3) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[2] = 1;
            }
            if (midiMidActiveFloat[2] == 1)
            {
              midC3 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[2] = 0;
            midiMidActiveFloat[2] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC3) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[2] = 1;
            }
            if (midiHighActiveFloat[2] == 1)
            {
              highC3 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
        }
        // c4 maps to fb satx
        if (message.control == 19)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[3] = 0;
            midiMidActiveFloat[3] = 0;
            midiHighActiveFloat[3] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[3] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[3][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[3] = 1;
              }
              if (midiActiveFloat[3] == 1)
              {
                p_lock[3][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[3] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vSaturation) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[3] = 1;
              }
              if (vmidiActiveFloat[3] == 1)
              {
                vSaturation = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[3] = 0;
            vmidiActiveFloat[3] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[3] = 0;
            midiHighActiveFloat[3] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC4) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[3] = 1;
            }
            if (midiLowActiveFloat[3] == 1)
            {
              lowC4 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[3] = 0;
            midiHighActiveFloat[3] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC4) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[3] = 1;
            }
            if (midiMidActiveFloat[3] == 1)
            {
              midC4 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[3] = 0;
            midiMidActiveFloat[3] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC4) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[3] = 1;
            }
            if (midiHighActiveFloat[3] == 1)
            {
              highC4 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
        }
        // c5 maps to fb brightx
        if (message.control == 20)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[4] = 0;
            midiMidActiveFloat[4] = 0;
            midiHighActiveFloat[4] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[4] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[4][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[4] = 1;
              }
              if (midiActiveFloat[4] == 1)
              {
                p_lock[4][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[4] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vBright) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[4] = 1;
              }
              if (vmidiActiveFloat[4] == 1)
              {
                vBright = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[4] = 0;
            vmidiActiveFloat[4] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[4] = 0;
            midiHighActiveFloat[4] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC5) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[4] = 1;
            }
            if (midiLowActiveFloat[4] == 1)
            {
              lowC5 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[4] = 0;
            midiHighActiveFloat[4] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC5) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[4] = 1;
            }
            if (midiMidActiveFloat[4] == 1)
            {
              midC5 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[4] = 0;
            midiMidActiveFloat[4] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC5) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[4] = 1;
            }
            if (midiHighActiveFloat[4] == 1)
            {
              highC5 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
        }
        // c6 maps to temporal filter
        if (message.control == 21)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[5] = 0;
            midiMidActiveFloat[5] = 0;
            midiHighActiveFloat[5] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[5] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[5][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[5] = 1;
              }
              if (midiActiveFloat[5] == 1)
              {
                p_lock[5][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[5] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      vTemporalFilterMix) < CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[5] = 1;
              }
              if (vmidiActiveFloat[5] == 1)
              {
                vTemporalFilterMix = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[5] = 0;
            vmidiActiveFloat[5] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[5] = 0;
            midiHighActiveFloat[5] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC6) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[5] = 1;
            }
            if (midiLowActiveFloat[5] == 1)
            {
              lowC6 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[5] = 0;
            midiHighActiveFloat[5] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC6) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[5] = 1;
            }
            if (midiMidActiveFloat[5] == 1)
            {
              midC6 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[5] = 0;
            midiMidActiveFloat[5] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC6) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[5] = 1;
            }
            if (midiHighActiveFloat[5] == 1)
            {
              highC6 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
            }
          }
        }
        // c7 maps to temporal filter resonance
        if (message.control == 22)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[6] = 0;
            midiMidActiveFloat[6] = 0;
            midiHighActiveFloat[6] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[6] = 0;
              if (abs(message.value / 127.0f - p_lock[6][p_lock_increment]) <
                  CONTROL_THRESHOLD)
              {
                midiActiveFloat[6] = 1;
              }
              if (midiActiveFloat[6] == 1)
              {
                p_lock[6][p_lock_increment] = message.value / 127.0f;
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[6] = 0;
              if (abs(message.value / 127.0f - vTemporalFilterResonance) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[6] = 1;
              }
              if (vmidiActiveFloat[6] == 1)
              {
                vTemporalFilterResonance = message.value / 127.0f;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[6] = 0;
            vmidiActiveFloat[6] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[6] = 0;
            midiHighActiveFloat[6] = 0;
            if (abs(message.value / 127.0f - lowC7) < CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[6] = 1;
            }
            if (midiLowActiveFloat[6] == 1)
            {
              lowC7 = message.value / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[6] = 0;
            midiHighActiveFloat[6] = 0;
            if (abs(message.value / 127.0f - midC7) < CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[6] = 1;
            }
            if (midiMidActiveFloat[6] == 1)
            {
              midC7 = message.value / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[6] = 0;
            midiMidActiveFloat[6] = 0;
            if (abs(message.value / 127.0f - highC7) < CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[6] = 1;
            }
            if (midiHighActiveFloat[6] == 1)
            {
              highC7 = message.value / 127.0f;
            }
          }
        }
        // c8 maps to sharpen
        if (message.control == 23)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[7] = 0;
            midiMidActiveFloat[7] = 0;
            midiHighActiveFloat[7] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[7] = 0;
              if (abs(message.value / 127.0f - p_lock[7][p_lock_increment]) <
                  CONTROL_THRESHOLD)
              {
                midiActiveFloat[7] = 1;
              }
              if (midiActiveFloat[7] == 1)
              {
                p_lock[7][p_lock_increment] = message.value / 127.0f;
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[7] = 0;
              if (abs(message.value / 127.0f - vSharpenAmount) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[7] = 1;
              }
              if (vmidiActiveFloat[7] == 1)
              {
                vSharpenAmount = message.value / 127.0f;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[7] = 0;
            vmidiActiveFloat[7] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[7] = 0;
            midiHighActiveFloat[7] = 0;
            if (abs(message.value / 127.0f - lowC8) < CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[7] = 1;
            }
            if (midiLowActiveFloat[7] == 1)
            {
              lowC8 = message.value / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[7] = 0;
            midiHighActiveFloat[7] = 0;
            if (abs(message.value / 127.0f - midC8) < CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[7] = 1;
            }
            if (midiMidActiveFloat[7] == 1)
            {
              midC8 = message.value / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[7] = 0;
            midiMidActiveFloat[7] = 0;
            if (abs(message.value / 127.0f - highC8) < CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[7] = 1;
            }
            if (midiHighActiveFloat[7] == 1)
            {
              highC8 = message.value / 127.0f;
            }
          }
        }
        // c9 maps to fb x displace
        if (message.control == 120)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[8] = 0;
            midiMidActiveFloat[8] = 0;
            midiHighActiveFloat[8] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[8] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[8][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[8] = 1;
              }
              if (midiActiveFloat[8] == 1)
              {
                p_lock[8][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (x_2 == true)
                {
                  p_lock[8][p_lock_increment] =
                      2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (x_5 == true)
                {
                  p_lock[8][p_lock_increment] =
                      5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (x_10 == true)
                {
                  p_lock[8][p_lock_increment] =
                      10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[8] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vX) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[8] = 1;
              }
              if (vmidiActiveFloat[8] == 1)
              {
                vX = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (x_2 == true)
                {
                  vX = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (x_5 == true)
                {
                  vX = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (x_10 == true)
                {
                  vX = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[8] = 0;
            vmidiActiveFloat[8] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[8] = 0;
            midiHighActiveFloat[8] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC9) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[8] = 1;
            }
            if (midiLowActiveFloat[8] == 1)
            {
              lowC9 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (x_2 == true)
              {
                lowC9 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (x_5 == true)
              {
                lowC9 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (x_10 == true)
              {
                lowC9 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[8] = 0;
            midiHighActiveFloat[8] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC9) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[8] = 1;
            }
            if (midiMidActiveFloat[8] == 1)
            {
              midC9 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (x_2 == true)
              {
                midC9 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (x_5 == true)
              {
                midC9 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (x_10 == true)
              {
                midC9 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[8] = 0;
            midiMidActiveFloat[8] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC9) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[8] = 1;
            }
            if (midiHighActiveFloat[8] == 1)
            {
              highC9 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (x_2 == true)
              {
                highC9 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (x_5 == true)
              {
                highC9 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (x_10 == true)
              {
                highC9 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
        }
        if (message.control == 121)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[9] = 0;
            midiMidActiveFloat[9] = 0;
            midiHighActiveFloat[9] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[9] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[9][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[9] = 1;
              }
              if (midiActiveFloat[9] == 1)
              {
                p_lock[9][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (y_2 == true)
                {
                  p_lock[9][p_lock_increment] =
                      2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (y_5 == true)
                {
                  p_lock[9][p_lock_increment] =
                      5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (y_10 == true)
                {
                  p_lock[9][p_lock_increment] =
                      10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[9] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vY) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[9] = 1;
              }
              if (vmidiActiveFloat[9] == 1)
              {
                vY = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (y_2 == true)
                {
                  vY = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (y_5 == true)
                {
                  vY = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (y_10 == true)
                {
                  vY = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[9] = 0;
            vmidiActiveFloat[9] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[9] = 0;
            midiHighActiveFloat[9] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC10) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[9] = 1;
            }
            if (midiLowActiveFloat[9] == 1)
            {
              lowC10 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (y_2 == true)
              {
                lowC10 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (y_5 == true)
              {
                lowC10 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (y_10 == true)
              {
                lowC10 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[9] = 0;
            midiHighActiveFloat[9] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC10) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[9] = 1;
            }
            if (midiMidActiveFloat[9] == 1)
            {
              midC10 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (y_2 == true)
              {
                midC10 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (y_5 == true)
              {
                midC10 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (y_10 == true)
              {
                midC10 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[9] = 0;
            midiMidActiveFloat[9] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC10) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[9] = 1;
            }
            if (midiHighActiveFloat[9] == 1)
            {
              highC10 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (y_2 == true)
              {
                highC10 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (y_5 == true)
              {
                highC10 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (y_10 == true)
              {
                highC10 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
        }
        // z displace
        if (message.control == 122)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[10] = 0;
            midiMidActiveFloat[10] = 0;
            midiHighActiveFloat[10] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[10] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[10][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[10] = 1;
              }
              if (midiActiveFloat[10] == 1)
              {
                p_lock[10][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (z_2 == true)
                {
                  p_lock[10][p_lock_increment] =
                      2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (z_5 == true)
                {
                  p_lock[10][p_lock_increment] =
                      5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (z_10 == true)
                {
                  p_lock[10][p_lock_increment] =
                      10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[10] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vZ) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[10] = 1;
              }
              if (vmidiActiveFloat[10] == 1)
              {
                vZ = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (z_2 == true)
                {
                  vZ = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (z_5 == true)
                {
                  vZ = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (z_10 == true)
                {
                  vZ = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[10] = 0;
            vmidiActiveFloat[10] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[10] = 0;
            midiHighActiveFloat[10] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC11) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[10] = 1;
            }
            if (midiLowActiveFloat[10] == 1)
            {
              lowC11 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (z_2 == true)
              {
                lowC11 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (z_5 == true)
              {
                lowC11 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (z_10 == true)
              {
                lowC11 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[10] = 0;
            midiHighActiveFloat[10] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC11) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[10] = 1;
            }
            if (midiMidActiveFloat[10] == 1)
            {
              midC11 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (z_2 == true)
              {
                midC11 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (z_5 == true)
              {
                midC11 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (z_10 == true)
              {
                midC11 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[10] = 0;
            midiMidActiveFloat[10] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC11) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[10] = 1;
            }
            if (midiHighActiveFloat[10] == 1)
            {
              highC11 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (z_2 == true)
              {
                highC11 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (z_5 == true)
              {
                highC11 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (z_10 == true)
              {
                highC11 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
        }
        // rotations
        if (message.control == 123)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[11] = 0;
            midiMidActiveFloat[11] = 0;
            midiHighActiveFloat[11] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[11] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[11][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[11] = 1;
              }
              if (midiActiveFloat[11] == 1)
              {
                p_lock[11][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;

                if (r_2 == true)
                {
                  p_lock[11][p_lock_increment] =
                      2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (r_5 == true)
                {
                  p_lock[11][p_lock_increment] =
                      5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (r_10 == true)
                {
                  p_lock[11][p_lock_increment] =
                      10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[11] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vRotate) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[11] = 1;
              }
              if (vmidiActiveFloat[11] == 1)
              {
                vRotate = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (r_2 == true)
                {
                  vRotate = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (r_5 == true)
                {
                  vRotate = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (r_10 == true)
                {
                  vRotate = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[11] = 0;
            vmidiActiveFloat[11] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[11] = 0;
            midiHighActiveFloat[11] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC12) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[11] = 1;
            }
            if (midiLowActiveFloat[11] == 1)
            {
              lowC12 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (r_2 == true)
              {
                lowC12 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (r_5 == true)
              {
                lowC12 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (r_10 == true)
              {
                lowC12 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[11] = 0;
            midiHighActiveFloat[11] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC12) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[11] = 1;
            }
            if (midiMidActiveFloat[11] == 1)
            {
              midC12 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (r_2 == true)
              {
                midC12 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (r_5 == true)
              {
                midC12 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (r_10 == true)
              {
                midC12 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[11] = 0;
            midiMidActiveFloat[11] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC12) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[11] = 1;
            }
            if (midiHighActiveFloat[11] == 1)
            {
              highC12 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (r_2 == true)
              {
                highC12 = 2.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (r_5 == true)
              {
                highC12 = 5.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (r_10 == true)
              {
                highC12 = 10.0 * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
        }
        if (message.control == 124)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[12] = 0;
            midiMidActiveFloat[12] = 0;
            midiHighActiveFloat[12] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[12] = 0;
              if (abs(message.value / 127.0f - p_lock[12][p_lock_increment]) <
                  CONTROL_THRESHOLD)
              {
                midiActiveFloat[12] = 1;
              }
              if (midiActiveFloat[12] == 1)
              {
                p_lock[12][p_lock_increment] = (message.value) / 32.0f;
                if (huexx_0 == true)
                {
                  p_lock[12][p_lock_increment] = message.value / 64.0f;
                }
                if (huexx_1 == true)
                {
                  p_lock[12][p_lock_increment] = message.value / 96.0f;
                }
                if (huexx_2 == true)
                {
                  p_lock[12][p_lock_increment] = message.value / 127.0f;
                }
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[12] = 0;
              if (abs(message.value / 127.0f - vHuexMod) < CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[12] = 1;
              }
              if (vmidiActiveFloat[12] == 1)
              {
                vHuexMod = (message.value) / 32.0f;
                if (huexx_0 == true)
                {
                  vHuexMod = message.value / 64.0f;
                }
                if (huexx_1 == true)
                {
                  vHuexMod = message.value / 96.0f;
                }
                if (huexx_2 == true)
                {
                  vHuexMod = message.value / 127.0f;
                }
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[12] = 0;
            vmidiActiveFloat[12] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[12] = 0;
            midiHighActiveFloat[12] = 0;
            if (abs(message.value / 127.0f - lowC13) < CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[12] = 1;
            }
            if (midiLowActiveFloat[12] == 1)
            {
              lowC13 = (message.value) / 32.0f;
              if (huexx_0 == true)
              {
                lowC13 = message.value / 64.0f;
              }
              if (huexx_1 == true)
              {
                lowC13 = message.value / 96.0f;
              }
              if (huexx_2 == true)
              {
                lowC13 = message.value / 127.0f;
              }
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[12] = 0;
            midiHighActiveFloat[12] = 0;
            if (abs(message.value / 127.0f - midC13) < CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[12] = 1;
            }
            if (midiMidActiveFloat[12] == 1)
            {
              midC13 = (message.value) / 32.0f;
              if (huexx_0 == true)
              {
                midC13 = message.value / 64.0f;
              }
              if (huexx_1 == true)
              {
                midC13 = message.value / 96.0f;
              }
              if (huexx_2 == true)
              {
                midC13 = message.value / 127.0f;
              }
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[12] = 0;
            midiMidActiveFloat[12] = 0;
            if (abs(message.value / 127.0f - highC13) < CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[12] = 1;
            }
            if (midiHighActiveFloat[12] == 1)
            {
              highC13 = (message.value) / 32.0f;
              if (huexx_0 == true)
              {
                highC13 = message.value / 64.0f;
              }
              if (huexx_1 == true)
              {
                highC13 = message.value / 96.0f;
              }
              if (huexx_2 == true)
              {
                highC13 = message.value / 127.0f;
              }
            }
          }
        }
        if (message.control == 125)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[13] = 0;
            midiMidActiveFloat[13] = 0;
            midiHighActiveFloat[13] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[13] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[13][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[13] = 1;
              }
              if (midiActiveFloat[13] == 1)
              {
                p_lock[13][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (huexy_0 == true)
                {
                  p_lock[13][p_lock_increment] =
                      2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexy_1 == true)
                {
                  p_lock[13][p_lock_increment] =
                      4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexy_2 == true)
                {
                  p_lock[13][p_lock_increment] =
                      8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[13] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vHuexOff) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[13] = 1;
              }
              if (vmidiActiveFloat[13] == 1)
              {
                vHuexOff = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (huexy_0 == true)
                {
                  vHuexOff = 2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexy_1 == true)
                {
                  vHuexOff = 4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexy_2 == true)
                {
                  vHuexOff = 8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[13] = 0;
            vmidiActiveFloat[13] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[13] = 0;
            midiHighActiveFloat[13] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC14) <
                CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[13] = 1;
            }
            if (midiLowActiveFloat[13] == 1)
            {
              lowC14 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (huexy_0 == true)
              {
                lowC14 = 2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (huexy_1 == true)
              {
                lowC14 = 4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (huexy_2 == true)
              {
                lowC14 = 8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[13] = 0;
            midiHighActiveFloat[13] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC14) <
                CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[13] = 1;
            }
            if (midiMidActiveFloat[13] == 1)
            {
              midC14 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (huexy_0 == true)
              {
                midC14 = 2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (huexy_1 == true)
              {
                midC14 = 4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (huexy_2 == true)
              {
                midC14 = 8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[13] = 0;
            midiMidActiveFloat[13] = 0;
            if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC14) <
                CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[13] = 1;
            }
            if (midiHighActiveFloat[13] == 1)
            {
              highC14 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              if (huexy_0 == true)
              {
                highC14 = 2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (huexy_1 == true)
              {
                highC14 = 4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
              if (huexy_2 == true)
              {
                highC14 = 8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
              }
            }
          }
        }
        if (message.control == 126)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[14] = 0;
            midiMidActiveFloat[14] = 0;
            midiHighActiveFloat[14] = 0;
            if (p_lock_0_switch == 1)
            {
              vmidiActiveFloat[14] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC -
                      p_lock[14][p_lock_increment]) < CONTROL_THRESHOLD)
              {
                midiActiveFloat[14] = 1;
              }
              if (midiActiveFloat[14] == 1)
              {
                p_lock[14][p_lock_increment] =
                    (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (huexz_0 == true)
                {
                  p_lock[14][p_lock_increment] =
                      2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_1 == true)
                {
                  p_lock[14][p_lock_increment] =
                      4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_2 == true)
                {
                  p_lock[14][p_lock_increment] =
                      8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (videoReactiveSwitch == 1)
            {
              midiActiveFloat[14] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - vHuexLfo) <
                  CONTROL_THRESHOLD)
              {
                vmidiActiveFloat[14] = 1;
              }
              if (vmidiActiveFloat[14] == 1)
              {
                vHuexLfo = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (huexz_0 == true)
                {
                  vHuexLfo = 2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_1 == true)
                {
                  vHuexLfo = 4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_2 == true)
                {
                  vHuexLfo = 8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (audioReactiveControlSwitch != 0)
            {
              midiActiveFloat[14] = 0;
              vmidiActiveFloat[14] = 0;
            }
            if (audioReactiveControlSwitch == 1)
            {
              midiMidActiveFloat[14] = 0;
              midiHighActiveFloat[14] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - lowC15) <
                  CONTROL_THRESHOLD)
              {
                midiLowActiveFloat[14] = 1;
              }
              if (midiLowActiveFloat[14] == 1)
              {
                lowC15 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (huexz_0 == true)
                {
                  lowC15 = 2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_1 == true)
                {
                  lowC15 = 4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_2 == true)
                {
                  lowC15 = 8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (audioReactiveControlSwitch == 2)
            {
              midiLowActiveFloat[14] = 0;
              midiHighActiveFloat[14] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - midC15) <
                  CONTROL_THRESHOLD)
              {
                midiMidActiveFloat[14] = 1;
              }
              if (midiMidActiveFloat[14] == 1)
              {
                midC15 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (huexz_0 == true)
                {
                  midC15 = 2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_1 == true)
                {
                  midC15 = 4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_2 == true)
                {
                  midC15 = 8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
            if (audioReactiveControlSwitch == 3)
            {
              midiLowActiveFloat[14] = 0;
              midiMidActiveFloat[14] = 0;
              if (abs((message.value - MIDI_MAGIC) / MIDI_MAGIC - highC15) <
                  CONTROL_THRESHOLD)
              {
                midiHighActiveFloat[14] = 1;
              }
              if (midiHighActiveFloat[14] == 1)
              {
                highC15 = (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                if (huexz_0 == true)
                {
                  highC15 = 2.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_1 == true)
                {
                  highC15 = 4.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
                if (huexz_2 == true)
                {
                  highC15 = 8.0f * (message.value - MIDI_MAGIC) / MIDI_MAGIC;
                }
              }
            }
          }
        }
        if (message.control == 127)
        {
          if (audioReactiveControlSwitch == 0)
          {
            midiLowActiveFloat[15] = 0;
            midiMidActiveFloat[15] = 0;
            midiHighActiveFloat[15] = 0;
            if (p_lock_0_switch == 1)
            {
              if (abs(message.value / 127.0f - p_lock[15][p_lock_increment]) <
                  CONTROL_THRESHOLD)
              {
                midiActiveFloat[15] = 1;
              }
              if (midiActiveFloat[15] == 1)
              {
                p_lock[15][p_lock_increment] = (message.value) / 127.0;
              }
            }
          }
          if (audioReactiveControlSwitch != 0)
          {
            midiActiveFloat[15] = 0;
            vmidiActiveFloat[15] = 0;
          }
          if (audioReactiveControlSwitch == 1)
          {
            midiMidActiveFloat[15] = 0;
            midiHighActiveFloat[15] = 0;
            if (abs(message.value / 127.0f - lowC16) < CONTROL_THRESHOLD)
            {
              midiLowActiveFloat[15] = 1;
            }
            if (midiLowActiveFloat[15] == 1)
            {
              lowC16 = message.value / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 2)
          {
            midiLowActiveFloat[15] = 0;
            midiHighActiveFloat[15] = 0;
            if (abs(message.value / 127.0f - midC16) < CONTROL_THRESHOLD)
            {
              midiMidActiveFloat[15] = 1;
            }
            if (midiMidActiveFloat[15] == 1)
            {
              midC16 = message.value / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 3)
          {
            midiLowActiveFloat[15] = 0;
            midiMidActiveFloat[15] = 0;
            if (abs(message.value / 127.0f - highC16) < CONTROL_THRESHOLD)
            {
              midiHighActiveFloat[15] = 1;
            }
            if (midiHighActiveFloat[15] == 1)
            {
              highC16 = message.value / 127.0f;
            }
          }
        }
        if (message.control == 60)
        {
          if (message.value == 127)
          {
            brightInvert = true;
          }
          if (message.value == 0)
          {
            brightInvert = false;
          }
        }
        if (message.control == 61)
        {
          if (message.value == 127)
          {
            saturationInvert = true;
          }
          if (message.value == 0)
          {
            saturationInvert = false;
          }
        }
        if (message.control == 62)
        {
          if (message.value == 127)
          {
            hueInvert = true;
          }
          if (message.value == 0)
          {
            hueInvert = false;
          }
        }
        if (message.control == 62)
        {
          if (message.value == 127)
          {
            hdmi_aspect_ratio_switch = true;
          }
          if (message.value == 0)
          {
            hdmi_aspect_ratio_switch = false;
          }
        }
        if (message.control == 41)
        {
          if (message.value == 127)
          {
            horizontalMirror = true;
          }
          if (message.value == 0)
          {
            horizontalMirror = false;
          }
        }
        if (message.control == 45)
        {
          if (message.value == 127)
          {
            verticalMirror = true;
          }
          if (message.value == 0)
          {
            verticalMirror = false;
          }
        }
        if (message.control == 59)
        {
          if (message.value == 127)
          {
            vLumakeyValue = 0.0;
            vMix = 0.0;
            vHue = 0.0;
            vSaturation = 0.0;
            vBright = 0.0;
            vTemporalFilterMix = 0.0;
            vTemporalFilterResonance = 0.0;
            vSharpenAmount = 0.0;
            vX = 0.0;
            vY = 0.0;
            vZ = 0.0;
            vRotate = 0.0;
            vHuexMod = 0.0;
            vHuexOff = 0.0;
            vHuexLfo = 0.0;
            lowC1 = lowC2 = lowC3 = lowC4 = lowC5 = lowC6 = lowC7 = lowC8 =
                lowC9 = lowC10 = lowC11 = lowC12 = lowC13 = lowC14 = lowC15 =
                    lowC16 = 0.0f;
            midC1 = midC2 = midC3 = midC4 = midC5 = midC6 = midC7 = midC8 =
                midC9 = midC10 = midC11 = midC12 = midC13 = midC14 = midC15 =
                    midC16 = 0.0f;
            highC1 = highC2 = highC3 = highC4 = highC5 = highC6 = highC7 =
                highC8 = highC9 = highC10 = highC11 = highC12 = highC13 =
                    highC14 = highC15 = highC16 = 0.0f;
            for (int i = 0; i < p_lock_number; i++)
            {
              midiActiveFloat[i] = 0;
              vmidiActiveFloat[i] = 0;
              midiLowActiveFloat[i] = 0;
              midiMidActiveFloat[i] = 0;
              midiHighActiveFloat[i] = 0;
              for (int j = 0; j < p_lock_size; j++)
              {
                p_lock[i][j] = 0;
              }
            }
          }
        }
        if (message.control == 58)
        {
          if (message.value == 127)
          {
            if (clear_switch == 0)
            {
              clear_switch = 1;
              // clear the framebuffer if thats whats up
              framebuffer0.begin();
              ofClear(0, 0, 0, 255);
              framebuffer0.end();
              for (int i = 0; i < framebufferLength; i++)
              {
                pastFrames[i].begin();
                ofClear(0, 0, 0, 255);
                pastFrames[i].end();
              }
            }
          }
        }
        if (message.control != 58)
        {
          clear_switch = 0;
        }
      }
    }
  }
}

//-----------------------------------------------------------
void ofApp::midibizOld()
{
  for (unsigned int i = 0; i < midiMessages.size(); ++i)
  {

    ofxMidiMessage &message = midiMessages[i];

    if (message.status < MIDI_SYSEX)
    {
      // text << "chan: " << message.channel;
      if (message.status == MIDI_CONTROL_CHANGE)
      {

        // How to Midi Map
        // uncomment the line that says cout<<message control etc
        // run the code and look down in the console
        // when u move a knob on yr controller keep track of the number that
        // shows up that is the cc value of the knob then go down to the part
        // labled 'MIDIMAPZONE' and change the numbers for each if
        // message.control== statement to the values on yr controller

        // cout << "message.control"<< message.control<< endl;
        // cout << "message.value"<< message.value<< endl;

        // MIDIMAPZONE
        // these are mostly all set to output bipolor controls at this moment
        // (ranging from -1.0 to 1.0) if u uncomment the second line on each of
        // these if statements that will switch thems to unipolor controls
        // (ranging from 0.0to 1.0) if  you prefer

        if (message.control == 39)
        {
          if (message.value == 127)
          {
            p_lock_0_switch = 0;
          }
          if (message.value == 0)
          {
            p_lock_0_switch = 1;
          }
        }
        if (message.control == 55)
        {
          if (message.value == 127)
          {
            p_lock_switch = 1;
          }
          if (message.value == 0)
          {
            p_lock_switch = 0;
          }
        }
        if (message.control == 58)
        {
          if (message.value == 127)
          {
            for (int i = 0; i < p_lock_number; i++)
            {
              for (int j = 0; j < p_lock_size; j++)
              {
                p_lock[i][j] = 0;
              }
            }
          }
        }
        if (message.control == 32)
        {
          if (message.value == 127)
          {
            x_2 = true;
          }
          if (message.value == 0)
          {
            x_2 = false;
          }
        }
        if (message.control == 48)
        {
          if (message.value == 127)
          {
            x_5 = true;
          }
          if (message.value == 0)
          {
            x_5 = false;
          }
        }
        if (message.control == 64)
        {
          if (message.value == 127)
          {
            x_10 = true;
          }

          if (message.value == 0)
          {
            x_10 = false;
          }
        }

        if (message.control == 33)
        {
          if (message.value == 127)
          {
            y_2 = true;
          }

          if (message.value == 0)
          {
            y_2 = false;
          }
        }

        if (message.control == 49)
        {
          if (message.value == 127)
          {
            y_5 = true;
          }

          if (message.value == 0)
          {
            y_5 = false;
          }
        }

        if (message.control == 65)
        {
          if (message.value == 127)
          {
            y_10 = true;
          }

          if (message.value == 0)
          {
            y_10 = false;
          }
        }

        if (message.control == 34)
        {
          if (message.value == 127)
          {
            z_2 = true;
          }

          if (message.value == 0)
          {
            z_2 = false;
          }
        }

        if (message.control == 50)
        {
          if (message.value == 127)
          {
            z_5 = true;
          }

          if (message.value == 0)
          {
            z_5 = false;
          }
        }

        if (message.control == 66)
        {
          if (message.value == 127)
          {
            z_10 = true;
          }

          if (message.value == 0)
          {
            z_10 = false;
          }
        }

        if (message.control == 35)
        {
          if (message.value == 127)
          {
            r_2 = true;
          }

          if (message.value == 0)
          {
            r_2 = false;
          }
        }

        if (message.control == 51)
        {
          if (message.value == 127)
          {
            r_5 = true;
          }

          if (message.value == 0)
          {
            r_5 = false;
          }
        }

        if (message.control == 67)
        {
          if (message.value == 127)
          {
            r_10 = true;
          }

          if (message.value == 0)
          {
            r_10 = false;
          }
        }

        if (message.control == 36)
        {
          if (message.value == 127)
          {
            huexx_0 = true;
          }

          if (message.value == 0)
          {
            huexx_0 = false;
          }
        }

        if (message.control == 52)
        {
          if (message.value == 127)
          {
            huexx_1 = true;
          }

          if (message.value == 0)
          {
            huexx_1 = false;
          }
        }

        if (message.control == 68)
        {
          if (message.value == 127)
          {
            huexx_2 = true;
          }

          if (message.value == 0)
          {
            huexx_2 = false;
          }
        }

        if (message.control == 46)
        {
          if (message.value == 127)
          {
            toroidSwitch = true;
          }

          if (message.value == 0)
          {
            toroidSwitch = false;
          }
        }

        if (y_skew_switch == true)
        {
          y_skew += .00001;
        }

        if (x_skew_switch == true)
        {
          x_skew += .00001;
        }

        if (message.control == 71)
        {
          if (message.value == 127)
          {
            wet_dry_switch = false;
          }

          if (message.value == 0)
          {
            wet_dry_switch = true;
          }
        }

        //---------------------

        if (message.control == 37)
        {
          if (message.value == 127)
          {
            huexy_0 = true;
          }

          if (message.value == 0)
          {
            huexy_0 = false;
          }
        }

        if (message.control == 53)
        {
          if (message.value == 127)
          {
            huexy_1 = true;
          }

          if (message.value == 0)
          {
            huexy_1 = false;
          }
        }

        if (message.control == 69)
        {
          if (message.value == 127)
          {
            huexy_2 = true;
          }

          if (message.value == 0)
          {
            huexy_2 = false;
          }
        }

        //---------------------

        if (message.control == 38)
        {
          if (message.value == 127)
          {
            huexz_0 = true;
          }

          if (message.value == 0)
          {
            huexz_0 = false;
          }
        }

        if (message.control == 54)
        {
          if (message.value == 127)
          {
            huexz_1 = true;
          }

          if (message.value == 0)
          {
            huexz_1 = false;
          }
        }

        if (message.control == 70)
        {
          if (message.value == 127)
          {
            huexz_2 = true;
          }

          if (message.value == 0)
          {
            huexz_2 = false;
          }
        }
        if (message.control == 43)
        {
          if (message.value == 127)
          {
            audioReactiveControlSwitch = 1;
          }

          if (message.value == 0)
          {
            audioReactiveControlSwitch = 0;
          }
        }

        if (message.control == 44)
        {
          if (message.value == 127)
          {
            audioReactiveControlSwitch = 2;
          }

          if (message.value == 0)
          {
            audioReactiveControlSwitch = 0;
          }
        }

        if (message.control == 42)
        {
          if (message.value == 127)
          {
            audioReactiveControlSwitch = 3;
          }

          if (message.value == 0)
          {
            audioReactiveControlSwitch = 0;
          }
        }
        // nanokontrol2 controls
        // c1 maps to fb0 lumakey
        if (message.control == 16)
        {

          if (audioReactiveControlSwitch == 0)
          {
            if (p_lock_0_switch == 1)
            {
              p_lock[0][p_lock_increment] = message.value / 127.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC1 = message.value / 127.0f;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC1 = message.value / 127.0f;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC1 = message.value / 127.0f;
          }
        }

        if (message.control == 17)
        {

          if (audioReactiveControlSwitch == 0)
          {
            if (p_lock_0_switch == 1)
            {
              p_lock[1][p_lock_increment] = (message.value - 63) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC2 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC2 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC2 = (message.value - 63.0f) / 63.0f;
          }
        }

        // c3 maps to fb0 huex
        if (message.control == 18)
        {
          if (audioReactiveControlSwitch == 0)
          {
            if (p_lock_0_switch == 1)
            {
              p_lock[2][p_lock_increment] = (message.value - 63) / 63.0f;
            }
          }
          if (audioReactiveControlSwitch == 1)
          {
            lowC3 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC3 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC3 = (message.value - 63.0f) / 63.0f;
          }
        }

        // c4 maps to fb0 satx
        if (message.control == 19)
        {
          if (audioReactiveControlSwitch == 0)
          {
            if (p_lock_0_switch == 1)
            {
              p_lock[3][p_lock_increment] = (message.value - 63) / 63.0f;
            }
          }
          if (audioReactiveControlSwitch == 1)
          {
            lowC4 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC4 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC4 = (message.value - 63.0f) / 63.0f;
          }
        }

        // c5 maps to fb0 brightx
        if (message.control == 20)
        {
          if (audioReactiveControlSwitch == 0)
          {
            if (p_lock_0_switch == 1)
            {
              p_lock[4][p_lock_increment] = (message.value - 63) / 63.0f;
            }
          }
          if (audioReactiveControlSwitch == 1)
          {
            lowC5 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC5 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC5 = (message.value - 63.0f) / 63.0f;
          }
        }

        // c6 maps to temporal filter
        if (message.control == 21)
        {
          if (audioReactiveControlSwitch == 0)
          {
            if (p_lock_0_switch == 1)
            {
              p_lock[5][p_lock_increment] = (message.value - 63) / 63.0f;
            }
          }
          if (audioReactiveControlSwitch == 1)
          {
            lowC6 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC6 = (message.value - 63.0f) / 63.0f;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC6 = (message.value - 63.0f) / 63.0f;
          }
        }

        // c7 maps to temporal filter resonance
        if (message.control == 22)
        {
          if (audioReactiveControlSwitch == 0)
          {
            if (p_lock_0_switch == 1)
            {
              p_lock[6][p_lock_increment] = (message.value) / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 1)
          {
            lowC7 = (message.value) / 127.0f;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC7 = (message.value) / 127.0f;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC7 = (message.value) / 127.0f;
          }
        }

        // c8 maps to brightx
        if (message.control == 23)
        {
          if (audioReactiveControlSwitch == 0)
          {
            if (p_lock_0_switch == 1)
            {
              p_lock[7][p_lock_increment] = (message.value) / 127.0f;
            }
          }
          if (audioReactiveControlSwitch == 1)
          {
            lowC8 = (message.value) / 127.0f;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC8 = (message.value) / 127.0f;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC8 = (message.value) / 127.0f;
          }
        }

        // c9 maps to fb0 x displace
        if (message.control == 120)
        {
          if (audioReactiveControlSwitch == 0)
          {
            p_lock[8][p_lock_increment] = (message.value - 63) / 63.0f;

            if (x_2 == true)
            {
              p_lock[8][p_lock_increment] =
                  2.0 * (message.value - 63.0) / 63.0f;
            }

            if (x_5 == true)
            {
              p_lock[8][p_lock_increment] =
                  5.0 * (message.value - 63.0) / 63.0f;
            }
            if (x_10 == true)
            {
              p_lock[8][p_lock_increment] =
                  10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC9 = (message.value - 63) / 63.0f;

            if (x_2 == true)
            {
              lowC9 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (x_5 == true)
            {
              lowC9 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (x_10 == true)
            {
              lowC9 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC9 = (message.value - 63) / 63.0f;

            if (x_2 == true)
            {
              midC9 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (x_5 == true)
            {
              midC9 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (x_10 == true)
            {
              midC9 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC9 = (message.value - 63) / 63.0f;

            if (x_2 == true)
            {
              highC9 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (x_5 == true)
            {
              highC9 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (x_10 == true)
            {
              highC9 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }
        }

        // c10 maps to fb0 y displace
        if (message.control == 121)
        {
          if (audioReactiveControlSwitch == 0)
          {
            p_lock[9][p_lock_increment] = (message.value - 63) / 63.0f;

            if (y_2 == true)
            {
              p_lock[9][p_lock_increment] =
                  2.0 * (message.value - 63.0) / 63.0f;
            }

            if (y_5 == true)
            {
              p_lock[9][p_lock_increment] =
                  5.0 * (message.value - 63.0) / 63.0f;
            }
            if (y_10 == true)
            {
              p_lock[9][p_lock_increment] =
                  10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC10 = (message.value - 63) / 63.0f;

            if (y_2 == true)
            {
              lowC10 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (y_5 == true)
            {
              lowC10 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (y_10 == true)
            {
              lowC10 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC10 = (message.value - 63) / 63.0f;

            if (y_2 == true)
            {
              midC10 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (y_5 == true)
            {
              midC10 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (y_10 == true)
            {
              midC10 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC10 = (message.value - 63) / 63.0f;

            if (y_2 == true)
            {
              highC10 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (y_5 == true)
            {
              highC10 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (y_10 == true)
            {
              highC10 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }
        }

        if (message.control == 122)
        {
          if (audioReactiveControlSwitch == 0)
          {
            p_lock[10][p_lock_increment] = (message.value - 63.0) / 63.0f;

            if (z_2 == true)
            {
              p_lock[10][p_lock_increment] =
                  2.0 * (message.value - 63.0) / 63.0f;
            }

            if (z_5 == true)
            {
              p_lock[10][p_lock_increment] =
                  5.0 * (message.value - 63.0) / 63.0f;
            }
            if (z_10 == true)
            {
              p_lock[10][p_lock_increment] =
                  10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC11 = (message.value - 63.0) / 63.0f;

            if (z_2 == true)
            {
              lowC11 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (z_5 == true)
            {
              lowC11 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (z_10 == true)
            {
              lowC11 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC11 = (message.value - 63.0) / 63.0f;

            if (z_2 == true)
            {
              midC11 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (z_5 == true)
            {
              midC11 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (z_10 == true)
            {
              midC11 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC11 = (message.value - 63.0) / 63.0f;

            if (z_2 == true)
            {
              highC11 = 2.0 * (message.value - 63.0) / 63.0f;
            }

            if (z_5 == true)
            {
              highC11 = 5.0 * (message.value - 63.0) / 63.0f;
            }
            if (z_10 == true)
            {
              highC11 = 10.0 * (message.value - 63.0) / 63.0f;
            }
          }
        }

        if (message.control == 123)
        {
          if (audioReactiveControlSwitch == 0)
          {
            p_lock[11][p_lock_increment] = (message.value - 63) / 63.0f;

            if (r_2 == true)
            {
              p_lock[11][p_lock_increment] = 2 * (message.value - 63.0) / 63.0f;
            }

            if (r_5 == true)
            {
              p_lock[11][p_lock_increment] = 4 * (message.value - 63.0) / 63.0f;
            }
            if (r_10 == true)
            {
              p_lock[11][p_lock_increment] = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC12 = (message.value - 63) / 63.0f;

            if (r_2 == true)
            {
              lowC12 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (r_5 == true)
            {
              lowC12 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (r_10 == true)
            {
              lowC12 = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC12 = (message.value - 63) / 63.0f;

            if (r_2 == true)
            {
              midC12 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (r_5 == true)
            {
              midC12 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (r_10 == true)
            {
              midC12 = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC12 = (message.value - 63) / 63.0f;

            if (r_2 == true)
            {
              highC12 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (r_5 == true)
            {
              highC12 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (r_10 == true)
            {
              highC12 = 8 * (message.value - 63.0) / 63.0f;
            }
          }
        }

        if (message.control == 124)
        {
          if (audioReactiveControlSwitch == 0)
          {
            p_lock[12][p_lock_increment] = (message.value) / 32.0f;

            if (huexx_0 == true)
            {
              p_lock[12][p_lock_increment] = message.value / 64.0f;
            }

            if (huexx_1 == true)
            {
              p_lock[12][p_lock_increment] = message.value / 96.0f;
            }
            if (huexx_2 == true)
            {
              p_lock[12][p_lock_increment] = message.value / 127.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC13 = (message.value) / 32.0f;

            if (huexx_0 == true)
            {
              lowC13 = message.value / 64.0f;
            }

            if (huexx_1 == true)
            {
              lowC13 = message.value / 96.0f;
            }
            if (huexx_2 == true)
            {
              lowC13 = message.value / 127.0f;
            }
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC13 = (message.value) / 32.0f;

            if (huexx_0 == true)
            {
              midC13 = message.value / 64.0f;
            }

            if (huexx_1 == true)
            {
              midC13 = message.value / 96.0f;
            }
            if (huexx_2 == true)
            {
              midC13 = message.value / 127.0f;
            }
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC13 = (message.value) / 32.0f;

            if (huexx_0 == true)
            {
              highC13 = message.value / 64.0f;
            }

            if (huexx_1 == true)
            {
              highC13 = message.value / 96.0f;
            }
            if (huexx_2 == true)
            {
              highC13 = message.value / 127.0f;
            }
          }
        }

        if (message.control == 125)
        {
          if (audioReactiveControlSwitch == 0)
          {
            p_lock[13][p_lock_increment] = (message.value - 63.0) / 63.0f;

            if (huexy_0 == true)
            {
              p_lock[13][p_lock_increment] = 2 * (message.value - 63.0) / 63.0f;
            }

            if (huexy_1 == true)
            {
              p_lock[13][p_lock_increment] = 4 * (message.value - 63.0) / 63.0f;
            }
            if (huexy_2 == true)
            {
              p_lock[13][p_lock_increment] = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC14 = (message.value - 63.0) / 63.0f;

            if (huexy_0 == true)
            {
              lowC14 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (huexy_1 == true)
            {
              lowC14 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (huexy_2 == true)
            {
              lowC14 = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC14 = (message.value - 63.0) / 63.0f;

            if (huexy_0 == true)
            {
              midC14 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (huexy_1 == true)
            {
              midC14 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (huexy_2 == true)
            {
              midC14 = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC14 = (message.value - 63.0) / 63.0f;

            if (huexy_0 == true)
            {
              highC14 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (huexy_1 == true)
            {
              highC14 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (huexy_2 == true)
            {
              highC14 = 8 * (message.value - 63.0) / 63.0f;
            }
          }
        }

        if (message.control == 126)
        {
          if (audioReactiveControlSwitch == 0)
          {
            p_lock[14][p_lock_increment] = (message.value - 63.0) / 63.0f;

            if (huexz_0 == true)
            {
              p_lock[14][p_lock_increment] = 2 * (message.value - 63.0) / 63.0f;
            }

            if (huexz_1 == true)
            {
              p_lock[14][p_lock_increment] = 4 * (message.value - 63.0) / 63.0f;
            }
            if (huexz_2 == true)
            {
              p_lock[14][p_lock_increment] = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC15 = (message.value - 63.0) / 63.0f;

            if (huexz_0 == true)
            {
              lowC15 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (huexz_1 == true)
            {
              lowC15 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (huexz_2 == true)
            {
              lowC15 = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC15 = (message.value - 63.0) / 63.0f;

            if (huexz_0 == true)
            {
              midC15 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (huexz_1 == true)
            {
              midC15 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (huexz_2 == true)
            {
              midC15 = 8 * (message.value - 63.0) / 63.0f;
            }
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC15 = (message.value - 63.0) / 63.0f;

            if (huexz_0 == true)
            {
              highC15 = 2 * (message.value - 63.0) / 63.0f;
            }

            if (huexz_1 == true)
            {
              highC15 = 4 * (message.value - 63.0) / 63.0f;
            }
            if (huexz_2 == true)
            {
              highC15 = 8 * (message.value - 63.0) / 63.0f;
            }
          }
        }

        if (message.control == 127)
        {
          if (audioReactiveControlSwitch == 0)
          {
            p_lock[15][p_lock_increment] = (message.value) / 127.0;
          }

          if (audioReactiveControlSwitch == 1)
          {
            lowC16 = (message.value) / 127.0;
          }

          if (audioReactiveControlSwitch == 2)
          {
            midC16 = (message.value) / 127.0;
          }

          if (audioReactiveControlSwitch == 3)
          {
            highC16 = (message.value) / 127.0;
          }
        }

        // gots to remap these to 60-61
        // cc43 maps to brightInvert
        if (message.control == 60)
        {
          if (message.value == 127)
          {
            brightInvert = true;
          }

          if (message.value == 0)
          {
            brightInvert = false;
          }
        }

        // cc44 maps to saturationInvert
        if (message.control == 61)
        {
          if (message.value == 127)
          {
            saturationInvert = true;
          }

          if (message.value == 0)
          {
            saturationInvert = false;
          }
        }

        if (message.control == 62)
        {
          if (message.value == 127)
          {
            // hueInvert=true;
            hdmi_aspect_ratio_switch = true;
          }

          if (message.value == 0)
          {
            // hueInvert=false;
            hdmi_aspect_ratio_switch = false;
          }
        }

        // cc41 maps to horizontalMirror
        if (message.control == 41)
        {
          if (message.value == 127)
          {
            horizontalMirror = true;
          }

          if (message.value == 0)
          {
            horizontalMirror = false;
          }
        }
        // cc45 maps to horizontalMirror
        if (message.control == 45)
        {
          if (message.value == 127)
          {
            verticalMirror = true;
          }

          if (message.value == 0)
          {
            verticalMirror = false;
          }
        }
        if (message.control == 59)
        {

          x_skew = y_skew = 0.0;
          lowC1 = lowC2 = lowC3 = lowC4 = lowC5 = lowC6 = lowC7 = lowC8 =
              lowC9 = lowC10 = lowC11 = lowC12 = lowC13 = lowC14 = lowC15 =
                  lowC16 = 0.0f;
          midC1 = midC2 = midC3 = midC4 = midC5 = midC6 = midC7 = midC8 =
              midC9 = midC10 = midC11 = midC12 = midC13 = midC14 = midC15 =
                  midC16 = 0.0f;
          highC1 = highC2 = highC3 = highC4 = highC5 = highC6 = highC7 =
              highC8 = highC9 = highC10 = highC11 = highC12 = highC13 =
                  highC14 = highC15 = highC16 = 0.0f;
        }
      }
    }
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}
