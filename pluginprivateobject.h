/**
    This is part of the example/base plugin files for creating FXPlugins for VIDIFOLD.
    see README.md for further notes.

    Author: John Day (contact@vidifold.com)
    File Release: 14/12/20 v1
    Uploaded to github: 06/05/24 v2

    Supplied files:

    fxpluginstructures.h
    main.cpp
    pluginprivateobject.cpp
    pluginprivateobject.h       (THIS ONE)

    You also need for this example:
    1: REQUIRED: the opengl loading library I've chosen to use can be sourced from https://github.com/imakris/glatter
    2: OPTIONAL: for the advance functions I chose https://github.com/g-truc/glm
*/

#ifndef PLUGINPRIVATEOBJECT_H
#define PLUGINPRIVATEOBJECT_H

#include <string>
#include <stdarg.h>

//#include <glm/glm.hpp>  //NOT included in this simple example plugin (https://github.com/g-truc/glm)

#include "fxpluginstructures.h"


// a basic state setup struct
struct PluginPrivateState{
    int r;
};

class PluginPrivateObject
{
	public:
		FXOBJECT* fx;

		PluginPrivateObject();
		virtual ~PluginPrivateObject();

		// helpers
		void CreateParam(FXPARAMTYPE t,std::string name,int minValue,int maxValue,int currentValue,int defaultValue);
		void CreateShaderParam(int shaderID,int t,std::string name,float value);
		void CreateShaders();

		// required (called from main.cpp)
		void InitPlugin();
		void Deinit();
		void Reset();
		void Random();
		void Update();
		void Process();
		void GetState();
		bool SetState(FXSTATE* fxstate);
        void RandomizeState();
	protected:
	private:
        bool resetTriggered;
        PluginPrivateState state;

        // example structs for more modern shader setup
        // not used in first example
        /*glm::vec3 sqverts[4];
        glm::vec2 squvs[4];
        GLuint sqvertsVBO;
        GLuint squvsVBO;*/

        double FPns;
        double localPrevTime;
        double localElapsedTime;
        double framesTraveled;
        double gentimer;

		bool firstRun;
		float displayWRatio,displayHRatio;


        string sourceLabels[11];
        //string beatLabels[9];
        //int beatmode;
        //int hdsqDelays[9];
        int prevbpm;
        int prevHit, curHit;
        bool newHit;

        float r;

		void Process120Example();
		// requires glm headers (not supplied https://github.com/g-truc/glm)
		//void Process330Example();

		// helper functions
		void nonUpdateOnParams();
		void ProgressTime();
        double GetRealTimestamp();
        string IntToString(int value);
        string DoubleToString(double value);
        void Debug(const char* format, ... );
        void DumpFBO(string filename,unsigned int w,unsigned int h,GLuint FBOID);
        bool SaveRawImage(string filename,unsigned char *buffer, int width, int height,int bytes);
        void DrawQuad(float x1, float y1, float tx1, float ty1, float x2, float y2, float tx2, float ty2, float z);
        void DrawQuad(float x1, float y1, float tx1, float ty1, float x2, float y2, float tx2, float ty2);
        void DrawQuad2(float x1, float y1, float tx1, float ty1,float ttx1,float tty1, float x2, float y2, float tx2,float ty2,float ttx2,float tty2);
        void ClearFBO(GLuint FBOID, int width, int height);
        void RGBA2Param(float r,float g, float b, float a, int p);
        void Param2RGBA(int p, float &r, float &g, float &b, float &a);
        unsigned long RGBA2Int(float r,float g, float b, float a);
        float BenderCalc(int deltaValue);

        //void _migrateState1(FXSTATE* fxstate, PluginPrivateState* localState);
        //void _useState2(FXSTATE* fxstate, PluginPrivateState* localState);
};

#endif // PLUGINPRIVATEOBJECT_H
