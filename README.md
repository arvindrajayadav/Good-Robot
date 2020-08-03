# Good Robot

This is the source code for Good Robot, a procedurally generated shoot 'em up with light RPG elements made by Pyrodactyl Games in collaboration with Shamus Young.
The code is based on a Good Robot prototype by Shamus Young, which was upgraded to SDL2 and a lot of CRAB engine (i.e. the Pyrodactyl engine used in Unrest and Will Fight for Food) features were integrated into it. 

# How to Compile

After cloning the repository, you will need to set a valid Boost source and lib directories in Visual Studio settings. 
For more information on how to set up Boost, visit https://www.boost.org/

# Linux - Setup

Install the build tools and the libraries

    sudo apt-get install build-essential mercurial git cmake
    sudo apt-get install libsdl1.2-dev libboost-filesystem-dev libboost-system-dev libglew-dev libfreetype6-dev libalut-dev libdevil-dev

# Running the Game

After successfully compiling and building the executable, you will need the game's assets. 
If you have a copy of the game, navigate to the install directory and:
1. Copy the "core" folder to the binary location
2. Copy the various DLL files to the binary location

If you're having any issues with getting the code up and running, or just want to chat about the game, feel free to contact me!
