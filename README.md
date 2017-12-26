# buzzmachines

## intro
Buzzmachines are audio generators and effects using an API designed by the free
modular software music studio "Jeskola Buzz". (see http://jeskola.net/buzz/ and
http://www.buzzmachines.com).

This module provides a set of buzzmachines that have been published in source
code form on the internet. The aims of the project are:
* allow to rebuild the machines for new platforms (e.g. 64bit, new compiler
  versions, etc.)
* fix bugs (crashers, dc offsets, denormals)
* improve performance (smarter algorithms)
* be a good resource for developers that want to write new machines

## build status
We are running continuous integration on travis-ci

[![Build Status](https://travis-ci.org/Buzztrax/buzzmachines.svg?branch=master)](https://travis-ci.org/Buzztrax/buzzmachines/builds)

and app-veyor:

[![Build Status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true)](https://ci.appveyor.com/project/ensonic/buzzmachines)

## building on windows
Build with nmake in "VS x86 Native Tools Command Prompt" or "VS x64 Cross 
Tools Command Prompt" for either 32 or 64 bit machines respectively.

Supports either Release or Debug configurations. F.ex:

    nmake -f makefile.nmake CONFIGURATION=Release PLATFORM=x86
    nmake -f makefile.nmake CONFIGURATION=Debug PLATFORM=x64

Supports the "clean" make target:
 
    nmake -f makefile.nmake CONFIGURATION=Debug PLATFORM=x64 clean
	
The machine DLLs are currently left in their respective build directories.

## building on linux
To build use autogen.sh instead of configure. This accept the same options like
configure. Later one can use autoregen.sh to rerun the bootstrapping.

They can be used via bml library or in all gstreamer app via bml+gst-buzztrax
(see http://www.buzztrax.org)
You can install this module locally too. Use following option for
./autogen.sh or ./configure

    --prefix=$HOME/buzztrax/

Add the path to the BML_PATH env var:

    export BML_PATH=$HOME/buzztrax/lib/Gear:$HOME/buzztrax/lib/Gear/Effects:\
      $HOME/buzztrax/lib/Gear/Generators

The native machines will be installed to $prefix/lib/Gear.

