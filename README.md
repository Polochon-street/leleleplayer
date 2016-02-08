# leleleplayer v0.7.0
Leleleplayer is a classic audio player, which comes with a [home-made library](https://github.com/Polochon-street/bliss) that computes distance between songs, using it in order to smoothly play similar songs from your audio library.
Handy when you don't want to bother yourself with creating a complete playlist.
(Unstable) Windows binaries are available in the [release](https://github.com/Polochon-street/leleleplayer/releases/) tab

![Leleleplayer screenshot](http://i.imgur.com/6CPLGVx.png)

## Dependencies
* [bliss](https://github.com/Polochon-street/bliss) library
* ffmpeg
* gstreamer-1.0 (and gst-plugins-good/bad/ugly)
* gtk3

## Installation

### Windows users:

* Get a release here: https://github.com/Polochon-street/leleleplayer/releases

* or (the hard way:) clone repository on github
```bash
$ git clone https://github.com/Polochon-street/leleleplayer.git
```
* go to leleleplayer root directory
```bash
$ cd leleleplayer 
```
* Create and enter the build directory
```bash
$ mkdir build_windows && cd build_windows
```
* You'll now need to have proper directories in your build\_windows directory, containing mingw-obtained dlls: bin/, images/, share/, lib/ <br />
(If you don't know how to do it, and still want to, contact me)
* Generate the Makefile
```bash
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../Windows_toolchain.cmake
```
* Compile the player & the analyze binaries
```bash
$ make
```
* Package the player
```bash
$ make package
```
* Patch the package
```bash
$ cd _CPack_Packages/win64/NSIS
```
* Edit project.nsi by adding SetOutPath « "$INSTDIR\bin" » before « CreateShortCut "$SMPROGRAMS\$STARTMENU\_FOLDER\Leleleplayer.lnk" "$INSTDIR\bin\leleleplayer" »
* Package the player again
```bash
$ makensis project.nsi
```
* The windows binary is now available in build\_windows/\_CPack\_Packages/win64/NSIS/Leleleplayer-xx-win64.exe

### Archlinux users

* install AUR package via yaourt
```bash
$ yaourt -S leleleplayer-git
```
* or download pkgbuild here: https://aur.archlinux.org/packages/leleleplayer-git/

### Other Linux users 

* clone repository on github
```bash
$ git clone https://github.com/Polochon-street/leleleplayer.git
```
* go to leleleplayer root directory
```bash
$ cd leleleplayer 
```
* Create and enter the build directory
```bash
$ mkdir build && cd build
```
* Generate the Makefile
```bash
$ cmake .. -DCMAKE_BUILD_TYPE=Release
```
* Compile the player & the analyze binaries
```bash
$ make
```
* Install the player
```bash
# make install && glib-compile-schemas /usr/share/glib-2.0/schemas
```
* Launch the player
```bash
$ leleleplayer
```
## Usage of the analyze binary

* Check whether a song is calm or loud
```bash
$ ./analyze /path/to/song
```
Returns 1 if the song is calm, 0 otherwise
* Check if a song is calm or loud, with debug information (see below for details)
```bash
$ ./analyze -d /path/to/song
```

## How does the analysis process work?

Leleleplayer and the analyze binary use the same library, liblelele. For every song analyzed, liblelele returns a struct song which contains, among other things, 
four floats, each rating an aspect of the song:

* The [tempo](https://en.wikipedia.org/wiki/Tempo "link to wikipedia") rating draws the envelope of the whole song, and then computes its DFT, obtaining peaks at the frequency of each dominant beat. 
The period of each dominant beat can then be deduced from the frequencies, hinting at the song's tempo.<br />
Warning: the tempo is not equal to the force of the song. As an example , a heavy metal track can have no steady beat at all, giving a very low tempo score while being very loud.

* The amplitude rating reprents the physical « force » of the song, that is, how much the speaker's membrane will move in order to create the sound.<br />
It is obtained by applying a magic formula with magic coefficients to a histogram of the values of all the song's samples

* The frequency rating is a ratio between high and low frequencies: a song with a lot of high-pitched sounds tends to wake humans up far more easily.<br />
This rating is obtained by performing a DFT over the sample array, and splitting the resulting array in 4 frequency bands: low, mid-low, mid, mid-high, and high.
Using the value in dB for each band, the final formula corresponds to freq\_result = high + mid-high + mid - (low + mid-low)

* The [attack](https://en.wikipedia.org/wiki/Synthesizer#ADSR_envelope "link to wikipedia") rating computes the difference between each value in the envelope and the next (its derivative).<br />
The final value is obtained by dividing the sum of the positive derivates by the number of samples, in order to avoid different results just because of the songs' length.<br />
As you have already guessed, a song with a lot of attacks also tends to wake humans up very quickly.

