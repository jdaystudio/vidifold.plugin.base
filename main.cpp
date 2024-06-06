/**
    This is part of the example/base plugin files for creating FXPlugins for VIDIFOLD.
    see README.md for further notes.

    Author: John Day (contact@vidifold.com)
    File Release: 14/12/20 v1
    Uploaded to github: 06/05/24 v2

    Supplied files:

    fxpluginstructures.h
    main.cpp                    (THIS ONE)
    pluginprivateobject.cpp
    pluginprivateobject.h

    You also need for this example:
    1: REQUIRED: the opengl loading library I've chosen to use can be sourced from https://github.com/imakris/glatter
    2: OPTIONAL: for the advance functions I chose https://github.com/g-truc/glm
*/

// MUST USE STRUCTURES FROM fxpluginstructures.h
#include "fxpluginstructures.h"

#include "pluginprivateobject.h"

extern "C"
{
    // plugin will be disabled if this does not match currently supported host version
	int FXSystemVersionFunc(void) {
		return 9;
	};

	// on loading plugin
	void* CreateInstance(void) {
		PluginPrivateObject* p = new PluginPrivateObject();
		return (void*) p;
	}
	void Init(void* pointer,void* fxobject) {
		PluginPrivateObject* p = (PluginPrivateObject*)pointer;
		p->fx = (FXOBJECT*)fxobject;
		p->InitPlugin();
	}

	// should reset the plugin to its default setup
	void Reset(void* pointer) {
		PluginPrivateObject* p = (PluginPrivateObject*)pointer;
		p->Reset();
	}
	void Random(void* pointer) {
		PluginPrivateObject* p = (PluginPrivateObject*)pointer;
		p->RandomizeState();
	}

	// called by host before process to feed parameter changes
	void Update(void* pointer) {
		PluginPrivateObject* p = (PluginPrivateObject*)pointer;
		p->Update();
	}
	// this is the actual call to update/produce frame
	void Process(void* pointer) {
		PluginPrivateObject* p = (PluginPrivateObject*)pointer;
		p->Process();
	}

	// on unloading plugin
	void Deinit(void* pointer) {
		PluginPrivateObject* p = (PluginPrivateObject*)pointer;
		delete(p);
	}

	// request for current setup for state storage
	void GetState(void* pointer) {
		PluginPrivateObject* p = (PluginPrivateObject*)pointer;
		p->GetState();
	}
	// called with a previous state to restore setup
	bool SetState(void* pointer, void* fxstate) {
		PluginPrivateObject* p = (PluginPrivateObject*)pointer;
		return p->SetState((FXSTATE*)fxstate);
	}
}
