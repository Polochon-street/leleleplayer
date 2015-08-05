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

## Usage of the analyze binary

* Check if a song is calm or loud
```bash
./analyze /path/to/song
```
Returns 1 if the song is calm, 0 otherwise
* Check if a song is calm or loud, with debug informations (see below for details)
```bash
./analyze -d /path/to/song
```

## How does the analyzing process work?

Leleleplayer and the analyze binaries use the same library, liblelele. For every song analyzed, liblelele returns a struct song which contains, among other things, 
four floats, each rating an aspect of the song:

* The [tempo](https://en.wikipedia.org/wiki/Tempo "link to wikipedia") rating draws the envelope of the whole song, and then calculates a FFT of this envelope, obtaining peaks at the frequency of each dominant beat. 
The period of each dominant beat can then be deduced from the frequencies, giving an indication of the tempo of the song.<br />
Warning: the tempo is not equal to the force of song. For example, a heavy metal track can have no regular beats at all, giving a very low tempo score while being very loud.

* The amplitude rating corresponds to the physical « force » of the song, that is, how much the speaker's membrane will move in order to create the sound.<br />
It is calculated by doing an histogram of the sample array, and then applying a magic formula with magic coefficients.

* The frequency rating evaluates the proportion of high-pitched and low-pitched frequencies: a song with a lot of high-pitched sounds tends to wake up someone far more easily.<br />
The rating's evaluation is done by calculating the sample array's FFT, and then cutting this array in 4 frequency bands: low, mid-low, mid, mid-high, and high.
There's a value in dB for each band; the final formula is freq\_result = high + mid-high + mid - (low + mid-low)

* The [attack](https://en.wikipedia.org/wiki/Synthesizer#ADSR_envelope "link to wikipedia") rating calculates the difference of each consecutive point of the envelope (its derivate).<br />
The final value is obtained by dividing the sum of the positive derivates by the number of samples, in order to avoid different results just because of the songs' length.<br />
As you have already guessed, a song with a lot of attacks also tends to wake someone up very quickly.

