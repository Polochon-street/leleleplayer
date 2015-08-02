# leleleplayer
Simple audio player with the main purpose of analyzing music folders in order to play relaxing songs, handy for sleeping.

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
cmake ..
```
* Make the player & the analyze binaries
```bash
make
```
* Launch player
```bash
./player
```
