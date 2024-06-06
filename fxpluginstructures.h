/**
    Header file for c++ structures used by VIDIFOLD plugins

    These structures have been in use since 2014 and evolved during development.
    The initial public release is therefore a few versions old.

    There are quite a few un-supported options left in here, some may become
    supported, others may never be supported. This is still a work in process.

    Author: John Day (contact@vidifold.com)
    Structure Version: 9
    File Release:

    6/05/24 Uploaded to github

    12/04/23 v4
    - added extra buffer and bpm
    - added DepthID (depth option) to requestedBuffers

    08/12/22 v3
        reassigned int textureSourceCount (never used) to indicate special texture source requests
        - int specialTexturesRequested

    --/--/-- v2
        FXP_SELECTOR enum should not have been marked deprecated!

    14/12/20 v1

    Some notes on the structures used:

    FXPT_           : bitwise flags indicating the type of plugin

	FXINFO          : basic details describing the plugin
                      includes the canonicalName (IMPORTANT NOTE: this must not change once in use)

    FXPARAMTYPE     : used for defining the type if interface the parameter uses
	FXPARAM         : the definition of a plugin parameter

	FXSHADERPARAMS  : defines type of param passed to shader

    FXSTATE         : used to pass/save/load setups between host and plugin

	FXSHADER        : holds shader type,details and code

    FXSOURCE        : details of sources and their state, also see extended notes next to FXOBJECT struct declaration

    FXBUFFERDETAILS : most details here are set by the host for output and spare buffers

    FXAUDIODETAILS  : lowfi, quick and dirty details

    FXDATABUFFER    : not supported

    FXOBJECT        : ** the shared object between host and plugin, incorporating the above structures **
*/


#ifndef FXPLUGINSTRUCTURES_H_INCLUDED
#define FXPLUGINSTRUCTURES_H_INCLUDED

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <glatter.h>    // the opengl loading library I've chosen to use (https://github.com/imakris/glatter)
#include <gl.h>              // gl.h we need OpenGL :-)
#include <glx.h>             // this file is needed for X11 applications if we want to use hardware rendering

#define MAXFXPARAMS 128         // the maximum parameter controls (ie in the front end via the plugin panel GUI)
#define MAXFXSHADERS 128
#define MAXFXSHADERPARAMS 128
// currently max of 10 required,
// .. but in theory you could have plugins that chain together and add their own sources for the next plugin
// .. so I'm setting this to a random max of 32 (128 seems way to much, very much doubt more then 10 will ever be required)
#define MAXFXSOURCES 32
#define MAXFXFBOBUFFERS 3

using namespace std;

#define FXPT_EFFECT (1<<0)  // normal default plugin which effects its inputs
#define FXPT_SOURCE (1<<1)  // can be used as a source
#define FXPT_MIXER  (1<<2)  // can be used as mixer on bus/output
#define FXPT_EVENT  (1<<3)  // produces output other than visual (not supported)
#define FXPT_AUDIO  (1<<4)  // requires audio feed

#define TEXREQ_NONE (1<<0)          // cant use 1 without rebuilding all the plugins
#define TEXREQ_WHITE_NOISE (1<<1)   // supply this texture in bufferA
#define TEXREQ_PERLIN_NOISE (1<<2)
#define TEXREQ_BLUE_NOISE (1<<3)

struct FXINFO {
    int fxplugintype;                       // bitfield of FPT_ added v2

    string canonicalName;                   // hidden canonical name (IMPORTANT NOTE: this must not change once in use)

	string name;                            // visible name
	string description;                     // shows up in the plugin dialog
	string tags;                            // used to be just author, comma seperated labels will become filter options in later releases
                                            // please DONT USE 'VF' this is the default plugins tag
	float version;                          // your current version of the plugin

	unsigned int paramCount;                // number of FX Parameters
	unsigned int shaderCount;               // number of shaders supplied
	unsigned int fboCount;                  // requested extra FBOs (NOTE: there are always 4 available in FXOBJECT output/bufferA/bufferB/bufferC)
                                            // ** please use those first before requesting another **

	int specialTexturesRequested;           // bitfield for special textures requested from host

	float fps;                              // set by source plugins
	long totalFrames;                       // set by source plugins (if left as 0 then sourcePlaybackPrecentage is used as offset distance from now)
};

// used for defining the plugin interface
enum FXPARAMTYPE {
	FXP_RANGE,              // a bar control
	FXP_BENDER,             // delta -100 > 100
	FXP_TOGGLE,             // boolean on/off button
	FXP_COLUMN_BREAK,       // use to make the next parameter start at the top of the next column in the plugin interface
	FXP_SELECTOR,           // use to list a selection of options
	FXP_NONE,               // use to leave a blank row in the plugin interface
	FXP_MULTI_STATE,        // button that can step through range of options
	FXP_TRIGGER,            // momentary button
	FXP_LABEL,              // just a label
	FXP_TEXTENTRY,          // a button to trigger a text entry dialog
	FXP_FONTSELECTOR,       // a button to trigger a font selection dialog
	FXP_SOURCESELECTOR,     // enables selected of available buffers
	FXP_COLOURSELECTOR,     // not supported
	FXP_JOYSTICK,           // not supported
	FXP_FILESELECTOR,       // a button to trigger a file selection dialog,
                            // file selector, displayvalue from plugin is folder to start from, on selected file, display becomes fullpath/filename and after processing needs to be reset to folder
    FXP_BEATSELECTOR
};


struct FXPARAM {
	FXPARAMTYPE t;
	bool global;              		// not supported, true if this is a global parameter across all instances

	string name;              		// this is the default label display (unless overriden with the displayValue below)
	long minValue,maxValue;    		// as the names suggests
	long defaultValue;        		// resets to this value
	long curValue;             		// actual value

	long deltaValue;           		// when using FXP_BENDER this indicates the current offset amount (-100 > 100)

	bool update;	            	// set by host before process,
									// can be cleared or set by client during process,
									// if set by client should be cleared by host after process

	bool reset;              		// flag reset to defaultValue, set in host, cleared in client

	string displayValue;      		// text version of curValue (for FXP_TEXTENTRY this is also used to pass text data)
	bool displayValueUpdate;  		// set by client, cleared by host

	vector<string>options;			// the displable list of all possible options for an FXP_SELECTOR type
};

struct FXSHADERPARAMS {
	int type;			// 0-int, 1-float
	string name;		// as defined in shader
	float value;		// will be converted to int if required
	int id;				// set by host
};

// orginally just used the host to capture/reset parameter states, but this proved
// limiting for advanced plugins, therefore plugins now encode/decode their state
// for host snapshots/cloning etc...
struct FXSTATE{
	int version;    // the version of this savestate blob (plugins should allow migration of prev versions of any saved states)
	int size;
	void *data;
};

struct FXSHADER {
	int t;						// 0-vert, 1-frag, 2-program
	string text;				// shader code (requires line breaks)
	string vertShaderName;		// set for vert or program
	string fragShaderName;		// set for frag or program
	string programShaderName;	// set for program
	int paramCount;
	FXSHADERPARAMS params[MAXFXSHADERPARAMS];
	bool error;
	int id;						// set by host
};

struct FXSOURCE {
	GLuint id;					    // set by host - texture object
	float tx2;                      // if < 1.0 then definitely source texture so w,h can be used to calc texcel sizes
	float ty2;                      // if 1.0 then is prob fixed envi buffer size (so take w/h from outputBuffer)
                                    // (might not be if source size is poweroftwo, but dont want to up the fxSystemVersion again at this time)
	// NOTE: w,h,d are set to 0 if this is the source plugin rendering as a source (ie in fxindex 0)
	unsigned int w;                 // this is sourceTexture width (was going to be orginal raw source details, now reassigned)
	unsigned int h;                 // this is sourceTexture height
	unsigned int d;                 // this is sourceTexutre depth RGB = 3 or RGBA = 4

	// also includes any temp multiplication
	double startTimestamp;			// set on play trigger
	double length;					// 0 if not a video (else length in ns in default FPS)
	float playbackPrecentage;     	// -1 if not avail/known (or some scratch distance if source plugin and no totalFrames value)
	double effectStartTimestamp;	// set on effect trigger, when fx is started this should match the curTime (when converted)

	long frameCount;                // if video source else 0
	long frame;                     // if video source else 0
	float timebase;                 // if video source else 0
	float localSpeed;               // if video source else 0
	bool reversed;                  // if video source else 0

	bool paused;                    // paused state
};

// most details here are set by the host for output and spare buffer
struct FXBUFFERDETAILS {
	GLuint FBOID;					// frame buffer object ID
	GLuint TextureID;               // set by host on success
	GLuint DepthID;                 // set to non-zero to request fbo with depth buffer, will return depthTextureID on success
	GLenum status;    				// error if ! GL_FRAMEBUFFER_COMPLETE_EXT
	unsigned int width, height;  	// set by plugin for requested buffers (leave as 0 to use envi default)
	unsigned int sqsize;            // power2 texture size eg. 256/512/1024...
	float orthRatioX,orthRatioY;    //(1.0f/actualWidth),(1.0f/actualHeight)
	int fboBufferSize;
};

// latest captured and processed audio data
// this is pretty low quality (fast and dirty)
struct FXAUDIODETAILS {
    bool active;                // indicates if audio capture devices has been started
    double lastUpdateTime;
    int bytes;                  // between 0>4096 (ie 2bytes per channel * 2channels * upto 1024 buffer)
    void* dataBuffer;           // buffer of signed int16 data (-32,768 and +32,767) (2 bytes left value,2 bytes right value, repeat)
    bool lowFilterState;        // current filter triggered states as seen on front end
    bool midLFilterState;
    bool midHFilterState;
    bool highFilterState;
};

// used to pass data between instances OR across plugins
struct FXDATABUFFER {
	string source;
	string identifier;
	void *data;
};

/**
    NOTES ON FXSOURCE[]

	MIXER PLUGINS ONLY ACTIVE IN BUS OR OUTPUT SLOTS
	if its a mixer plugin and this is not a bus or output then it gets bypassed

	Host supplies sources in fixed order

	0 - standard default source input from fxchain for this fx stage

	* 1 & 6 are only available when the plugin is in an output slot *

	1 - bus A (if available)
	2-5 bus layers top>bot (if in single mode only top layer (2) is supplied)

	6 - bus B (if available)
	7-10 - bus layers top>bot (if in single mode only top layer (7) is supplied)

	the id will be 0 if the source is unavailable
	NOTE: local rendering of bus layers comes out bot to top,
	but for plugins we want it to look like 1>4 (top to bot) order so we swizzle it here
*/
struct FXOBJECT {
	FXINFO info;                                          	// plugin details

	FXPARAM interfaceparams[MAXFXPARAMS];                 	// interface params
	FXSTATE state;											// used to pass snapshot of state between host/client
															// managed by client
	FXSHADER shaders[MAXFXSHADERS];                       	// created/destroyed by host
	FXSOURCE source[MAXFXSOURCES];	                        // running-sources (see notes above)
	FXBUFFERDETAILS requestedBuffers[MAXFXFBOBUFFERS];    	// created/destroyed by host

	bool error;                                             // set be plugin code on error
	string errorMessage;

	bool bypass;	// set by plugin (if process/render will/did not do anything)

	// set by host
	FXBUFFERDETAILS outputBuffer;						// default dest output
	FXBUFFERDETAILS bufferA;							// reuseable host buffer (if need own storage use requestedBuffers)
	FXBUFFERDETAILS bufferB;
	FXBUFFERDETAILS bufferC;
	FXAUDIODETAILS audioData;

	// current host state
	struct timespec curTime;
	float globalSpeed;                  // percent (should default to 1 if not used) (globalPause sets this to 0)
	bool globalReverse;
	double hemidemisemiQuaverCount;		// 0>256
	float bpm;
	unsigned int bar;                   // 0>64 (same as max tape length)
	int displayWidth;                   // output window width in pixels
	int displayHeight;                  // output window height in pixels

	float pluginLevel;                  // 0-1 dry-wet
	int pluginPos;                      // seq pos 0-7
	float mutableLevel;                 // mostly used for randomess option

	void *rawSource;			// not supported, this will be the orginal source at start of chain
	unsigned int rawSourceW;    // not supported
	unsigned int rawSourceH;    // not supported
	unsigned int rawSourceD;    // not supported

	vector<FXDATABUFFER*> dataBuffers;	//not supported, shared accross plugin chain via host
};

#endif // FXPLUGINSTRUCTURES_H_INCLUDED
