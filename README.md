# mtherapp
Engine for music and voice applications, including music therapy

This application was developed with Qt 5.15.2 using QML. Tested on linux/android/windows.

Current status is pre-alpha, no releases yet.

## Features

* Audio recording, audio playing (from file or recorder)
* MIDI files loading, saving, generating (several effects functions migrated from guitar tablatures)
* Audio generation from MIDI files using soundfonts
* Feature extration: root mean square, F0, F0 to midi, mean, median, mode, range, peaks finder
* Audio qml component, to explore sound using root mean square, with start\end note peaks finder and F0
* Spectrograph component using fast fourier transform with various parameters
* Possible noise detection in fft window (yet very plain)
* Tapping componet, to tap rhythm using mouse\mobile device screen with export to midi\pcm
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

![mtherapp screenshot](res/info/screenshots/midi.png)

There is tapping function, that lets you tap rhythm on screen, and save it to pcm or midi:

![mtherapp screenshot](res/info/screenshots/tapping.png)

And there is tablature editor\viewer, yet in development:

![mtherapp screenshot](res/info/screenshots/tab.png)


## Libraries

Project uses 2 libraries:

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

* Wavelib - wavelet library (currently turned of, but library is in the repo)

https://github.com/rafat/wavelib


All libraries have permissive license, that help use same license for this project.