**VIDIFOLD plugin base**

This is the example/base plugin files for creating FXPlugins for VIDIFOLD

I'm not a C++ person outside of this project, so this may seem basic / wrong in places, but it is how I got it built. These files have been my starting point since 2014 and as of June 24 there are 111 plugins built from this. The project is built in Code::Blocks IDE, so there are no make files as of yet.

Author: John Day (contact@vidifold.com)<br>
Uploaded to github: 06/05/24 v2<br>
Files release as archive: 14/12/20 v1<br>

**Supplied files**<br>
fxpluginstructures.h < you should not play with this<br>
main.cpp<br>
pluginprivateobject.cpp<br>
pluginprivateobject.h<br>

You will also need for this example:

1: REQUIRED: a opengl loading library, the one I've use can be sourced from https://github.com/imakris/glatter<br>
2: OPTIONAL: for the more modern functions, I chose https://github.com/g-truc/glm

Link libraries:  GL<br>
Linker options:  -rdynamic, -fPIC

When compiled, place in VIDIFOLD/plugins to use.

---

**Host (VIDIFOLD) behaviour**

1- locates and checks that the version return by FXSystemVersionFunc matches the currently supported version and all required functions exist.<br>
2 - locates Main function<br>
3 - host supplies an FXOBJECT which handles most of the parameter interaction<br>
4 - calls Init<br>
<br>
in the Render loop (if the plugin is enabled and in use)<br>
5 - calls update<br>
6 - calls process<br>
7 - (recently added optional) calls random<br>

---

**Testing / Developement**

Due to the need to run the plugin in the host for development it can get tricky to test.

I tend to get the plugin working without changing the PluginPrivateState storage. 

Once happy with the effect and options, I follow through on the state storage, **it is much less trouble if you delete the registered plugin from within VIDIFOLD between changes to the storage structure!**  


Note: Canonical names MUST BE UNIQUE, with this in mind, VIDIFOLD internal plugin names are 000-VIDIFOLD-PLUGINNAME (there are also a few legacy names around which I'll clean up if they become a problem) and VF-PLUGINNAME.so for the actual binaries.  You a free to use whatever format you like, but I suggest you prepend them with something unique to avoid confusion.



I doubt anyone will ever use this, but if you do, you are welcome to get in touch.
