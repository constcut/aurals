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

![mtherapp screenshot](info/screenshots/wave_and_spectrum.png)

Wave view claps start end detection (blue is start, red is end):

![mtherapp screenshot](info/screenshots/claps.png)

Wave view with f0 diplay (red line below)

![mtherapp screenshot](info/screenshots/wave_f0.png)
