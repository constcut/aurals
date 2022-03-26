# Aurals
Engine for music and voice analysis, and music creation.
There is a hope to extend it into music therapy applications.

This application was developed with Qt 5.15.2 using QML. Tested on linux/android/windows.

Current status is pre-alpha, no releases yet.
Most of components yet build to the stage 'proof of concept', when all types of components would be fullfilled there would start work to complete them all.

## Features

* Audio recording, audio playing (from file or recorder)
* MIDI files loading, saving, generating (several effects functions migrated from guitar tablatures)
* Audio generation from MIDI files using soundfonts
* Feature extration: root mean square, F0, F0 to midi, mean, median, mode, range, peaks finder
* Audio qml component, to explore sound using root mean square, with start\end note peaks finder and F0
* Spectrograph component using fast fourier transform with various parameters
* Possible noise detection in fft window (yet very plain)
* STFT computation and visualization
* Constant-Q computation and visualization
* Cepstrum calculation
* Autocorrelation visualization and YIN algorithm visualization
* Tapping componet, to tap rhythm using mouse\mobile device screen with export to midi\pcm
* Piano roll component - to load and edit midi files
* Pattern input - a way to fast write drums, polyrithm option included
* Guitar tablature components - to view and edit guitar tabs
* QML logger component - to read logs on mobile screen (or without running application from command line)

Application contains several soundfonts and midi files to check functionality,
Implemented test to detect MIDI files regression.

## Screenshots

Wave view and spectrograph:

![mtherapp screenshot](res/info/screenshots/wave_fft.png)

Wave view claps start end detection (blue is start, red is end):

![mtherapp screenshot](res/info/screenshots/claps.png)

Wave view with f0 diplay (red line below)

![mtherapp screenshot](res/info/screenshots/wave_f0.png)

Short time fourier transform:

![mtherapp screenshot](res/info/screenshots/stft.png)

Constant-Q transform:

![mtherapp screenshot](res/info/screenshots/constq.png)

Autocorelation and Yin visualization:

![mtherapp screenshot](res/info/screenshots/acf.png)

Filters comparison visualization (yet only low and high pass + both):

![mtherapp screenshot](res/info/screenshots/filters.png)

Cepstrum visualization:

![mtherapp screenshot](res/info/screenshots/cepstrum.png)

Most of the visualization could be saved as image.

To access all types of audio analysis there is a simple recorder:

![mtherapp screenshot](res/info/screenshots/recorder.png)

There is midi player based on sound fonts:

![mtherapp screenshot](res/info/screenshots/midi_2.png)

There is tapping function, that lets you tap rhythm on screen, and save it to pcm or midi:

![mtherapp screenshot](res/info/screenshots/tapping.png)

And there is tablature editor\viewer, yet in development:

![mtherapp screenshot](res/info/screenshots/tab.png)

Piano roll, to load and edit midi files, yet no all the functions like velocy supported:

![mtherapp screenshot](res/info/screenshots/piano_roll.png)

Pattern input lets you write drum tracks fast, it has a polyrithm option, that lets you write different drum instuments with different bar sizes, PI will calculate when bar sizes converge and generate midi\wav:

![mtherapp screenshot](res/info/screenshots/pattern_input.png)

## Libraries

Project uses several libraries:

* TinySoundFont

https://github.com/schellingb/TinySoundFont

* FFTReal (tiny tweaked)

https://github.com/cyrilcode/fft-real

* KISS fft

https://github.com/mborgerding/kissfft

* Constant-Q Library (tweaked for float samples)

https://code.soundsoftware.ac.uk/projects/constant-q-cpp

* DSP IIR realtime filter library

https://github.com/berndporr/iir1

* Wavelib - wavelet library (currently turned off, but library is in the repo)

https://github.com/rafat/wavelib

All libraries have permissive license, that help use same type of license for this project.


## Icons

All icons used in current version are taken from icons8 site!


## Current status

Project is suspended because lack of resourses, but this would not last forever (I hope).
