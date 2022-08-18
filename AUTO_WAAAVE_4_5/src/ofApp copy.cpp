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

#define CONTROL_THRESHOLD .025f

//flip this switch to try different scalings
//0 is 320 1 is 640
//if you reduce scale to 320 you can up the total delay time to
//about 4 seconds/ 120 frames
//so try that out sometime and see how that feels!
bool scaleswitch=1;
const int framebufferLength=60;
//const int framebufferLength=120;

//0 is picaputre, 1 is usbinput
bool inputswitch=1;

//0 is wet (framebuffer fed from final output, internal
//feedback mode
//1 is dry (framebuffer fed direct from camera input,
//traditional video delay mode
bool wet_dry_switch=1;

//0 is sd aspect ratio
//use definitely with all of the VSERPI devices 
//and anything else doing like 480i/p over hdmi
//1 is corner cropping to fill the screen
int hdmi_aspect_ratio_switch=0;

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

float fb1_brightx=0.0;
bool toroidSwitch=0;
bool y_skew_switch=FALSE;
bool x_skew_switch=FALSE;
bool lumakeyInvertSwitch=FALSE;
bool x_mirrorSwitch=FALSE;
bool y_mirrorSwitch=FALSE;
float y_skew=0;
float x_skew=0;
bool mirrorSwitch=FALSE;

int fb0_delayamount=0;

//dummy variables for midi control

int width=640;
int height=480;


//dummy variables for midi to audio attenuatiors
//0 is direct midi, 1 is low_x, 2 is mid_x, 3 is high_x
int control_switch=0;

float low_c1=0.0;
float low_c2=0.0;
float low_c3=0.0;
float low_c4=0.0;
float low_c5=0.0;
float low_c6=0.0;
float low_c7=0.0;
float low_c8=0.0;
float low_c9=0.0;
float low_c10=0.0;
float low_c11=0.0;
float low_c12=0.0;
float low_c13=0.0;
float low_c14=0.0;
float low_c15=0.0;
float low_c16=0.0;

float mid_c1=0.0;
float mid_c2=0.0;
float mid_c3=0.0;
float mid_c4=0.0;
float mid_c5=0.0;
float mid_c6=0.0;
float mid_c7=0.0;
float mid_c8=0.0;
float mid_c9=0.0;
float mid_c10=0.0;
float mid_c11=0.0;
float mid_c12=0.0;
float mid_c13=0.0;
float mid_c14=0.0;
float mid_c15=0.0;
float mid_c16=0.0;

float high_c1=0.0;
float high_c2=0.0;
float high_c3=0.0;
float high_c4=0.0;
float high_c5=0.0;
float high_c6=0.0;
float high_c7=0.0;
float high_c8=0.0;
float high_c9=0.0;
float high_c10=0.0;
float high_c11=0.0;
float high_c12=0.0;
float high_c13=0.0;
float high_c14=0.0;
float high_c15=0.0;
float high_c16=0.0;


bool clear_switch=0;
//toggle switch variables
bool hueInvert=FALSE;
bool saturationInvert=FALSE;
bool brightInvert=FALSE;

bool cam1_h_invert=FALSE;
bool cam1_s_invert=FALSE;
bool cam1_b_invert=FALSE;

bool horizontalMirror=FALSE;
bool verticalMirror=FALSE;

bool clear_flip=FALSE;

bool x_2=FALSE;
bool x_5=FALSE;
bool x_10=FALSE;

bool y_2=FALSE;
bool y_5=FALSE;
bool y_10=FALSE;

bool z_2=FALSE;
bool z_5=FALSE;
bool z_10=FALSE;

bool theta_0=FALSE;
bool theta_1=FALSE;
bool theta_2=FALSE;

bool huexx_0=FALSE;
bool huexx_1=FALSE;
bool huexx_2=FALSE;

bool huexy_0=FALSE;
bool huexy_1=FALSE;
bool huexy_2=FALSE;

bool huexz_0=FALSE;
bool huexz_1=FALSE;
bool huexz_2=FALSE;

//framebuffer management biziness

//framebufferLength is the total number of frames that will be stored.  setting framebufferLength to
//30 while fps is set to 30 means 1 second of video will be stored in memory  
//75 seems to work ok with usb cams but the capture device might need a little more memory

int framedelayoffset=0;  // this is used as an index to the circular framebuffers eeettt
unsigned int framecount=0; // framecounter used to calc offset eeettt

//this is an array of framebuffer objects
ofFbo pastFrames[framebufferLength];


void incIndex()  // call this every frame to calc the offset eeettt
{
    framecount++;
    framedelayoffset=framecount % framebufferLength;
}

//p_lock biz
//maximum total size of the plock array
const int p_lock_size=240;

bool p_lock_switch=0;

bool p_lock_erase=0;

//maximum number of p_locks available...maybe there can be one for every knob
//for whatever wacky reason the last member of this array of arrays has a glitch
//so i guess just make an extra array and forget about it for now
const int p_lock_number=17;

//so how we will organize the p_locks is in multidimensional arrays
//to access the data at timestep x for p_lock 2 (remember counting from 0) we use p_lock[2][x]
float p_lock[p_lock_number][p_lock_size];

//smoothing parameters(i think for all of these we can array both the arrays and the floats
//for now let us just try 1 smoothing parameter for everything.
float p_lock_smooth=.5;

//and then lets try an array of floats for storing the smoothed values
float p_lock_smoothed[p_lock_number];

//turn on and off writing to the array
bool p_lock_0_switch=1;

//global counter for all the locks
int p_lock_increment=0;

float my_normalize=0;

float low_value_smoothed=0.0;
float mid_value_smoothed=0.0;
float high_value_smoothed=0.0;
float smoothing_rate=.8;

ofFbo aspect_fix_fbo;
ofFbo dry_framebuffer;

bool videoReactiveSwitch=FALSE;

float vLumakeyValue=0.0;
float vMix=0.0;
float vHue=0.0;
float vSaturation=0.0;
float vBright=0.0;
float vTemporalFilterMix=0.0;
float vTemporalFilterResonance=0.0;
float vSharpenAmount=0.0;
float vX=0.0;
float vY=0.0;
float vZ=0.0;
float vRotate=0.0;
float vHuexMod=0.0;
float vHuexOff=0.0;
float vHuexLfo=0.0;

//latching
bool midiActiveFloat[17];
bool vmidiActiveFloat[17];
bool midiLowActiveFloat[17];
bool midiMidActiveFloat[17];
bool midiHighActiveFloat[17];

float range=200;
float fftLow;
float fftMid;
float fftHigh;

float c_lumakey_value=1.01;
float c_mix=2;
float c_hue=.25;
float c_sat=.2;
float c_bright=.2;
float c_temporalFilterMix=1.1;
float c_sharpenAmount=.5;
float c_x=.01;
float c_y=.01;
float c_z=.05;
float c_rotate=.314159265;
float c_huex_off=.25;
float c_huex_lfo=.25;

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
void ofApp::setup() {
	//ofSetVerticalSync(true);
	ofSetFrameRate(30);
    ofBackground(0);
	//ofToggleFullscreen();
    ofHideCursor();
	//omx_settings();  
	inputSetup();
	midiSetup();
	fbDeclareAndAllocate();
	shader_mixer.load("shadersES2/shader_mixer");
	shaderSharpen.load("shadersES2/shaderSharpen");
	//fft biz
	fft.setup();
    fft.setNormalize(false);
    //fft.setVolumeRange(1.0f);

	p_lockClear();
	midiLatchClear();
}
//-------------------------------------------------------------
void ofApp::midiLatchClear(){
	for(int i=0;i<17;i++){
		vmidiActiveFloat[i]=0;
		midiActiveFloat[i]=0;
		midiLowActiveFloat[i]=0;
		midiMidActiveFloat[i]=0;
		midiHighActiveFloat[i]=0;
	}
}
//--------------------------------------------------------------
void ofApp::update() {
	
	fft.update();
	inputUpdate();
	midibiz();
	p_lockUpdate();
	
	if(fft.getLowVal()>my_normalize){
		my_normalize=fft.getLowVal();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	
	
	//the range changes depending on what yr using for sound input.  just usb cams have shitter ranges
	//so 50 works
	//float range=200;
	
	fftAssignValues();
	
	parametersAssign();

	framebuffer0.begin();
    shader_mixer.begin();

	
	//videoGrabber.getTextureReference().draw(0, 0, 320, 640);
	if(scaleswitch==0){		
		if(inputswitch==0){
			//videoGrabber.draw(0,0,320,240);
		}
		if(inputswitch==1){
			cam1.draw(0,0,320,240);
		}
	}
	if(scaleswitch==1){
		if(inputswitch==0){
			//videoGrabber.draw(0,0);
		}
		if(inputswitch==1){
			if(hdmi_aspect_ratio_switch==0){
				cam1.draw(0,0,640,480);
			}
			if(hdmi_aspect_ratio_switch==1){
				aspect_fix_fbo.draw(0,0,640,480);
			}
		}
	}
	/*
	//textures
	shader_mixer.setUniformTexture("fb0", pastFrames[d_delay].getTexture(),1);
	
	if(wet_dry_switch==1){
		shader_mixer.setUniformTexture("fb1", pastFrames[(abs(framedelayoffset-framebufferLength)-1)%framebufferLength].getTexture(),2);
	}
	if(wet_dry_switch==0){
		shader_mixer.setUniformTexture("fb1", dry_framebuffer.getTexture(),2);
	}	
	//continuous variables
	shader_mixer.setUniform1f("fb0_lumakey_value",d_lumakey_value);
    shader_mixer.setUniform1f("fb0_mix",d_mix);
    shader_mixer.setUniform3f("fb0_hsbx",ofVec3f(d_hue,d_sat,d_bright));
    shader_mixer.setUniform1f("temporalFilterMix",d_temporalFilterMix);
    shader_mixer.setUniform1f("fb1_brightx",d_temporalFilterResonance );
    shader_mixer.setUniform1f("cam1_brightx",d_cam1_x);
    shader_mixer.setUniform1f("fb0_xdisplace",d_x);
    shader_mixer.setUniform1f("fb0_ydisplace",d_y);
    shader_mixer.setUniform1f("fb0_zdisplace",d_z);
    shader_mixer.setUniform1f("fb0_rotate",d_rotate);
    shader_mixer.setUniform3f("fb0_huex",ofVec3f(d_huex_mod,d_huex_off,d_huex_lfo));
    
    if(wet_dry_switch==1){
		shader_mixer.setUniform1i("toroidSwitch",toroidSwitch);
        shader_mixer.setUniform1i("mirrorSwitch",0);
	}
	
	 if(wet_dry_switch==0){
		shader_mixer.setUniform1i("toroidSwitch",0);
        shader_mixer.setUniform1i("mirrorSwitch",toroidSwitch);
	}
    
   
    shader_mixer.setUniform1i("brightInvert",brightInvert);
    shader_mixer.setUniform1i("hueInvert",hueInvert);
    shader_mixer.setUniform1i("saturationInvert",saturationInvert);
    
    shader_mixer.setUniform1i("horizontalMirror",horizontalMirror);
    shader_mixer.setUniform1i("verticalMirror",verticalMirror);
    
    
    shader_mixer.setUniform1i("lumakeyInvertSwitch",lumakeyInvertSwitch);
	*/
	//send the textures
	shader_mixer.setUniformTexture("fb", pastFrames[d_delay].getTexture(),1);
	if(wet_dry_switch==1){
		shader_mixer.setUniformTexture("temporalFilter", pastFrames[(abs(framedelayoffset-framebufferLength)-1)%framebufferLength].getTexture(),2);
	}
	if(wet_dry_switch==0){
		shader_mixer.setUniformTexture("temporalFilter", dry_framebuffer.getTexture(),2);
	}	
	//send the continuous variables
	shader_mixer.setUniform1f("lumakey",d_lumakey_value);
    shader_mixer.setUniform1f("fbMix",d_mix); 
    shader_mixer.setUniform1f("fbHue",d_hue);
    shader_mixer.setUniform1f("fbSaturation",d_sat);
    shader_mixer.setUniform1f("fbBright",d_bright);
    shader_mixer.setUniform1f("temporalFilterMix",d_temporalFilterMix);
    shader_mixer.setUniform1f("temporalFilterResonance",d_temporalFilterResonance );
    shader_mixer.setUniform1f("fbXDisplace",d_x);
    shader_mixer.setUniform1f("fbYDisplace",d_y);
    shader_mixer.setUniform1f("fbZDisplace",d_z);
    shader_mixer.setUniform1f("fbRotate",d_rotate);
    shader_mixer.setUniform3f("fb_huex",ofVec3f(d_huex_mod,d_huex_off,d_huex_lfo));
    shader_mixer.setUniform1f("fbHuexMod",d_huex_mod);
    shader_mixer.setUniform1f("fbHuexOff",d_huex_off);
	shader_mixer.setUniform1f("fbHuexLfo",d_huex_lfo);	
   
	//send the switches
    shader_mixer.setUniform1i("toroidSwitch",toroidSwitch);
    shader_mixer.setUniform1i("mirrorSwitch",mirrorSwitch);
    shader_mixer.setUniform1i("brightInvert",brightInvert);
    shader_mixer.setUniform1i("hueInvert",hueInvert);
    shader_mixer.setUniform1i("saturationInvert",saturationInvert);
    shader_mixer.setUniform1i("horizontalMirror",horizontalMirror);
    shader_mixer.setUniform1i("verticalMirror",verticalMirror);
    shader_mixer.setUniform1i("lumakeyInvertSwitch",lumakeyInvertSwitch);
    
    //send the videoreactive controls
    shader_mixer.setUniform1f("vLumakey",vLumakeyValue*c_lumakey_value);
    shader_mixer.setUniform1f("vMix",vMix*c_mix);
    shader_mixer.setUniform1f("vHue",2.0f*vHue*c_hue);
    shader_mixer.setUniform1f("vSat",2.0f*vSaturation*c_sat);
    shader_mixer.setUniform1f("vBright",2.0f*vBright*c_bright);
    shader_mixer.setUniform1f("vtemporalFilterMix",vTemporalFilterMix*c_temporalFilterMix);
    shader_mixer.setUniform1f("vFb1X",vTemporalFilterResonance);
    shader_mixer.setUniform1f("vX",.01*vX);
    shader_mixer.setUniform1f("vY",.01*vY);
    shader_mixer.setUniform1f("vZ",vZ*c_z);
    shader_mixer.setUniform1f("vRotate",vRotate*c_rotate);
    shader_mixer.setUniform1f("vHuexMod",1.0f-vHuexMod);
    shader_mixer.setUniform1f("vHuexOff",vHuexOff*c_huex_off);
    shader_mixer.setUniform1f("vHuexLfo",vHuexLfo*c_huex_lfo);
    shader_mixer.end();
	framebuffer0.end();
	
	/**sharpening mode**/	
	sharpenFramebuffer.begin();
	shaderSharpen.begin();
	framebuffer0.draw(0,0);
	shaderSharpen.setUniform1f("sharpenAmount",d_sharpenAmount);
	shaderSharpen.setUniform1f("vSharpenAmount",vSharpenAmount*c_sharpenAmount);
	shaderSharpen.end();
	sharpenFramebuffer.end();

	//framebuffer0.draw(0,0,ofGetWidth(),ofGetHeight());
	sharpenFramebuffer.draw(0,0,ofGetWidth(),ofGetHeight());
	
	pastFrames[abs(framebufferLength-framedelayoffset)-1].begin(); //eeettt
    if(wet_dry_switch==0){
		if(inputswitch==0){
			//videoGrabber.draw(0,0,framebuffer0.getWidth(),framebuffer0.getHeight());
		}
		if(inputswitch==1){
			if(hdmi_aspect_ratio_switch==0){
				cam1.draw(0,0,640,480);
			}
			if(hdmi_aspect_ratio_switch==1){
				aspect_fix_fbo.draw(0,0,640,480);
			}
		}
	
	dry_framebuffer.begin();
	//framebuffer0.draw(0,0);
	sharpenFramebuffer.draw(0,0);
	dry_framebuffer.end();
	
	}//endifwetdry0
		
	if(wet_dry_switch==1){
		//framebuffer0.draw(0,0);
		sharpenFramebuffer.draw(0,0);
		}//endifwetdry1
	
    pastFrames[abs(framebufferLength-framedelayoffset)-1].end(); //eeettt
	incIndex();
   
	//p_lock biz
	 if(p_lock_switch==1){
        p_lock_increment++;
        p_lock_increment=p_lock_increment%p_lock_size;
    }
   
	//ofDrawBitmapString("fps =" + ofToString(ofGetFrameRate())+"fft low =" + ofToString(fftLow)+"fft low_smoothed =" + ofToString(low_value_smoothed)+"fft mid =" + ofToString(fftLow)+"fft mid_smoothed =" + ofToString(mid_value_smoothed), 10, ofGetHeight() - 5 );
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
	
	
//i use this block of code to print out like useful information for debugging various things and/or just to keep the 
//framerate displayed to make sure i'm not losing any frames while testing out new features.  uncomment the ofDrawBitmap etc etc
//to print the stuff out on screen
   ofSetColor(255);
   string msg="fps="+ofToString(ofGetFrameRate(),2)+" clear switch"+ofToString(clear_switch,5);//+" z="+ofToString(az,5);
   //ofDrawBitmapString(msg,10,10);
}

//--------------------------------------------------------------
void ofApp::exit() {
	
	// clean up
	midiIn.closePort();
	midiIn.removeListener(this);
}
//--------------------------------------------------------------
/*

void ofApp::omx_settings(){
	
	settings.sensorWidth = 640;
    settings.sensorHeight = 480;
    settings.framerate = 30;
    settings.enableTexture = true;
    settings.sensorMode=7;
    
    settings.whiteBalance ="Off";
    settings.exposurePreset ="Off";
    settings.whiteBalanceGainR = 1.0;
    settings.whiteBalanceGainB = 1.0;
	
	}
//------------------------------------------------------------

void ofApp::omx_updates(){
	
		videoGrabber.setSharpness(50);
		videoGrabber.setBrightness(40);
		videoGrabber.setContrast(100);
		videoGrabber.setSaturation(0);
	
}
*/	
//--------------------------------------------------------------
void ofApp::fftAssignValues(){
	fftLow=fft.getLowVal();
	fftLow=ofClamp(fftLow,1.0f,range);
	fftLow=fftLow/range;
	low_value_smoothed=fftLow*smoothing_rate+(1.0f-smoothing_rate)*low_value_smoothed;
	
	fftMid=fft.getMidVal();
	fftMid=ofClamp(fftMid,1.0f,range);
	fftMid=fftMid/range;
	mid_value_smoothed=fftMid*smoothing_rate+(1.0f-smoothing_rate)*mid_value_smoothed;
	
	fftHigh=fft.getHighVal();
	fftHigh=ofClamp(fftHigh,1.0f,range);
	fftHigh=fftHigh/range;
	high_value_smoothed=fftHigh*smoothing_rate+(1.0f-smoothing_rate)*high_value_smoothed;
}
//-------------------------------------------------------------
void ofApp::parametersAssign(){
	if(wet_dry_switch==0){
		c_hue=1.0;
		c_sat=1.0;
		c_bright=1.0;
		c_x=.1;
		c_y=.1;
		c_z=.5;
	}
	
	d_delay=(abs(framedelayoffset-framebufferLength-fb0_delayamount-
				int((p_lock_smoothed[15]+low_c16*low_value_smoothed+mid_c16*mid_value_smoothed+high_c16*high_value_smoothed)*(framebufferLength-1.0))
				)-1)%framebufferLength;
	d_lumakey_value=kk+c_lumakey_value*p_lock_smoothed[0]+4.0f*(
							low_c1*low_value_smoothed+
							mid_c1*mid_value_smoothed+
							high_c1*high_value_smoothed);
	d_mix=jm+c_mix*p_lock_smoothed[1]+4.0f*(
							low_c2*low_value_smoothed+
							mid_c2*mid_value_smoothed+
							high_c2*high_value_smoothed);
	d_hue=fv*(1.0f+c_hue*p_lock_smoothed[2])+4.0f*(
							low_c3*low_value_smoothed+
							mid_c3*mid_value_smoothed+
							high_c3*high_value_smoothed);
	d_sat=gb*(1.0f+c_sat*p_lock_smoothed[3])+4.0f*(
							low_c4*low_value_smoothed+
							mid_c4*mid_value_smoothed+
							high_c4*high_value_smoothed);
	d_bright=hn*(1.0f+c_bright*p_lock_smoothed[4])+4.0f*(
							low_c5*low_value_smoothed+
							mid_c5*mid_value_smoothed+
							high_c5*high_value_smoothed);
	d_temporalFilterMix=op+c_temporalFilterMix*p_lock_smoothed[5]+4.0f*(
							low_c6*low_value_smoothed+
							mid_c6*mid_value_smoothed+
							high_c6*high_value_smoothed);
	d_temporalFilterResonance=fb1_brightx+p_lock_smoothed[6]+4.0*(
							low_c7*low_value_smoothed+
							mid_c7*mid_value_smoothed+
							high_c7*high_value_smoothed);
	d_sharpenAmount=ll+c_sharpenAmount*p_lock_smoothed[7]+8.0f*(
							low_c8*low_value_smoothed+
							mid_c8*mid_value_smoothed+
							high_c8*high_value_smoothed);
	d_x=sx+c_x*p_lock_smoothed[8]+4.0f*(
							c_x*(low_c9*low_value_smoothed+
							mid_c9*low_value_smoothed+
							high_c9*high_value_smoothed));
	d_y=dc+c_y*p_lock_smoothed[9]+4.0f*(
							c_y*(low_c10*low_value_smoothed+
							mid_c10*mid_value_smoothed+
							high_c10*high_value_smoothed));
	d_z=az*(1.0f+c_z*p_lock_smoothed[10])+4.0f*(
							c_z*(low_c11*low_value_smoothed+
							mid_c11*mid_value_smoothed+
							high_c11*high_value_smoothed));
	d_rotate=qw+c_rotate*p_lock_smoothed[11]+4.0*(
							c_rotate*(low_c12*low_value_smoothed+
							mid_c12*mid_value_smoothed+
							high_c12*high_value_smoothed));
	d_huex_mod=er*(1.0f-p_lock_smoothed[12])+4.0f*(
							low_c13*low_value_smoothed+
							mid_c13*mid_value_smoothed+
							high_c13*high_value_smoothed);
	d_huex_off=ty+c_huex_off*p_lock_smoothed[13]+4.0f*(
							c_huex_off*(low_c14*low_value_smoothed+
							mid_c14*mid_value_smoothed+
							high_c14*high_value_smoothed));
	d_huex_lfo=ui+c_huex_lfo*p_lock_smoothed[14]+4.0f*(
							c_huex_lfo*(low_c15*low_value_smoothed+
							mid_c15*mid_value_smoothed+
							high_c15*high_value_smoothed));
}	
//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
	// add the latest message to the message queue
	midiMessages.push_back(msg);
	// remove any old messages if we have too many
	while(midiMessages.size() > 2) {
		midiMessages.erase(midiMessages.begin());
	}
}
//------------------------------------------------------------
void ofApp::p_lockUpdate(){
	for(int i=0;i<p_lock_number;i++){
        p_lock_smoothed[i]=p_lock[i][p_lock_increment]*(1.0f-p_lock_smooth)+p_lock_smoothed[i]*p_lock_smooth; 
        if(abs(p_lock_smoothed[i])<.05){p_lock_smoothed[i]=0;} 
    }
}
//-------------------------------------------------------------
void ofApp::inputUpdate(){
	if(inputswitch==1){
		cam1.update();
		//corner crop and stretch to preserve hd aspect ratio
		if(hdmi_aspect_ratio_switch==1){
			aspect_fix_fbo.begin();
			cam1.draw(0,0,853,480);
			aspect_fix_fbo.end();
		}
	}
	if(inputswitch==0){
		//omx_updates();
	}
}
//------------------------------------------------------------
void ofApp::p_lockClear(){
	for(int i=0;i<p_lock_number;i++){
        for(int j=0;j<p_lock_size;j++){
            p_lock[i][j]=0;
        }
    }
}
//------------------------------------------------------------
void ofApp::midiSetup(){
	// print input ports to console
	midiIn.listInPorts();
	
	// open port by number (you may need to change this)
	midiIn.openPort(1);
	//midiIn.openPort("IAC Pure Data In");	// by name
	//midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port
	
	// don't ignore sysex, timing, & active sense messages,
	// these are ignored by default
	midiIn.ignoreTypes(false, false, false);
	
	// add ofApp as a listener
	midiIn.addListener(this);
	
	// print received messages to the console
	midiIn.setVerbose(true);
}
//-------------------------------------------------------------
void ofApp::fbDeclareAndAllocate(){
	framebuffer0.allocate(width,height);
	framebuffer0.begin();
	ofClear(0,0,0,255);
	framebuffer0.end();
	
	aspect_fix_fbo.allocate(width,height);
	aspect_fix_fbo.begin();
	ofClear(0,0,0,255);
	aspect_fix_fbo.end();
	
	dry_framebuffer.allocate(width,height);
	dry_framebuffer.begin();
	ofClear(0,0,0,255);
	dry_framebuffer.end();
	
	sharpenFramebuffer.allocate(width,height);
	sharpenFramebuffer.begin();
	ofClear(0,0,0,255);
	sharpenFramebuffer.end();
	
	for(int i=0;i<framebufferLength;i++){  
       pastFrames[i].allocate(width, height);
       pastFrames[i].begin();
       ofClear(0,0,0,255);
       pastFrames[i].end();
    }
}
//-------------------------------------------------------------
void ofApp::inputSetup(){
	 //pass in the settings and it will start the camera
	if(inputswitch==0){
		//videoGrabber.setup(settings);
	}
	if(inputswitch==1){
		cam1.setDesiredFrameRate(30);
		cam1.initGrabber(width,height);
	}
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == ')') {fft.setNormalize(true);}
	
	//error-if i decrement fb0_delayamount it always crashes...
	if (key == '[') {fb0_delayamount += 1;}
    if (key == ']') {
		fb0_delayamount = fb0_delayamount-1;
		if(fb0_delayamount<0){
			fb0_delayamount=framebufferLength-fb0_delayamount;
		}//endiffb0
	}//endifkey
    
    //fb1 mix
    if (key == 'o') {op += .01;}
    if (key == 'p') {op -= .01;}
    
    //fb0 z displace
    if (key == 'a') {az += .0001;}
    if (key == 'z') {az -= .0001;}
    
    //fb0 x displace
    if (key == 's') {sx += .0001;}
    if (key == 'x') {sx -= .0001;}
    
    //fb0 y displace
    if (key == 'd') {dc += .0001;}
    if (key == 'c') {dc -= .0001;}
    
    //fb0 hue attenuate
    if (key == 'f') {fv += .001;}
    if (key == 'v') {fv -= .001;}
    
    //fb0 saturation attenuate
    if (key == 'g') {gb += .001;}
    if (key == 'b') {gb -= .001;}
    
    //fb0 brightness attenuate
    if (key == 'h') {hn += .001;}
    if (key == 'n') {hn -= .001;}
    
    //fb0 mix
    if (key == 'j') {jm += .01;}
    if (key == 'm') {jm -= .01;}
    
    //fb0 lumakey value
    if (key == 'k') {kk = ofClamp(kk+.01,0.0,1.0);}
    if (key == ',') {kk = ofClamp(kk-.01,0.0,1.0);}
    
    
    if (key == 'l') {ll += .01;}
    if (key == '.') {ll -= .01;}
    
    if (key == ';') {fb1_brightx += .01;}
    if (key == '\'') {fb1_brightx -= .01;}
    
    //fb0 rotation
    if (key == 'q') {qw += .0001;}
    if (key == 'w') {qw -= .0001;}


	//hue chaos1
    if (key == 'e') {er += .001;}
    if (key == 'r') {er -= .001;}
    
    //hue chaos2
    if (key == 't') {ty += .01;}
    if (key == 'y') {ty -= .01;}
    
    //hue chaos3
    if (key == 'u') {ui += .01;}
    if (key == 'i') {ui -= .01;}

    if (key == '1') {
        //clear the framebuffer if thats whats up
        framebuffer0.begin();
        ofClear(0, 0, 0, 255);
        framebuffer0.end();
      for(int i=0;i<framebufferLength;i++){
        pastFrames[i].begin();
        ofClear(0,0,0,255);
        pastFrames[i].end();

		}//endifor
    }
    
    if(key=='2'){brightInvert=!brightInvert;}
    if(key=='3'){hueInvert=!hueInvert;}
    if(key=='4'){saturationInvert=!saturationInvert;}
    
    if(key=='5'){verticalMirror=!verticalMirror;}
    if(key=='6'){horizontalMirror=!horizontalMirror;}
    
    if(key=='7'){toroidSwitch=!toroidSwitch;}
    
    if (key == '-') {y_skew += .01;}
    if (key == '=') {y_skew -= .01;}
    if (key == '9') {x_skew += .01;}
    if (key == '0') {x_skew -= .01;}
    
	//reset button
    if (key == '!') {	
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
	fb0_delayamount=0;
	
	brightInvert=0;
	hueInvert=0;
	saturationInvert=0;
	
	verticalMirror=0;
	horizontalMirror=0;
	
	x_skew=0;
	y_skew=0;
	
	framebuffer0.begin();
    ofClear(0, 0, 0, 255);
    framebuffer0.end();
	 
	for(int i=0;i<framebufferLength;i++){
        pastFrames[i].begin();
        ofClear(0,0,0,255);
        pastFrames[i].end();
		}//endifor
	}
}
//------------------------------------------------------------------
void ofApp::midibiz(){
	
    //lets figure out the hd switch thing here
	//bool cc_aspect_switch=0;
    //int cc_aspect_int=0;
                
	for(unsigned int i = 0; i < midiMessages.size(); ++i) {

		ofxMidiMessage &message = midiMessages[i];
	
		if(message.status < MIDI_SYSEX) {
			//text << "chan: " << message.channel;
            if(message.status == MIDI_CONTROL_CHANGE) {
                
                //How to Midi Map
                //uncomment the line that says cout<<message control etc
                //run the code and look down in the console
                //when u move a knob on yr controller keep track of the number that shows up
                //that is the cc value of the knob
                //then go down to the part labled 'MIDIMAPZONE'
                //and change the numbers for each if message.control== statement to the values
                //on yr controller
                
                 // cout << "message.control"<< message.control<< endl;
                 // cout << "message.value"<< message.value<< endl;

                //MIDIMAPZONE
                //these are mostly all set to output bipolor controls at this moment (ranging from -1.0 to 1.0)
                //if u uncomment the second line on each of these if statements that will switch thems to unipolor
                //controls (ranging from 0.0to 1.0) if  you prefer

                //videoreactive
                if(message.control==39){
                    if(message.value==127){
						videoReactiveSwitch=1;
						p_lock_0_switch=0;
                    }
                    if(message.value==0){
						videoReactiveSwitch=0;
						p_lock_0_switch=1;
                    }
                }
                if(message.control==55){
                    if(message.value==127){
                        p_lock_switch=1;
                        for(int i=0;i<p_lock_number;i++){
							p_lock_smoothed[i]=0;
							for(int j=0;j<p_lock_size;j++){
								p_lock[i][j]=p_lock[i][p_lock_increment];
							}
						}
                    }
                    if(message.value==0){
                        p_lock_switch=0;
                    }
                }
                if(message.control==32){
					if(message.value==127){
						x_2=TRUE;
					}		
					if(message.value==0){
						x_2=FALSE;
					}
                }
                if(message.control==48){
					if(message.value==127){
						x_5=TRUE;
					}
					if(message.value==0){
						x_5=FALSE;
					}
                }
                if(message.control==64){
					if(message.value==127){
						x_10=TRUE;
					}
					if(message.value==0){
						x_10=FALSE;
					}
                }
                if(message.control==33){
					if(message.value==127){
						y_2=TRUE;
					}	
					if(message.value==0){
						y_2=FALSE;
					}
                }
                if(message.control==49){
					if(message.value==127){
						y_5=TRUE;
					}
					if(message.value==0){
						y_5=FALSE;
					}
                }
                if(message.control==65){
					if(message.value==127){
						y_10=TRUE;
					}
					if(message.value==0){
						y_10=FALSE;
					}
                }
                if(message.control==34){
					if(message.value==127){
						z_2=TRUE;
					}
					if(message.value==0){
						z_2=FALSE;
					}
                }
                if(message.control==50){
					if(message.value==127){
						z_5=TRUE;
					}
					if(message.value==0){
						z_5=FALSE;
					}
                }
                if(message.control==66){
					if(message.value==127){
						z_10=TRUE;
					}
					if(message.value==0){
						z_10=FALSE;
					}
                }
                if(message.control==35){
					if(message.value==127){
						theta_0=TRUE;
					}	
					if(message.value==0){
						theta_0=FALSE;
					}
                }
                if(message.control==51){
					if(message.value==127){
						theta_1=TRUE;
					}
					if(message.value==0){
						theta_1=FALSE;
					}
                }
                if(message.control==67){
					if(message.value==127){
						theta_2=TRUE;
					}
					if(message.value==0){
						theta_2=FALSE;
					}
                }
                if(message.control==36){
					if(message.value==127){
						huexx_0=TRUE;
					}
					if(message.value==0){
						huexx_0=FALSE;
					}
                }
                if(message.control==52){
					if(message.value==127){
						huexx_1=TRUE;
					}
					if(message.value==0){
						huexx_1=FALSE;
					}
                }
                if(message.control==68){
					if(message.value==127){
						huexx_2=TRUE;
					}
					if(message.value==0){
						huexx_2=FALSE;
					}
                }
                if(message.control==46){
					if(message.value==127){
						toroidSwitch=TRUE;
					}
					if(message.value==0){
						toroidSwitch=FALSE;
					}
                }
                if(message.control==61){
					if(message.value==127){
						mirrorSwitch=TRUE;
					}
					if(message.value==0){
						mirrorSwitch=FALSE;
					}
                }
                if(y_skew_switch==TRUE){
					y_skew+=.00001;
                }
                if(x_skew_switch==TRUE){
					x_skew+=.00001;
                }
                if(message.control==71){
					if(message.value==127){
						wet_dry_switch=FALSE;
					}
					if(message.value==0){
						wet_dry_switch=TRUE;
					}
		        }
				if(message.control==37){
					if(message.value==127){
						huexy_0=TRUE;
					}
					if(message.value==0){
						huexy_0=FALSE;
					}
                }
                if(message.control==53){
					if(message.value==127){
						huexy_1=TRUE;
					}
					if(message.value==0){
						huexy_1=FALSE;
					}
                }
                if(message.control==69){
					if(message.value==127){
						huexy_2=TRUE;
					}
					if(message.value==0){
						huexy_2=FALSE;
					}
                }
                if(message.control==38){
					if(message.value==127){
						huexz_0=TRUE;
					}
					if(message.value==0){
						huexz_0=FALSE;
					}
                }
                if(message.control==54){
					if(message.value==127){
						huexz_1=TRUE;
					}
					if(message.value==0){
						huexz_1=FALSE;
					}
                }
                if(message.control==70){
					if(message.value==127){
						huexz_2=TRUE;
					}
					if(message.value==0){
						huexz_2=FALSE;
					}
                }
                if(message.control==60){
					if(message.value==127){
						lumakeyInvertSwitch=TRUE;
					}
					if(message.value==0){
						lumakeyInvertSwitch=FALSE;
					}
                }
                //CONTINUOUS CONTROLS
                if(message.control==16){
					if(control_switch==0){	
						if(p_lock_0_switch==1){
							vmidiActiveFloat[0]=0;
							if(abs(message.value/127.0f-p_lock[0][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[0]=1;
							}
							if(midiActiveFloat[0]==1){
								p_lock[0][p_lock_increment]=message.value/127.0f;
							}
						}
						if(videoReactiveSwitch==1){
							midiActiveFloat[0]=0;
							if(abs(message.value/127.0f-vLumakeyValue)<CONTROL_THRESHOLD){
								vmidiActiveFloat[0]=1;
							}
							if(vmidiActiveFloat[0]==1){
								vLumakeyValue=message.value/127.0f;
							}		
						}              
                    }
               }
               if(message.control==17){
                   if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[1]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[1][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[1]=1;
							}
							if(midiActiveFloat[1]==1){
								p_lock[1][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
						}
                    }
					if(videoReactiveSwitch==1){
						midiActiveFloat[1]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vMix)<CONTROL_THRESHOLD){
							vmidiActiveFloat[1]=1;
						}
						if(vmidiActiveFloat[1]==1){
							vMix=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
						}
					}  
                }
                //c3 maps to fb hue
                if(message.control==18){
                    if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[2]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[2][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[2]=1;
							}
							if(midiActiveFloat[2]==1){
								p_lock[2][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
						}
                    }
					if(videoReactiveSwitch==1){
						midiActiveFloat[2]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vHue)<CONTROL_THRESHOLD){
							vmidiActiveFloat[2]=1;
						}
						if(vmidiActiveFloat[2]==1){
							vHue=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
						}
					} 
                }
                //c4 maps to fb satx
                if(message.control==19){
                  if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[3]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[3][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[3]=1;
							}
							if(midiActiveFloat[3]==1){
								p_lock[3][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
						}
                    }
					if(videoReactiveSwitch==1){
						midiActiveFloat[3]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vSaturation)<CONTROL_THRESHOLD){
							vmidiActiveFloat[3]=1;
						}
						if(vmidiActiveFloat[3]==1){
							vSaturation=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
						}
					} 
                }
                //c5 maps to fb brightx
                if(message.control==20){
                    if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[4]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[4][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[4]=1;
							}
							if(midiActiveFloat[4]==1){
								p_lock[4][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
						}
                    }
					if(videoReactiveSwitch==1){
						midiActiveFloat[4]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vBright)<CONTROL_THRESHOLD){
							vmidiActiveFloat[4]=1;
						}
						if(vmidiActiveFloat[4]==1){
							vBright=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
						}
					} 
                }
                //c6 maps to temporal filter
                if(message.control==21){
                   if(control_switch==0){
					   if(p_lock_0_switch==1){
					   vmidiActiveFloat[5]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[5][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[5]=1;
							}
							if(midiActiveFloat[5]==1){
								p_lock[5][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
						}
                    }
					if(videoReactiveSwitch==1){
						midiActiveFloat[5]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vTemporalFilterMix)<CONTROL_THRESHOLD){
							vmidiActiveFloat[5]=1;
						}
						if(vmidiActiveFloat[5]==1){
							vTemporalFilterMix =(message.value-MIDI_MAGIC)/MIDI_MAGIC;
						}
					} 
                }
                //c7 maps to temporal filter resonance
                if(message.control==22){
                    if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[6]=0;
							if(abs(message.value/127.0f-p_lock[6][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[6]=1;
							}
							if(midiActiveFloat[6]==1){
								p_lock[6][p_lock_increment]=message.value/127.0f;
							}
						}
						if(videoReactiveSwitch==1){
							midiActiveFloat[6]=0;
							if(abs(message.value/127.0f-vTemporalFilterResonance)<CONTROL_THRESHOLD){
								vmidiActiveFloat[6]=1;
							}
							if(vmidiActiveFloat[6]==1){
								vTemporalFilterResonance=message.value/127.0f;
							}		
						}              
                    }
                }
                //c8 maps to sharpen
                if(message.control==23){
                     if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[7]=0;
							if(abs(message.value/127.0f-p_lock[7][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[7]=1;
							}
							if(midiActiveFloat[7]==1){
								p_lock[7][p_lock_increment]=message.value/127.0f;
							}
						}
						if(videoReactiveSwitch==1){
							midiActiveFloat[7]=0;
							if(abs(message.value/127.0f-vSharpenAmount)<CONTROL_THRESHOLD){
								vmidiActiveFloat[7]=1;
							}
							if(vmidiActiveFloat[7]==1){
								vSharpenAmount=message.value/127.0f;
							}		
						}              
                    }
                }

                //c9 maps to fb x displace
                if(message.control==120){
					if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[8]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[8][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[8]=1;
							}
							if(midiActiveFloat[8]==1){
								p_lock[8][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								if(x_2==TRUE){
									p_lock[8][p_lock_increment]=2.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(x_5==TRUE){
									p_lock[8][p_lock_increment]=5.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(x_10==TRUE){
									p_lock[8][p_lock_increment]=10.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
							}		 	 
						}
                    }
                    if(videoReactiveSwitch==1){
						midiActiveFloat[8]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vX)<CONTROL_THRESHOLD){
								vmidiActiveFloat[8]=1;
						}
						if(vmidiActiveFloat[8]==1){
							vX=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							if(x_2==TRUE){
								vX=2.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(x_5==TRUE){
								vX=5.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(x_10==TRUE){
								vX=10.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}	 
						}		 
                    }
                }
                if(message.control==121){
					if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[9]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[9][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[9]=1;
							}
							if(midiActiveFloat[9]==1){
								p_lock[9][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								if(y_2==TRUE){
									p_lock[9][p_lock_increment]=2.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(y_5==TRUE){
									p_lock[9][p_lock_increment]=5.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(y_10==TRUE){
									p_lock[9][p_lock_increment]=10.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
							}		 	 
						}
                    }
                    if(videoReactiveSwitch==1){
						midiActiveFloat[9]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vY)<CONTROL_THRESHOLD){
								vmidiActiveFloat[9]=1;
						}
						if(vmidiActiveFloat[9]==1){
							vY=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							if(y_2==TRUE){
								vY=2.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(y_5==TRUE){
								vY=5.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(y_10==TRUE){
								vY=10.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}	 
						}		 
                    }
                }                
               //z displace
                if(message.control==122){
					if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[10]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[10][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[10]=1;
							}
							if(midiActiveFloat[10]==1){
								p_lock[10][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;						 
								if(z_2==TRUE){
									p_lock[10][p_lock_increment]=2.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(z_5==TRUE){
									p_lock[10][p_lock_increment]=5.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(z_10==TRUE){
									p_lock[10][p_lock_increment]=10.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
							}		 	 
						}
                    }
                    if(videoReactiveSwitch==1){
						midiActiveFloat[10]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vZ)<CONTROL_THRESHOLD){
								vmidiActiveFloat[10]=1;
						}
						if(vmidiActiveFloat[10]==1){
							vZ=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							if(z_2==TRUE){
								vZ=2.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(z_5==TRUE){
								vZ=5.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(z_10==TRUE){
								vZ=10.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}	 
						}		 
                    }	
                }
				//rotations
                if(message.control==123){
					if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[11]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[11][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[11]=1;
							}
							if(midiActiveFloat[11]==1){
								p_lock[11][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
						 
								if(theta_0==TRUE){
									p_lock[11][p_lock_increment]=2.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(theta_1==TRUE){
									p_lock[11][p_lock_increment]=5.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(theta_2==TRUE){
									p_lock[11][p_lock_increment]=10.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
							}		 	 
						}
                    }
                    if(videoReactiveSwitch==1){
						midiActiveFloat[11]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vRotate)<CONTROL_THRESHOLD){
								vmidiActiveFloat[11]=1;
						}
						if(vmidiActiveFloat[11]==1){
							vRotate=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							if(theta_0==TRUE){
								vRotate=2.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							} 
							if(theta_1==TRUE){
								vRotate=5.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(theta_2==TRUE){
								vRotate=10.0*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}	 
						}		 
                    }	
                }
                if(message.control==124){
                    if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[12]=0;
							if(abs(message.value/127.0f-p_lock[12][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[12]=1;
							}
							if(midiActiveFloat[12]==1){
								p_lock[12][p_lock_increment]=(message.value)/32.0f;
								if(huexx_0==TRUE){
									p_lock[12][p_lock_increment]=message.value/64.0f;
								}
								if(huexx_1==TRUE){
									p_lock[12][p_lock_increment]=message.value/96.0f;
								}
								if(huexx_2==TRUE){
									p_lock[12][p_lock_increment]=message.value/127.0f;
								}
							}		 
						}
					}
					if(videoReactiveSwitch==1){
						midiActiveFloat[12]=0;
						if(abs(message.value/127.0f-vHuexMod)<CONTROL_THRESHOLD){
								vmidiActiveFloat[12]=1;
						}
						if(vmidiActiveFloat[12]==1){
							vHuexMod=(message.value)/32.0f;
							if(huexx_0==TRUE){
								vHuexMod=message.value/64.0f;
							}
							 
							if(huexx_1==TRUE){
								vHuexMod=message.value/96.0f;
							}
							if(huexx_2==TRUE){
								vHuexMod=message.value/127.0f;
							}
						}
					}
                }
                if(message.control==125){
					if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[13]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[13][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[13]=1;
							}
							if(midiActiveFloat[13]==1){
								p_lock[13][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								if(huexy_0==TRUE){
									p_lock[13][p_lock_increment]=2*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(huexy_1==TRUE){
									p_lock[13][p_lock_increment]=4*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(huexy_2==TRUE){
									p_lock[13][p_lock_increment]=8*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
							}	 
						}
					}
					if(videoReactiveSwitch==1){
						midiActiveFloat[13]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vHuexOff)<CONTROL_THRESHOLD){
							vmidiActiveFloat[13]=1;
						}
						if(vmidiActiveFloat[13]==1){
							vHuexOff=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							if(huexy_0==TRUE){
								vHuexOff=2*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(huexy_1==TRUE){
								vHuexOff=4*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(huexy_2==TRUE){
								vHuexOff=8*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
						}    
					}
                }
                if(message.control==126){
					if(control_switch==0){
						if(p_lock_0_switch==1){
							vmidiActiveFloat[14]=0;
							if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-p_lock[14][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[14]=1;
							}
							if(midiActiveFloat[14]==1){
								p_lock[14][p_lock_increment]=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								if(huexz_0==TRUE){
									p_lock[14][p_lock_increment]=2.0f*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								} 
								if(huexz_1==TRUE){
									p_lock[14][p_lock_increment]=4.0f*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}
								if(huexz_2==TRUE){
									p_lock[14][p_lock_increment]=8.0f*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
								}	 
							}
						}
                     }
                     if(videoReactiveSwitch==1){
						midiActiveFloat[14]=0;
						if(abs((message.value-MIDI_MAGIC)/MIDI_MAGIC-vHuexLfo)<CONTROL_THRESHOLD){
							vmidiActiveFloat[14]=1;
						}
						if(vmidiActiveFloat[14]==1){
							vHuexLfo=(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							if(huexz_0==TRUE){
								vHuexLfo=2.0f*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(huexz_1==TRUE){
								vHuexLfo=4.0f*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
							if(huexz_2==TRUE){
								vHuexLfo=8.0f*(message.value-MIDI_MAGIC)/MIDI_MAGIC;
							}
						}    
					} 
				}
                if(message.control==127){
					if(control_switch==0){
						if(p_lock_0_switch==1){
							if(abs(message.value/127.0f-p_lock[15][p_lock_increment])<CONTROL_THRESHOLD){
								midiActiveFloat[15]=1;
							}
							if(midiActiveFloat[15]==1){
								p_lock[15][p_lock_increment]=(message.value)/127.0;
							}
						}
					} 
                }
                if(message.control==43){
					if(message.value==127){
						brightInvert=TRUE;
					}
					if(message.value==0){
						brightInvert=FALSE;
					}
                }
                if(message.control==44){
					if(message.value==127){
						saturationInvert=TRUE;
					}
					if(message.value==0){
						saturationInvert=FALSE;
					}
                }
                if(message.control==62){
					if(message.value==127){
						hdmi_aspect_ratio_switch=TRUE;
					}
					if(message.value==0){
						hdmi_aspect_ratio_switch=FALSE;
					}
                }
                if(message.control==42){
					if(message.value==127){
						hueInvert=TRUE;						
					}					
					if(message.value==0){
						hueInvert=FALSE;
					}
                }
                if(message.control==41){
					if(message.value==127){
						horizontalMirror=TRUE;
					}
					if(message.value==0){
						horizontalMirror=FALSE;
					}
                }
                if(message.control==45){
					if(message.value==127){
						verticalMirror=TRUE;
					}
					if(message.value==0){
						verticalMirror=FALSE;
					}
                 }
                 if(message.control==59){		
					if(message.value==127){
						vLumakeyValue=0.0;
						vMix=0.0;
						vHue=0.0;
						vSaturation=0.0;
						vBright=0.0;
						vTemporalFilterMix=0.0;
						vTemporalFilterResonance=0.0;
						vSharpenAmount=0.0;
						vX=0.0;
						vY=0.0;
						vZ=0.0;
						vRotate=0.0;
						vHuexMod=0.0;
						vHuexOff=0.0;
						vHuexLfo=0.0;
                        for(int i=0;i<p_lock_number;i++){
                            midiActiveFloat[i]=0;
                            vmidiActiveFloat[i]=0;
                            for(int j=0;j<p_lock_size;j++){                               
                                p_lock[i][j]=0;   
                            }                        
                        }
                    }
               }
               if(message.control==58){
					if(message.value==127){
						if(clear_switch==0){
							clear_switch=1;
							//clear the framebuffer if thats whats up
							framebuffer0.begin();
							ofClear(0, 0, 0, 255);
							framebuffer0.end();
							for(int i=0;i<framebufferLength;i++){
								pastFrames[i].begin();
								ofClear(0,0,0,255);
								pastFrames[i].end();
							}
						}
					}
                }
                if(message.control!=58){
					clear_switch=0;
                }
            }		
		}
	}	
}

//-----------------------------------------------------------
void ofApp::midibizOld(){
	for(unsigned int i = 0; i < midiMessages.size(); ++i) {

		ofxMidiMessage &message = midiMessages[i];
	
		if(message.status < MIDI_SYSEX) {
			//text << "chan: " << message.channel;
            if(message.status == MIDI_CONTROL_CHANGE) {
                
                //How to Midi Map
                //uncomment the line that says cout<<message control etc
                //run the code and look down in the console
                //when u move a knob on yr controller keep track of the number that shows up
                //that is the cc value of the knob
                //then go down to the part labled 'MIDIMAPZONE'
                //and change the numbers for each if message.control== statement to the values
                //on yr controller
                
                 // cout << "message.control"<< message.control<< endl;
                 // cout << "message.value"<< message.value<< endl;
                
                
                
                //MIDIMAPZONE
                //these are mostly all set to output bipolor controls at this moment (ranging from -1.0 to 1.0)
                //if u uncomment the second line on each of these if statements that will switch thems to unipolor
                //controls (ranging from 0.0to 1.0) if  you prefer

                if(message.control==39){
                    if(message.value==127){
                        p_lock_0_switch=0;
                    }
                    if(message.value==0){
                        p_lock_0_switch=1;
                    }
                }                                
                if(message.control==55){
                    if(message.value==127){
                        p_lock_switch=1;
                    }
                    if(message.value==0){
                        p_lock_switch=0;
                    }
                }
                if(message.control==58){
                    if(message.value==127){
                        for(int i=0;i<p_lock_number;i++){
                            for(int j=0;j<p_lock_size;j++){
                                p_lock[i][j]=0;                              
                            }                           
                        }
                    } 
                }               
                if(message.control==32){
					if(message.value==127){
						x_2=TRUE;
					}
					if(message.value==0){
						x_2=FALSE;
					}
                }
                if(message.control==48){
					if(message.value==127){
						x_5=TRUE;
					}
					if(message.value==0){
						x_5=FALSE;
					}
                }  
                if(message.control==64){
					if(message.value==127){
						x_10=TRUE;
					}
					
					if(message.value==0){
						x_10=FALSE;
					}
                }
                
                
                
                if(message.control==33){
					if(message.value==127){
						y_2=TRUE;
					}
					
					if(message.value==0){
						y_2=FALSE;
					}
                }
                
                if(message.control==49){
					if(message.value==127){
						y_5=TRUE;
					}
					
					if(message.value==0){
						y_5=FALSE;
					}
                }
                
                 if(message.control==65){
					if(message.value==127){
						y_10=TRUE;
					}
					
					if(message.value==0){
						y_10=FALSE;
					}
                }
                
                
                
                if(message.control==34){
					if(message.value==127){
						z_2=TRUE;
					}
					
					if(message.value==0){
						z_2=FALSE;
					}
                }
                
                if(message.control==50){
					if(message.value==127){
						z_5=TRUE;
					}
					
					if(message.value==0){
						z_5=FALSE;
					}
                }
                
                 if(message.control==66){
					if(message.value==127){
						z_10=TRUE;
					}
					
					if(message.value==0){
						z_10=FALSE;
					}
                }
                
                
                
                
                if(message.control==35){
					if(message.value==127){
						theta_0=TRUE;
					}
					
					if(message.value==0){
						theta_0=FALSE;
					}
                }
                
                if(message.control==51){
					if(message.value==127){
						theta_1=TRUE;
					}
					
					if(message.value==0){
						theta_1=FALSE;
					}
                }
                
                 if(message.control==67){
					if(message.value==127){
						theta_2=TRUE;
					}
					
					if(message.value==0){
						theta_2=FALSE;
					}
                }
                
                
                
                
                if(message.control==36){
					if(message.value==127){
						huexx_0=TRUE;
					}
					
					if(message.value==0){
						huexx_0=FALSE;
					}
                }
                
                if(message.control==52){
					if(message.value==127){
						huexx_1=TRUE;
					}
					
					if(message.value==0){
						huexx_1=FALSE;
					}
                }
                
                 if(message.control==68){
					if(message.value==127){
						huexx_2=TRUE;
					}
					
					if(message.value==0){
						huexx_2=FALSE;
					}
                }
                
                if(message.control==46){
					if(message.value==127){
						toroidSwitch=TRUE;
					}
					
					if(message.value==0){
						toroidSwitch=FALSE;
					}
                }
                
                
                
             
                if(y_skew_switch==TRUE){
					y_skew+=.00001;
                }
                
                if(x_skew_switch==TRUE){
					x_skew+=.00001;
                }
                
                
                if(message.control==71){
					if(message.value==127){
						wet_dry_switch=FALSE;
					}
					
					if(message.value==0){
						wet_dry_switch=TRUE;
					}
					
                }
                
                //---------------------
                
                
                
				if(message.control==37){
					if(message.value==127){
						huexy_0=TRUE;
					}
					
					if(message.value==0){
						huexy_0=FALSE;
					}
                }
                
                if(message.control==53){
					if(message.value==127){
						huexy_1=TRUE;
					}
					
					if(message.value==0){
						huexy_1=FALSE;
					}
                }
                
                 if(message.control==69){
					if(message.value==127){
						huexy_2=TRUE;
					}
					
					if(message.value==0){
						huexy_2=FALSE;
					}
                }
                
                
                //---------------------
                
                
                
                if(message.control==38){
					if(message.value==127){
						huexz_0=TRUE;
					}
					
					if(message.value==0){
						huexz_0=FALSE;
					}
                }
                
                if(message.control==54){
					if(message.value==127){
						huexz_1=TRUE;
					}
					
					if(message.value==0){
						huexz_1=FALSE;
					}
                }
                
                 if(message.control==70){
					if(message.value==127){
						huexz_2=TRUE;
					}
					
					if(message.value==0){
						huexz_2=FALSE;
					}
                }
                if(message.control==43){
					if(message.value==127){
						control_switch=1;
					}
					
					if(message.value==0){
						control_switch=0;
					}
					
                }
                
                if(message.control==44){
					if(message.value==127){
						control_switch=2;
					}
					
					if(message.value==0){
						control_switch=0;
					}
					
                }
                
                 if(message.control==42){
					if(message.value==127){
						control_switch=3;
					}
					
					if(message.value==0){
						control_switch=0;
					}
					
                }
                //nanokontrol2 controls
                 //c1 maps to fb0 lumakey
                if(message.control==16){
                   
					if(control_switch==0){	
						if(p_lock_0_switch==1){
							p_lock[0][p_lock_increment]=message.value/127.0f;
						}              
                    }
                    
                    if(control_switch==1){
						low_c1=message.value/127.0f;
					}
					
					if(control_switch==2){
						mid_c1=message.value/127.0f;
					}
					
					if(control_switch==3){
						high_c1=message.value/127.0f;
					}
                }
                
               if(message.control==17){
                   
                   if(control_switch==0){
						if(p_lock_0_switch==1){
							p_lock[1][p_lock_increment]=(message.value-63)/63.0f;
						}
                    }
                    
                    if(control_switch==1){
						low_c2=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==2){
						mid_c2=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==3){
						high_c2=(message.value-63.0f)/63.0f;
					}
                }
                
                //c3 maps to fb0 huex
                if(message.control==18){
                    if(control_switch==0){
						if(p_lock_0_switch==1){
							p_lock[2][p_lock_increment]=(message.value-63)/63.0f;
						}
                    }
                    if(control_switch==1){
						low_c3=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==2){
						mid_c3=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==3){
						high_c3=(message.value-63.0f)/63.0f;
					}
                    
                }
                
                //c4 maps to fb0 satx
                if(message.control==19){
                  if(control_switch==0){
						if(p_lock_0_switch==1){
							p_lock[3][p_lock_increment]=(message.value-63)/63.0f;
						}
                    }
                   if(control_switch==1){
						low_c4=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==2){
						mid_c4=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==3){
						high_c4=(message.value-63.0f)/63.0f;
					}
                }
                
                //c5 maps to fb0 brightx
                if(message.control==20){
                    if(control_switch==0){
						if(p_lock_0_switch==1){
							p_lock[4][p_lock_increment]=(message.value-63)/63.0f;
						}
                    }
					if(control_switch==1){
						low_c5=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==2){
						mid_c5=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==3){
						high_c5=(message.value-63.0f)/63.0f;
					}
					
                }
                
                //c6 maps to temporal filter
                if(message.control==21){
                   if(control_switch==0){
						if(p_lock_0_switch==1){
							p_lock[5][p_lock_increment]=(message.value-63)/63.0f;
						}
                    }
                    if(control_switch==1){
						low_c6=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==2){
						mid_c6=(message.value-63.0f)/63.0f;
					}
					
					if(control_switch==3){
						high_c6=(message.value-63.0f)/63.0f;
					}
                }
                
                //c7 maps to temporal filter resonance
                if(message.control==22){
                    if(control_switch==0){
						if(p_lock_0_switch==1){
							p_lock[6][p_lock_increment]=(message.value)/127.0f;
						}
                    }
                    if(control_switch==1){
						low_c7=(message.value)/127.0f;
					}
					
					if(control_switch==2){
						mid_c7=(message.value)/127.0f;
					}
					
					if(control_switch==3){
						high_c7=(message.value)/127.0f;
					}
                }
                
                //c8 maps to brightx
                if(message.control==23){
                     if(control_switch==0){
						if(p_lock_0_switch==1){
							p_lock[7][p_lock_increment]=(message.value)/127.0f;
						}
                    }
                   if(control_switch==1){
						low_c8=(message.value)/127.0f;
					}
					
					if(control_switch==2){
						mid_c8=(message.value)/127.0f;
					}
					
					if(control_switch==3){
						high_c8=(message.value)/127.0f;
					}
                   
                }
                
                //c9 maps to fb0 x displace
                if(message.control==120){
					if(control_switch==0){
						p_lock[8][p_lock_increment]=(message.value-63)/63.0f;
                     
						if(x_2==TRUE){
							p_lock[8][p_lock_increment]=2.0*(message.value-63.0)/63.0f;
							}
						 
						if(x_5==TRUE){
							p_lock[8][p_lock_increment]=5.0*(message.value-63.0)/63.0f;
							}
						if(x_10==TRUE){
							p_lock[8][p_lock_increment]=10.0*(message.value-63.0)/63.0f;
							}	 	 
                    }
                    
                    if(control_switch==1){
						low_c9=(message.value-63)/63.0f;
                     
						if(x_2==TRUE){
							low_c9=2.0*(message.value-63.0)/63.0f;
							}
						 
						if(x_5==TRUE){
							low_c9=5.0*(message.value-63.0)/63.0f;
							}
						if(x_10==TRUE){
							low_c9=10.0*(message.value-63.0)/63.0f;
							}	 	 
                    }
                    
                    if(control_switch==2){
						mid_c9=(message.value-63)/63.0f;
                     
						if(x_2==TRUE){
							mid_c9=2.0*(message.value-63.0)/63.0f;
							}
						 
						if(x_5==TRUE){
							mid_c9=5.0*(message.value-63.0)/63.0f;
							}
						if(x_10==TRUE){
							mid_c9=10.0*(message.value-63.0)/63.0f;
							}	 	 
                    }
                    
                    if(control_switch==3){
						high_c9=(message.value-63)/63.0f;
                     
						if(x_2==TRUE){
							high_c9=2.0*(message.value-63.0)/63.0f;
							}
						 
						if(x_5==TRUE){
							high_c9=5.0*(message.value-63.0)/63.0f;
							}
						if(x_10==TRUE){
							high_c9=10.0*(message.value-63.0)/63.0f;
							}	 	 
                    }
                   
                }
                
                 //c10 maps to fb0 y displace
                if(message.control==121){
					if(control_switch==0){
						p_lock[9][p_lock_increment]=(message.value-63)/63.0f;
                     
						if(y_2==TRUE){
							p_lock[9][p_lock_increment]=2.0*(message.value-63.0)/63.0f;
						 }
						 
						if(y_5==TRUE){
							p_lock[9][p_lock_increment]=5.0*(message.value-63.0)/63.0f;
						 }
						if(y_10==TRUE){
							p_lock[9][p_lock_increment]=10.0*(message.value-63.0)/63.0f;
						 }	 	 
                     }
                     
                     if(control_switch==1){
						low_c10=(message.value-63)/63.0f;
                     
						if(y_2==TRUE){
							low_c10=2.0*(message.value-63.0)/63.0f;
						 }
						 
						if(y_5==TRUE){
							low_c10=5.0*(message.value-63.0)/63.0f;
						 }
						if(y_10==TRUE){
							low_c10=10.0*(message.value-63.0)/63.0f;
						 }	 	 
                     }
                     
                     if(control_switch==2){
						mid_c10=(message.value-63)/63.0f;
                     
						if(y_2==TRUE){
							mid_c10=2.0*(message.value-63.0)/63.0f;
						 }
						 
						if(y_5==TRUE){
							mid_c10=5.0*(message.value-63.0)/63.0f;
						 }
						if(y_10==TRUE){
							mid_c10=10.0*(message.value-63.0)/63.0f;
						 }	 	 
                     }
                     
                     if(control_switch==3){
						high_c10=(message.value-63)/63.0f;
                     
						if(y_2==TRUE){
							high_c10=2.0*(message.value-63.0)/63.0f;
						 }
						 
						if(y_5==TRUE){
							high_c10=5.0*(message.value-63.0)/63.0f;
						 }
						if(y_10==TRUE){
							high_c10=10.0*(message.value-63.0)/63.0f;
						 }	 	 
                     }
                     
                    
                   
                }
                
               
                if(message.control==122){
					if(control_switch==0){
						p_lock[10][p_lock_increment]=(message.value-63.0)/63.0f;
                     
						if(z_2==TRUE){
							p_lock[10][p_lock_increment]=2.0*(message.value-63.0)/63.0f;
						 }
						 
						if(z_5==TRUE){
							p_lock[10][p_lock_increment]=5.0*(message.value-63.0)/63.0f;
						 }
						if(z_10==TRUE){
							p_lock[10][p_lock_increment]=10.0*(message.value-63.0)/63.0f;
						 }	 	 
                   }
                   
                   if(control_switch==1){
						low_c11=(message.value-63.0)/63.0f;
                     
						if(z_2==TRUE){
							low_c11=2.0*(message.value-63.0)/63.0f;
						 }
						 
						if(z_5==TRUE){
							low_c11=5.0*(message.value-63.0)/63.0f;
						 }
						if(z_10==TRUE){
							low_c11=10.0*(message.value-63.0)/63.0f;
						 }	 	 
                   }
                   
                   if(control_switch==2){
						mid_c11=(message.value-63.0)/63.0f;
                     
						if(z_2==TRUE){
							mid_c11=2.0*(message.value-63.0)/63.0f;
						 }
						 
						if(z_5==TRUE){
							mid_c11=5.0*(message.value-63.0)/63.0f;
						 }
						if(z_10==TRUE){
							mid_c11=10.0*(message.value-63.0)/63.0f;
						 }	 	 
                   }
                   
                   if(control_switch==3){
						high_c11=(message.value-63.0)/63.0f;
                     
						if(z_2==TRUE){
							high_c11=2.0*(message.value-63.0)/63.0f;
						 }
						 
						if(z_5==TRUE){
							high_c11=5.0*(message.value-63.0)/63.0f;
						 }
						if(z_10==TRUE){
							high_c11=10.0*(message.value-63.0)/63.0f;
						 }	 	 
                   }
                   
                }
              
                if(message.control==123){
					if(control_switch==0){
						p_lock[11][p_lock_increment]=(message.value-63)/63.0f;
                     
						if(theta_0==TRUE){
							p_lock[11][p_lock_increment]=2*(message.value-63.0)/63.0f;
						 }
						 
						if(theta_1==TRUE){
							p_lock[11][p_lock_increment]=4*(message.value-63.0)/63.0f;
						 }
						if(theta_2==TRUE){
							p_lock[11][p_lock_increment]=8*(message.value-63.0)/63.0f;
						 }	 	
					}   
					
					if(control_switch==1){
						low_c12=(message.value-63)/63.0f;
                     
						if(theta_0==TRUE){
							low_c12=2*(message.value-63.0)/63.0f;
						 }
						 
						if(theta_1==TRUE){
							low_c12=4*(message.value-63.0)/63.0f;
						 }
						if(theta_2==TRUE){
							low_c12=8*(message.value-63.0)/63.0f;
						 }	 	
					}   
					
					if(control_switch==2){
						mid_c12=(message.value-63)/63.0f;
                     
						if(theta_0==TRUE){
							mid_c12=2*(message.value-63.0)/63.0f;
						 }
						 
						if(theta_1==TRUE){
							mid_c12=4*(message.value-63.0)/63.0f;
						 }
						if(theta_2==TRUE){
							mid_c12=8*(message.value-63.0)/63.0f;
						 }	 	
					}   
					
					if(control_switch==3){
						high_c12=(message.value-63)/63.0f;
                     
						if(theta_0==TRUE){
							high_c12=2*(message.value-63.0)/63.0f;
						 }
						 
						if(theta_1==TRUE){
							high_c12=4*(message.value-63.0)/63.0f;
						 }
						if(theta_2==TRUE){
							high_c12=8*(message.value-63.0)/63.0f;
						 }	 	
					}   
                  
                   
                }
                
             
                if(message.control==124){
                    if(control_switch==0){
						p_lock[12][p_lock_increment]=(message.value)/32.0f;
                    
						if(huexx_0==TRUE){
							p_lock[12][p_lock_increment]=message.value/64.0f;
						 }
						 
						if(huexx_1==TRUE){
							p_lock[12][p_lock_increment]=message.value/96.0f;
						 }
						if(huexx_2==TRUE){
							p_lock[12][p_lock_increment]=message.value/127.0f;
						 }	 
					}
					
					if(control_switch==1){
						low_c13=(message.value)/32.0f;
                    
						if(huexx_0==TRUE){
							low_c13=message.value/64.0f;
						 }
						 
						if(huexx_1==TRUE){
							low_c13=message.value/96.0f;
						 }
						if(huexx_2==TRUE){
							low_c13=message.value/127.0f;
						 }	 
					}
					
					if(control_switch==2){
						mid_c13=(message.value)/32.0f;
                    
						if(huexx_0==TRUE){
							mid_c13=message.value/64.0f;
						 }
						 
						if(huexx_1==TRUE){
							mid_c13=message.value/96.0f;
						 }
						if(huexx_2==TRUE){
							mid_c13=message.value/127.0f;
						 }	 
					}
					
					if(control_switch==3){
						high_c13=(message.value)/32.0f;
                    
						if(huexx_0==TRUE){
							high_c13=message.value/64.0f;
						 }
						 
						if(huexx_1==TRUE){
							high_c13=message.value/96.0f;
						 }
						if(huexx_2==TRUE){
							high_c13=message.value/127.0f;
						 }	 
					}
                }
              
            
                if(message.control==125){
					if(control_switch==0){
						p_lock[13][p_lock_increment]=(message.value-63.0)/63.0f;
                     
						if(huexy_0==TRUE){
							p_lock[13][p_lock_increment]=2*(message.value-63.0)/63.0f;
						 }
						 
						if(huexy_1==TRUE){
							p_lock[13][p_lock_increment]=4*(message.value-63.0)/63.0f;
						 }
						if(huexy_2==TRUE){
							p_lock[13][p_lock_increment]=8*(message.value-63.0)/63.0f;
						 }	 
                     }
                     
                     if(control_switch==1){
						low_c14=(message.value-63.0)/63.0f;
                     
						if(huexy_0==TRUE){
							low_c14=2*(message.value-63.0)/63.0f;
						 }
						 
						if(huexy_1==TRUE){
							low_c14=4*(message.value-63.0)/63.0f;
						 }
						if(huexy_2==TRUE){
							low_c14=8*(message.value-63.0)/63.0f;
						 }	 
                     }
                     
                     if(control_switch==2){
						mid_c14=(message.value-63.0)/63.0f;
                     
						if(huexy_0==TRUE){
							mid_c14=2*(message.value-63.0)/63.0f;
						 }
						 
						if(huexy_1==TRUE){
							mid_c14=4*(message.value-63.0)/63.0f;
						 }
						if(huexy_2==TRUE){
							mid_c14=8*(message.value-63.0)/63.0f;
						 }	 
                     }
                     
                     if(control_switch==3){
						high_c14=(message.value-63.0)/63.0f;
                     
						if(huexy_0==TRUE){
							high_c14=2*(message.value-63.0)/63.0f;
						 }
						 
						if(huexy_1==TRUE){
							high_c14=4*(message.value-63.0)/63.0f;
						 }
						if(huexy_2==TRUE){
							high_c14=8*(message.value-63.0)/63.0f;
						 }	 
                     }
                     
                     
                    
                   
                }
                
               
                if(message.control==126){
					if(control_switch==0){
						p_lock[14][p_lock_increment]=(message.value-63.0)/63.0f;
                     
						if(huexz_0==TRUE){
							p_lock[14][p_lock_increment]=2*(message.value-63.0)/63.0f;
						}
						 
						if(huexz_1==TRUE){
							p_lock[14][p_lock_increment]=4*(message.value-63.0)/63.0f;
						}
						if(huexz_2==TRUE){
							p_lock[14][p_lock_increment]=8*(message.value-63.0)/63.0f;
						}	 
                     }
                     
                     if(control_switch==1){
						low_c15=(message.value-63.0)/63.0f;
                     
						if(huexz_0==TRUE){
							low_c15=2*(message.value-63.0)/63.0f;
						}
						 
						if(huexz_1==TRUE){
							low_c15=4*(message.value-63.0)/63.0f;
						}
						if(huexz_2==TRUE){
							low_c15=8*(message.value-63.0)/63.0f;
						}	 
                     }
                     
                     if(control_switch==2){
						mid_c15=(message.value-63.0)/63.0f;
                     
						if(huexz_0==TRUE){
							mid_c15=2*(message.value-63.0)/63.0f;
						}
						 
						if(huexz_1==TRUE){
							mid_c15=4*(message.value-63.0)/63.0f;
						}
						if(huexz_2==TRUE){
							mid_c15=8*(message.value-63.0)/63.0f;
						}	 
                     }
                     
                     if(control_switch==3){
						high_c15=(message.value-63.0)/63.0f;
                     
						if(huexz_0==TRUE){
							high_c15=2*(message.value-63.0)/63.0f;
						}
						 
						if(huexz_1==TRUE){
							high_c15=4*(message.value-63.0)/63.0f;
						}
						if(huexz_2==TRUE){
							high_c15=8*(message.value-63.0)/63.0f;
						}	 
                     }
                    
                   
                }
                
                
                if(message.control==127){
                   if(control_switch==0){
						p_lock[15][p_lock_increment]=(message.value)/127.0;
                   }
                   
                   if(control_switch==1){
						low_c16=(message.value)/127.0;
                   }
                   
                   if(control_switch==2){
						mid_c16=(message.value)/127.0;
                   }
                   
                    if(control_switch==3){
						high_c16=(message.value)/127.0;
                   }
                }
                
                
                
                
                //gots to remap these to 60-61
                 //cc43 maps to brightInvert
                if(message.control==60){
					if(message.value==127){
						brightInvert=TRUE;
					}
					
					if(message.value==0){
						brightInvert=FALSE;
					}
                }
                
                //cc44 maps to saturationInvert
                if(message.control==61){
					if(message.value==127){
						saturationInvert=TRUE;
					}
					
					if(message.value==0){
						saturationInvert=FALSE;
					}
                }
                
               if(message.control==62){
					if(message.value==127){
						//hueInvert=TRUE;
						hdmi_aspect_ratio_switch=TRUE;
					}
					
					if(message.value==0){
						//hueInvert=FALSE;
						hdmi_aspect_ratio_switch=FALSE;
					}
                }
                
                 
                //cc41 maps to horizontalMirror
                if(message.control==41){
					if(message.value==127){
						horizontalMirror=TRUE;
					}
					
					if(message.value==0){
						horizontalMirror=FALSE;
					}
                }
                
                 //cc45 maps to horizontalMirror
                if(message.control==45){
					if(message.value==127){
						verticalMirror=TRUE;
					}
					
					if(message.value==0){
						verticalMirror=FALSE;
					}
                }
                if(message.control==59){
					
					x_skew=y_skew=0.0;
					low_c1=low_c2=low_c3=low_c4=low_c5=low_c6=low_c7=low_c8=low_c9=low_c10=low_c11=low_c12=low_c13=low_c14=low_c15=low_c16=0.0f;
					mid_c1=mid_c2=mid_c3=mid_c4=mid_c5=mid_c6=mid_c7=mid_c8=mid_c9=mid_c10=mid_c11=mid_c12=mid_c13=mid_c14=mid_c15=mid_c16=0.0f;
					high_c1=high_c2=high_c3=high_c4=high_c5=high_c6=high_c7=high_c8=high_c9=high_c10=high_c11=high_c12=high_c13=high_c14=high_c15=high_c16=0.0f;
                }
            }			
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}
