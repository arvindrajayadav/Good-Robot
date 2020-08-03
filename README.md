# Good Robot #

Most of the critical documents have been moved to Google docs. (Please email for access if you don't have it.)

##Official To-Do List##
https://docs.google.com/document/d/13Q1Y-BjT8PzEYVomvTaWnKbx-FZ15oiXEtL_w2zIwBY/edit

##Design Doc##
https://docs.google.com/document/d/1wugYxf3V9UDNGxsh5QyFkvdqQUqbJ1La86AHo2Pfd3Q/edit?usp=sharing

##Feature Discussions (Outdated)##
https://docs.google.com/document/d/1vc-LK_1EkZCmcxi3P_nMlwjF4z8X3SsHj7I_Sw3k0mw/edit#

##Story Discussion (Outdated)##
https://docs.google.com/document/d/1rEi0WgMdNQxI10TsJL57FW-EdbCDLC9AzIEXcKj_08Q/edit

### Linux ###
These instructions are assuming Ubuntu. They are not yet using the steam-runtime.

#### Linux - Setup ####

Install the build tools and the libraries

* `sudo apt-get install build-essential mercurial git cmake`
* `sudo apt-get install libsdl1.2-dev libboost-filesystem-dev libboost-system-dev libglew-dev libfreetype6-dev libalut-dev libdevil-dev`

#### Linux - Compile ####

* Clone the good-robot and good-robot-build repositories into a directory
* Go into good-robot checkout and create a build directory and enter build directory
( `cd good-robot`, `mkdir build`, `cd build`)
* Run Cmake in build directory pointing at the good-robot directory (`cmake ..`)
* Run make in the build directory (`make`)
* Go into the good-robot-build directory (`cd ../../good-robot-build`)
* Link (or copy) the "good_robot" executable into good-robot-build (`ln -s ../good-robot/build/good_robot`)
* Run "good_robot" in the good-robot-build directory (`./good_robot`)


[Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)