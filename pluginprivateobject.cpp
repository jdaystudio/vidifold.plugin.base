/**
    This is part of the example/base plugin files for creating FXPlugins for VIDIFOLD.
    see README.md for further notes.

    Author: John Day (contact@vidifold.com)
    File Release: 14/12/20 v1
    Uploaded to github: 06/05/24 v2

    Supplied files:

    fxpluginstructures.h
    main.cpp
    pluginprivateobject.cpp     (THIS ONE)
    pluginprivateobject.h

    You also need for this example:
    1: REQUIRED: the opengl loading library I've chosen to use can be sourced from https://github.com/imakris/glatter
    2: OPTIONAL: for the advance functions I chose https://github.com/g-truc/glm
*/

#include "pluginprivateobject.h"

#include <sstream>
#include <cmath>

/**
these some optional includes for the more modern shader setups
NOT included in this simple example plugin (https://github.com/g-truc/glm)

#include <glm/gtc/matrix_transform.hpp>     // ortho
#include <glm/gtx/transform.hpp>            // for translate
#include <glm/gtc/quaternion.hpp>           // for rotation
#include <glm/gtx/quaternion.hpp>
*/

PluginPrivateObject::PluginPrivateObject() {
	firstRun = true;
    FPns = floor((double)1E9 / (double)25);  // defaulting speed to 25fps
    resetTriggered = false;
    srand (time(NULL));     // initialize random seed


    // some common values and lists I use
    //PI = 3.14159265359;
    //PI180 = 3.14159265359 / 180.;
    //PIHALF = PI / 2.0;

    /*sourceLabels[0] = "Source In";
    sourceLabels[1] = "A Output";
    sourceLabels[2] = "A Top Layer";
    sourceLabels[3] = "A Upper";
    sourceLabels[4] = "A Lower";
    sourceLabels[5] = "A Bottom Layer";
    sourceLabels[6] = "B Output";
    sourceLabels[7] = "B Top Layer";
    sourceLabels[8] = "B Upper";
    sourceLabels[9] = "B Lower";
    sourceLabels[10] = "B Bottom Layer";

    beatLabels[0] = "1/16";
    beatLabels[1] = "1/8";
    beatLabels[2] = "1/4";
    beatLabels[3] = "1/2";  // default
    beatLabels[4] = "1";
    beatLabels[5] = "2";
    beatLabels[6] = "4";
    beatLabels[7] = "8";
    beatLabels[8] = "16";

    hdsqDelays[0] = 16;     // 1/16
    hdsqDelays[1] = 32;     // 1/8
    hdsqDelays[2] = 64;     // 1/4
    hdsqDelays[3] = 128;    // 1/2
    hdsqDelays[4] = 256;    // 1 bar
    hdsqDelays[5] = 512;    // 2 bar
    hdsqDelays[6] = 1024;   // 4 bar
    hdsqDelays[7] = 2048;   // 8 bar
    hdsqDelays[8] = 4096;   // 16 bar*/

    /*channelLabels[0] = "RGB";
    channelLabels[1] = "R--";
    channelLabels[2] = "-G-";
    channelLabels[3] = "--B";
    channelLabels[4] = "RG-";
    channelLabels[5] = "R-B";
    channelLabels[6] = "-GB";*/
}

PluginPrivateObject::~PluginPrivateObject() {
	// empty
}

/**
    helper for creating interface parameter
*/
void PluginPrivateObject::CreateParam(FXPARAMTYPE t,std::string name,int minValue,int maxValue,int currentValue,int defaultValue) {
	fx->interfaceparams[fx->info.paramCount].t = t;
	fx->interfaceparams[fx->info.paramCount].name.assign(name);
	fx->interfaceparams[fx->info.paramCount].minValue = minValue;
	fx->interfaceparams[fx->info.paramCount].maxValue = maxValue;
	fx->interfaceparams[fx->info.paramCount].curValue = currentValue;
	fx->interfaceparams[fx->info.paramCount].defaultValue = defaultValue;
	fx->interfaceparams[fx->info.paramCount].global = false;// not supported
	fx->interfaceparams[fx->info.paramCount].update = false;
	if (FXP_LABEL == t) fx->interfaceparams[fx->info.paramCount].displayValue = name;
	fx->info.paramCount++;
};
/**
    helper for creating shader parameter
*/
void PluginPrivateObject::CreateShaderParam(int shaderID,int t,std::string name,float value) {
	int i = fx->shaders[shaderID].paramCount;
	fx->shaders[shaderID].params[i].type = t;
	fx->shaders[shaderID].params[i].name = name;
	fx->shaders[shaderID].params[i].value = value;
	fx->shaders[shaderID].paramCount++;
};
//----------------------------------------------------------------------------
/**
    Shaders with same name are only created once for all instances
    there are a couple default host shaders available

    vert shaders
        000-0TextureVert (just gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex)
        000-1TextureVert (also sets gl_TexCoord[0] = gl_MultiTexCoord0)
        000-2TextureVert (like 000-1 but includes gl_TexCoord[1] = gl_MultiTexCoord1)
        000-3TextureVert (as above, cont)
        000-4TextureVert (as above, cont)

        000-1TextureVert330 (used by example plugin)

    frag shaders
        nothing worth reusing at this time
        except to note the names are prepended with 000- or 001-
 */
void PluginPrivateObject::CreateShaders() {
    unsigned int id = 0;

	fx->shaders[id].t = 1;  // 0-vert, 1-frag, 2-program
	fx->shaders[id].text =
        "#version 120\n"

		"uniform sampler2D tex0;\n"
		"uniform float i[1];\n"

		"void main(){\n"
		"  vec4 c = texture2D(tex0,gl_TexCoord[0].st);\n" // #120
        "  gl_FragColor = vec4(c.r * i[0], c.g, c.b , c.a);\n"

		/**
		// a more modern example
		"#version 330\n"
		"in vec2 UV;\n"            // from out vec2 texCoord in vertshader above
		"out vec4 color;\n"         // holds final fragment colour
		" vec2 resolution = vec2(i[1],i[2]);\n"
		" vec2 cord = gl_FragCoord.xy / resolution.xy;\n"
		"  uv.x *= i[3];\n"									// scale to texture edges (not nessarly full 1.0!)
		"  uv.x = mod((mod(uv.x,i[3])+i[3]),i[3]);\n" // roll around 0-tx
		"  uv.y *= i[4];\n"
		"  uv.y = mod((mod(uv.y,i[4])+i[4]),i[4]);\n" // roll around 0-ty
		"  gl_fragColor = texture2D(tex0, uv);\n"
		*/


		"}\n";
	fx->shaders[id].vertShaderName = "";                                // not set for frag
	fx->shaders[id].fragShaderName = "000-VIDIFOLD-DEMO-TextureFrag";   // not set for vert
	fx->shaders[id].programShaderName = "";                             // only set for program
	fx->shaders[id].paramCount = 0;
	fx->shaders[id].error = false;	// set by host program
	fx->shaders[id].id = 0;	// set by host program
	fx->info.shaderCount++;

    id++;
	fx->shaders[id].t = 2;
	fx->shaders[id].text = "";
	fx->shaders[id].vertShaderName = "000-1TextureVert"; //*** or  "000-1TextureVert330"; // for version 330
	fx->shaders[id].fragShaderName = "000-VIDIFOLD-DEMO-TextureFrag";
	fx->shaders[id].programShaderName = "000-VIDIFOLD-DEMO-Shader";
	fx->shaders[id].paramCount = 0;
	//CreateShaderParam(id,0,"MVP",0.0f);	// v330 the 2nd and 4th params dont mean anything here
	CreateShaderParam(id,0,"tex0",0.0f);
	CreateShaderParam(id,1,"i",0.0f);
	fx->shaders[id].error = false;	// set by host program
	fx->shaders[id].id = 0;	// set by host program
	fx->info.shaderCount++;
}

void PluginPrivateObject::InitPlugin() {
    fx->info.fxplugintype = FXPT_EFFECT;
    fx->info.canonicalName = "000-VIDIFOLD-DEMO";  // MUST NEVER CHANGE!
	fx->info.name = "000";
	fx->info.description = "Default plugin project";
	fx->info.tags = "VF";
	fx->info.version = 0.1;
	fx->info.paramCount = 0;
	fx->info.shaderCount = 0;
	//fx->info.specialTexturesRequested = TEXREQ_NONE; // replaced with
	fx->info.specialTexturesRequested = TEXREQ_NONE;
	fx->info.fboCount = 0;
    fx->info.fps = 25;
    fx->info.totalFrames = 0; // na


    //NOTE: requested size is videoTexture buffers size, so sqsize should also be the same power of 2
    // ie. both are 1024 and so tx2,ty2 is also 1.0
    //**** NOTE: remove this if not using so easier to grep
    //fx->info.fboCount = 1;
    //fx->requestedBuffers[0].width = 0;    // default host envi size please (MAXFXFBOBUFFERS = 3)
    //fx->requestedBuffers[0].height = 0;
    //fx->requestedBuffers[0].DepthID = 0;

	CreateParam(FXP_RANGE,"Red",0,100,0,0);
	// example of a multistate button
	//CreateParam(FXP_MULTI_STATE,dirLabel[dir],0,3,dir,dir);  // forward/backward/bothward
	//CreateParam(FXP_COLOURSELECTOR,"Tint",0,0,0,0xFFFFFFFF);

	CreateShaders();

	fx->bypass = false;
	fx->outputBuffer.FBOID = 0;
	fx->outputBuffer.TextureID = 0;
	fx->outputBuffer.width = 256;
	fx->outputBuffer.height = 256;
	fx->bufferA.FBOID = 0;
	fx->bufferA.TextureID = 0;
	fx->bufferA.width = 256;
	fx->bufferA.height = 256;
	fx->bufferB.FBOID = 0;
	fx->bufferB.TextureID = 0;
	fx->bufferB.width = 256;
	fx->bufferB.height = 256;

	fx->error = false;
	fx->errorMessage = "";


    localPrevTime = (fx->curTime.tv_sec *1E9) + fx->curTime.tv_nsec;
    srand(time(0));

	Reset();

    // init vbo
	//glGenBuffers(1,&sqvertsVBO);
	//glGenBuffers(1,&squvsVBO);
};

void PluginPrivateObject::Deinit() {
    /**
        //a more modern example
        glDeleteBuffers(1,&squvsVBO);
        glDeleteBuffers(1,&sqvertsVBO);
    */
}

/**
    This result from this will be stored by VIDIFOLD

    MOST of the time its easier to use the interface state,
    as it can be differcult to reverse the algos
*/
void PluginPrivateObject::GetState() {
    // NOTE: snap is is passed to host to deal with (DO NOT delete/free this in CLIENT)
    PluginPrivateState* snapForHOST = new PluginPrivateState;
	snapForHOST->r = fx->interfaceparams[0].curValue;

	fx->state.version = 1; // dont forget to update this with current state version
	fx->state.size = sizeof(PluginPrivateState);
	fx->state.data = (void*) snapForHOST;
};

void PluginPrivateObject::nonUpdateOnParams(){
    //fx->interfaceparams[5].update = false; // eg dont want to trigger a button
}

/**
    When recalling a setup (patch/snapshot/interlude..etc)
    VIDIFOLD will send over a state that was prev capture from the GetState function

    If you update/change your state after release, you should expect and handle
    older states being supplied
*/
bool PluginPrivateObject::SetState(FXSTATE* fxstate) {
    /**
        //possible example of how to handle an state migration
        PluginPrivateState* localState = new PluginPrivateState;

        if (fxstate->version == 1) {
            _migrateState1(fxstate, localState);
            _useState2(fxstate, localState);
        }else if (fxstate->version == 2){
            _useState2(fxstate,0);
        }else {
            return false;
        }

        return true;
    */

	if (fxstate->version == 1) {

        PluginPrivateState* localState = new PluginPrivateState;

		localState = (PluginPrivateState*)fxstate->data;

		fx->interfaceparams[0].curValue = localState->r;

		// now upate / convert values locally
		for(unsigned int i=0; i<fx->info.paramCount; i++) fx->interfaceparams[i].update = true;
        nonUpdateOnParams();
        // NOTE: dont force resetTriggered here (causes problems)
		Update();
		// Host is responsible for panel update in this case
		return true;
	}

	return false;
};

/**
    possible example of how to handle an state migrations
// copy old state value across and add new defaults
// fxstate required to update version, this fills localstate
void PluginPrivateObject::_migrateState1(FXSTATE* fxstate, PluginPrivateState* localState){
    PluginPrivateState_v1* snap_v1 = (PluginPrivateState_v1*)fxstate->data;

    localState->mode = snap_v1->mode;
    localState->scale = snap_v1->scale;
    localState->freq = snap_v1->freq;

    // add new param default
    localState->mirror = 0;

    fxstate->version = 2;
}
//if local state passed, then it holds the newly migrated details
void PluginPrivateObject::_useState2(FXSTATE* fxstate, PluginPrivateState* localState){
    // didn't have to migrate so grab data
    if (localState == 0){
        localState = (PluginPrivateState*)fxstate->data;
    }

    fx->interfaceparams[0].curValue = localState->mode;
    fx->interfaceparams[1].curValue = localState->scale;
    fx->interfaceparams[2].curValue = localState->freq;

    // now upate / convert values locally
    for(unsigned int i=0; i<fx->info.paramCount; i++) fx->interfaceparams[i].update = true;
    nonUpdateOnParams();
    Update();

    phaseOffset = localState->phaseOffset;
    timepos = localState->timepos;
}
*/


void PluginPrivateObject::Reset() {
    for(unsigned int i=0; i<fx->info.paramCount; i++) {
        fx->interfaceparams[i].curValue = fx->interfaceparams[i].defaultValue;
        fx->interfaceparams[i].update = true;

        if (fx->interfaceparams[i].t == FXP_BENDER){
            fx->interfaceparams[i].reset = true;
        }
    }
    nonUpdateOnParams();

    resetTriggered = true;
    Update();
    resetTriggered = false;
}

void PluginPrivateObject::RandomizeState(){
    resetTriggered = true;
    Random();
    Update();
    resetTriggered = false;
}

void PluginPrivateObject::Random(){
    int p = 0;
	fx->interfaceparams[p].curValue = (rand() %101);
	fx->interfaceparams[p].update = true;

	/**
        p++;
        float r = (rand() % 101) * 0.01;
        float g = (rand() % 101) * 0.01;
        float b = (rand() % 101) * 0.01;
        float a = 1.0;
        if (rand()%101 > 95) a = 0.5 + ((rand() % 50) * 0.01);
        RGBA2Param(r,g,b,a,p);
        fx->interfaceparams[p].update = true;
    */

}

/**
    when plugin is enabled and in use this is always called before Process
    so you can handle any VIDIFOLD flagged updates (ie from the GUI)

    NOTE:
    if we change a parameter internally we need to flag it back for an update on VIDFOLD
    so the GUI can keep in sync

    otherwise set update to false (handled)
*/
void PluginPrivateObject::Update() {
	unsigned int p; // local GUI parameter index

	// sometimes a trigger might require all values to be refresh on host
	// so I tend to use this
	bool forceHostUpdate = false || resetTriggered;

	// range type control
	p = 0;
	if (fx->interfaceparams[p].update) {
		r = (fx->interfaceparams[p].curValue * 0.01);
		fx->interfaceparams[p].update = forceHostUpdate;
	}

	/**
        // toggle type control example
        p = 1;
        if (fx->interfaceparams[p].update) {
            if (fx->interfaceparams[p].curValue == 0){
                effectENABLED = false;
            }else{
                effectENABLED = true;
            }
            fx->interfaceparams[p].update = forceHostUpdate;
        }

        // multiple option example
        p = 1;
        if (fx->interfaceparams[p].update) {
            dir = fx->interfaceparams[p].curValue;
            if (dir > 2) dir = 2;   // this should happen from host, but just incase
            // now feed back to VIDIFOLD that we want the label to change
            fx->interfaceparams[p].displayValue = dirLabel[dir];
            fx->interfaceparams[p].displayValueUpdate = true;
            fx->interfaceparams[p].update = true;
        }

        // NOTE: range types can also have their value label changed

        // beat hit example
        if (fx->interfaceparams[p].update) {
            state.genSpeed = fx->interfaceparams[p].curValue;

            if (state.genSpeed > 8) state.genSpeed = 8;
            fx->interfaceparams[p].displayValue = "On "+beatLabels[state.genSpeed];
            fx->interfaceparams[p].displayValueUpdate = true;

            if (state.genSpeed < 4){
                prevHit = floor(fx->hemidemisemiQuaverCount / hdsqDelays[state.genSpeed]); //beat
            }else{
                prevHit = fx->bar;
            }
        }

        // colour selector example
        p++;
        if (fx->interfaceparams[p].update) {
            Param2RGBA(p,r,g,b,a);
            fx->interfaceparams[p].update = forceHostUpdate;
        }

        // bender example
        p++;
        if (fx->interfaceparams[p].update) {
            if (fx->interfaceparams[p].reset){
                FOV = 45.0;
                fx->interfaceparams[p].reset = false;
            }else{
                FOV += (BenderCalc(fx->interfaceparams[p].deltaValue) * 3.);
                if (FOV < 5) FOV = 5.;
                if (FOV > 175) FOV = 175;
            }
            fx->interfaceparams[p].update = false;
        }
	*/
};

/**
    Call in each frame if plugin is in use and enabled
*/
void PluginPrivateObject::Process() {

	if (fx->error) {
		fx->bypass = true;  // if for any reason the plugin should not be used then set this flag
		return;
	}

	// optional
	ProgressTime();

	// first run
	if (firstRun){
        //fx->outputBuffer.width = actual texture size
        //fx->displayWidth = final output display width
        displayWRatio = ( (float)fx->outputBuffer.width / (float)fx->displayWidth );
        displayHRatio = ( (float)fx->outputBuffer.height / (float)fx->displayHeight);

        /**
		// if using glsl 330, a quad to render
		/*sqverts[0] = glm::vec3(0.0,0.0,0.0);
		sqverts[1] = glm::vec3(0.0,fx->outputBuffer.height,0.0);
		sqverts[2] = glm::vec3(fx->outputBuffer.width,0.0,0.0);
		sqverts[3] = glm::vec3(fx->outputBuffer.width,fx->outputBuffer.height,0.0);

		// if rendering to sq first
        squvs[0] = glm::vec2(0,0);
        squvs[1] = glm::vec2(0,1.0);
        squvs[2] = glm::vec2(1.0,0);
        squvs[3] = glm::vec2(1.0,1.0);
        */

		firstRun = false;
	}

	// effect started
	if (fx->source[0].effectStartTimestamp == GetRealTimestamp()){
        gentimer = fx->source[0].effectStartTimestamp;
        prevbpm = fx->bpm;
        prevHit = 0;
        curHit = 0;
        newHit = false;
	}

	/**
        // if using glsl 330
        // if not relying on sq
        squvs[0] = glm::vec2(0,0);
        squvs[1] = glm::vec2(0,fx->source[0].ty2);
        squvs[2] = glm::vec2(fx->source[0].tx2,0);
        squvs[3] = glm::vec2(fx->source[0].tx2,fx->source[0].ty2);
    */

	// prepare output fbo
	Process120Example();
};

/**
    OLD school state based example

    NOTE: if you change any global states you MUST put them back to default before exiting
    or you can make a right mess of the GUI :)
*/
void PluginPrivateObject::Process120Example(){
    // select the output buffer
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fx->outputBuffer.FBOID);

    // setup viewport
	glViewport(0, 0, fx->outputBuffer.width, fx->outputBuffer.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, (GLfloat)fx->outputBuffer.width,0.0f,(GLfloat)fx->outputBuffer.height,-1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the default source texture comming in
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,fx->source[0].id);

    /*if (mirror){
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT);
    }*/

	// use our shader
    unsigned int id = 1;
	glUseProgram(fx->shaders[id].id);

	glUniform1i(fx->shaders[id].params[0].id, 0);		//tex

	int pcount = 1;
	float i[pcount];
	i[0]=r;

    /**
        example of using more parameters
        i[0] = localtime.tm_sec;
        i[1] = localtime.tm_nsec;
        i[2]=fx->outputBuffer.width;
        i[3]=fx->outputBuffer.height;
        i[4]=fx->source[0].tx2;
        i[5]=fx->source[0].ty2;
	*/

	glUniform1fv(fx->shaders[id].params[1].id, pcount, i);

	DrawQuad(0,0,0,0,fx->outputBuffer.width,fx->outputBuffer.height,fx->source[0].tx2,fx->source[0].ty2,1.0);

	// put it back!
    /*if (mirror){
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    }*/

	// stop using our shader
	glUseProgram(0);
}

/**
    Example of using a more modern approach
    this also has an example of using one of the extra buffers for converting to a square texture

    requires glm headers, not supplied : https://github.com/g-truc/glm
*/
/*
void PluginPrivateObject::Process330Example(){

    // example shader requires square texture
	// if plugin is supplied with a non-square texture, then we need to first
	// create a square one (could prob get around this but out of time for mo)
	GLuint sourceTextureID = fx->source[0].id;

	/*if (fx->source[0].tx2 != 1.0f
			|| fx->source[0].ty2 != 1.0f) {

		glBindFramebuffer(GL_FRAMEBUFFER_EXT, fx->bufferA.FBOID);

		glViewport(0, 0, fx->bufferA.width, fx->bufferA.height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0f, (GLfloat)fx->bufferA.width,0.0f,(GLfloat)fx->bufferA.height,-1.0f,1.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_TEXTURE_2D);

		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glColor4f(1.0f,1.0f,1.0f,1.0f);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,sourceTextureID);

		DrawQuad(0,0,0,0,fx->bufferA.width,fx->bufferA.height,fx->source[0].tx2,fx->source[0].ty2,1.0);

		glBindFramebuffer(GL_FRAMEBUFFER_EXT,0);
		sourceTextureID = fx->bufferA.TextureID;
	}*//*

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, fx->outputBuffer.FBOID);
	glViewport(0, 0, fx->outputBuffer.width, fx->outputBuffer.height);

	//** still valid in openGL 3+ **
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,fx->source[0].id);// or sourceTextureID if squaring

    unsigned int pid = 1;
    glUseProgram(fx->shaders[pid].id);

    // not required at time of writing but keeping as pattern
    /*glm::mat4 myTransform1 = glm::translate(glm::mat4(), glm::vec3(halfWidth,halfHeight , 0.0f));

    glm::vec3 eulerangles = glm::vec3(0.0, 0.0, 0.0);
	glm::quat myQuaternion = glm::quat(eulerangles);
	glm::mat4 myRotationMatrix = glm::toMat4(myQuaternion);

	glm::mat4 myModelMatrix = myTransform1 * myRotationMatrix;*//*

    // init projection matrix (glm::mat4) (could be done first time only)
    float ow = (GLfloat)fx->outputBuffer.width;
    float oh = (GLfloat)fx->outputBuffer.height;
    glm::mat4 projection = glm::ortho(0.0f, ow,0.0f,oh,0.0f,100.0f);

	// no camara or model matrix in this case (else could be ! projection * view * model !)
	glm::mat4 mvp = projection;// * myModelMatrix;


	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
	glUniformMatrix4fv(fx->shaders[pid].params[0].id, 1, GL_FALSE, &mvp[0][0]);
    glUniform1i(fx->shaders[pid].params[1].id, 0);	// texture

	// my frag params
    float coffset = 0.0;
	float s = GetRealTimestamp() / 1e9;     // convert from ns to sec
	float t = roundf((s + coffset) * 100.0) / 100.0;    // round to 2 dp

	unsigned int pcount = 1;
	float i[pcount];
	i[0]= cellwidth;
	glUniform1fv(fx->shaders[pid].params[2].id, pcount   , i);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, sqvertsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sqverts), sqverts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(
		0, 						// attribute (must match layout in shader)
		3, 						// size
		GL_FLOAT, 				// type
		GL_FALSE, 				// normalized?
		0, 						// stride
		0						// array buffer offset
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, squvsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squvs), squvs, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// a quad

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}
*/

/**
    HELPER FUNCTIONS
*/

//example of tracking time passed
void PluginPrivateObject::ProgressTime(){
	double curTime = GetRealTimestamp();

	localElapsedTime = curTime - localPrevTime;

	// default speed of clip is FPns Frames per nano secs
	double thisFramesTravelled = (localElapsedTime / FPns);
	framesTraveled = thisFramesTravelled * fx->globalSpeed;

	localElapsedTime -= (FPns * thisFramesTravelled);

    localPrevTime = curTime - localElapsedTime;

    /*newHit = false;
    //double eightbarmeter = floor((float)fx->bar / 8.0);
    //hemidemisemiQuaverCount = 64th of a beat (4 beats to a bar = 64*4=256)
    //barsTravelled = floor(beats.hemidemisemiQuaverCount / 256.0);
    // hemidemisemiquaver = 64th of a note
    //int semiquaver = floor(fx->hemidemisemiQuaverCount / 16.0); // quaver = 0 to 15 (every 16th note)
    // crotchet = semiquaver % 4;
    //curHit = (semiquaver >> 2);   //4/4

    int hsqc = fx->hemidemisemiQuaverCount;
    if (beatmode >= 4) hsqc += (fx->bar * 256);
    int curHit = floor(hsqc / hdsqDelays[beatmode]); // on beat
    if (curHit != prevHit){
        prevHit = curHit;
        newHit = true;
    }

    if (r){
        cycleOffset -= framesTraveled * cycleSpeed;
        cycleOffset = 100. - (fmod(-cycleOffset,100.));
    }else{
        cycleOffset += framesTraveled * cycleSpeed;
        cycleOffset = fmod(cycleOffset, 100.);
    }
    }*/
};


double PluginPrivateObject::GetRealTimestamp(){
    return (fx->curTime.tv_sec * 1E9)+fx->curTime.tv_nsec;
}

// GL_QUADS is deprecated
void PluginPrivateObject::DrawQuad(float x1, float y1, float tx1, float ty1, float x2, float y2, float tx2, float ty2, float z){
		glBegin(GL_TRIANGLES);
		glTexCoord2f(tx1,ty1);
		glVertex3f(x1, y1, z);
		glTexCoord2f(tx2,ty1);
		glVertex3f(x2,y1, z);
		glTexCoord2f(tx1,ty2);
		glVertex3f(x1,y2, z);

		glTexCoord2f(tx1,ty2);
		glVertex3f(x1, y2, z);
		glTexCoord2f(tx2,ty1);
		glVertex3f(x2,y1, z);
		glTexCoord2f(tx2,ty2);
		glVertex3f(x2,y2, z);
		glEnd();
}

void PluginPrivateObject::DrawQuad(float x1, float y1, float tx1, float ty1, float x2, float y2, float tx2, float ty2){
		glBegin(GL_TRIANGLES);
		glTexCoord2f(tx1,ty1);
		glVertex2f(x1, y1);
		glTexCoord2f(tx2,ty1);
		glVertex2f(x2,y1);
		glTexCoord2f(tx1,ty2);
		glVertex2f(x1,y2);

		glTexCoord2f(tx1,ty2);
		glVertex2f(x1, y2);
		glTexCoord2f(tx2,ty1);
		glVertex2f(x2,y1);
		glTexCoord2f(tx2,ty2);
		glVertex2f(x2,y2);
		glEnd();
}

void PluginPrivateObject::DrawQuad2(float x1, float y1, float tx1, float ty1,float ttx1,float tty1, float x2, float y2, float tx2,float ty2,float ttx2,float tty2){
		glBegin(GL_TRIANGLES);
        glMultiTexCoord2f(GL_TEXTURE0,tx1,ty1);
        glMultiTexCoord2f(GL_TEXTURE1,ttx1,tty1);
		glVertex2f(x1, y1);

        glMultiTexCoord2f(GL_TEXTURE0,tx2,ty1);
        glMultiTexCoord2f(GL_TEXTURE1,ttx2,tty1);
		glVertex2f(x2,y1);

        glMultiTexCoord2f(GL_TEXTURE0,tx1,ty2);
        glMultiTexCoord2f(GL_TEXTURE1,ttx1,tty2);
		glVertex2f(x1,y2);

		glVertex2f(x1, y2);

        glMultiTexCoord2f(GL_TEXTURE0,tx2,ty1);
        glMultiTexCoord2f(GL_TEXTURE1,ttx2,tty1);
		glVertex2f(x2,y1);

        glMultiTexCoord2f(GL_TEXTURE0,tx2,ty2);
        glMultiTexCoord2f(GL_TEXTURE1,ttx2,tty2);
		glVertex2f(x2,y2);
		glEnd();
}


string PluginPrivateObject::IntToString(int value) {
    char buf[50];
    sprintf(buf,"%d",value);
    string v = buf;
    return v;
}
string PluginPrivateObject::DoubleToString(double value) {
    std::stringstream s;
    s << value;
    return s.str();
}
void PluginPrivateObject::Debug(const char* format, ... ) {
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg); //Pay Attantion: vprintf - not printf
	va_end(arg);

	fflush(stdout);
}
void PluginPrivateObject::DumpFBO(string filename,unsigned int w,unsigned int h,GLuint FBOID){
	// debug (dump fbo texture)
	unsigned int buffersize = w*h*4;
	unsigned char *memblock=(unsigned char *)malloc(buffersize);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOID);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)memblock);	// automaticaly drops alpha if not requested (seems to work)
    SaveRawImage(filename,memblock,w,h,4);
    free(memblock);
}
bool PluginPrivateObject::SaveRawImage(string filename,unsigned char *buffer, int width, int height,int bytes) {
    //jbdlog(DEBUGSYS_DEFAULT,"Saving RAW Buffer %s %dx%dx%d\n",filename.c_str(),width,height,bytes);
    FILE *pFile;
    unsigned int buffersize = width * height * bytes;
    pFile=fopen(filename.c_str(), "wb");
    if(pFile==NULL) {
        Debug("Failed to open file for writing %s\n",filename.c_str());
        return false;
    }
    fwrite(buffer,1,buffersize,pFile);
    fclose(pFile);
    return true;
}
void PluginPrivateObject::ClearFBO(GLuint FBOID, int width, int height){
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, FBOID);

    // setup viewport
	glViewport(0, 0, width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, (GLfloat)width,0.0f,(GLfloat)height,-1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER_EXT,0);
}
void PluginPrivateObject::Param2RGBA(int p, float &r, float &g, float &b, float &a){
    unsigned long rgba = fx->interfaceparams[p].curValue;
    //Debug("Plugin->Param2RGBA incomming %u \n",rgba);

    r = (rgba & 0xFF000000) >> 24;
    g = (rgba & 0x00FF0000) >> 16;
    b = (rgba & 0x0000FF00) >> 8;
    a = (rgba & 0x000000FF);

    r /= 255.;
    g /= 255.;
    b /= 255.;
    a /= 255.;
    //Debug("Plugin->Param2RGBA converted %f %f %f %f \n\n",r,g,b,a);
}


void PluginPrivateObject::RGBA2Param(float r,float g, float b, float a, int p){
    unsigned long rgba = RGBA2Int(r,g,b,a);
    fx->interfaceparams[p].curValue = rgba;
}
unsigned long PluginPrivateObject::RGBA2Int(float r,float g, float b, float a){
    unsigned long rgba = 0;
    long int ri = lrint(255. * r);
    long int gi = lrint(255. * g);
    long int bi = lrint(255. * b);
    long int ai = lrint(255. * a);

    //jbdlog("rgba %i %i %i %i\n\n",ri,gi,bi,ai);

    rgba += (ri << 24);
    rgba += (gi << 16);
    rgba += (bi << 8);
    rgba += ai;

    return rgba;
}

float PluginPrivateObject::BenderCalc(int deltaValue){
    float d = abs(deltaValue);
    float dist = pow(d*0.1,3.) * 0.0001;
    if (deltaValue < 0) dist = -dist;
    //Debug("d%i = %f \n",d, dist);
    return dist;
}
