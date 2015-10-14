# leleleplayer
Leleleplayer is a classic audio player, which comes with a home-made library that computes distance between songs, using it in order to smoothly play similar songs from your audio library.
Handy when you don't want to bother yourself with creating a complete playlist.

## Dependencies
* ffmpeg
* gstreamer-1.0 (and gst-plugins-good/bad/ugly)
* gtk3

## Installation

* clone repository on github
```bash
$ git clone https://github.com/Polochon-street/leleleplayer.git
```
* go to LPlayer root directory
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
# make install
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

