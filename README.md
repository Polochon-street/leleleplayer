# leleleplayer
Leleleplayer is a classic audio player, which comes with a home-made library that calculates distance between songs, using it in order to smoothly play similar songs from your audio library.
Handy when you don't want bothering yourself with creating a complete playlist.

## Dependencies
* ffmpeg
* gstreamer-1.0 (and gst-plugins-good/bad/ugly)
* gtk3

## Installation

* clone repository on github
```bash
git clone https://github.com/Polochon-street/leleleplayer.git
```
* go to LPlayer root directory
```bash
cd leleleplayer 
```
* Create & go to build directory
```bash
mkdir build && cd build
```
* Generate the Makefile
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```
* Make the player & the analyze binaries
```bash
make
```
* Launch player
```bash
./player
```
