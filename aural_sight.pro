QT += quick multimedia core
CONFIG += c++17

#Doesn't work on windows
linux:Debug:CONFIG += sanitizer sanitize_address sanitize_memory sanitize_undefined
#sanitize_thread

QMAKE_CXXFLAGS_RELEASE += -O3


#CONFIG += no_keywords #was used only for python interpreter
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += -Wall

android: QT += androidextras

SOURCES += \
    app/AndroidTools.cpp \
    app/Init.cpp \
    app/LogHandler.cpp \
    audio/features/ACFgraph.cpp \
    audio/wave/AudioHandler.cpp \
    audio/wave/AudioReceiver.cpp \
    audio/wave/AudioSpeaker.cpp \
    audio/wave/AudioUtils.cpp \
    audio/features/FeatureExtractor.cpp \
    audio/spectrum/FrequencySpectrum.cpp \
    audio/spectrum/Spectrograph.cpp \
    audio/spectrum/SpectrumAnalyser.cpp \
    audio/wave/WavFile.cpp \
    audio/wave/WaveContour.cpp \
    audio/wave/WaveShape.cpp \
    audio/features/Yin.cpp \
    libs/cqt/CQInverse.cpp \
    libs/cqt/CQKernel.cpp \
    libs/cqt/CQSpectrogram.cpp \
    libs/cqt/Chromagram.cpp \
    libs/cqt/ConstantQ.cpp \
    libs/cqt/Pitch.cpp \
    libs/cqt/dsp/FFT.cpp \
    libs/cqt/dsp/KaiserWindow.cpp \
    libs/cqt/dsp/MathUtilities.cpp \
    libs/cqt/dsp/Resampler.cpp \
    libs/cqt/dsp/SincWindow.cpp \
    libs/kiss/kfc.c \
    libs/kiss/kiss_fft.c \
    libs/kiss/kiss_fftnd.c \
    libs/kiss/kiss_fftndr.c \
    libs/kiss/kiss_fftr.c \
    libs/sf/tsf.cpp \
    libs/wavelet/conv.c \
    libs/wavelet/cwt.c \
    libs/wavelet/cwtmath.c \
    libs/wavelet/hsfft.c \
    libs/wavelet/real.c \
    libs/wavelet/wavefilt.c \
    libs/wavelet/wavefunc.c \
    libs/wavelet/wavelib.c \
    libs/wavelet/wtmath.c \
    main.cpp \
    midi/MidiEngine.cpp \
    midi/MidiFile.cpp \
    midi/MidiMessage.cpp \
    midi/MidiRender.cpp \
    midi/MidiTrack.cpp \
    midi/NBytesInt.cpp \
    music/GraphicMap.cpp \
    music/Tapper.cpp

RESOURCES += qml.qrc \
    fonts.qrc \
    soundfonts.qrc


QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


HEADERS += \
    app/AndroidTools.hpp \
    app/Clipboard.hpp \
    app/Init.hpp \
    app/LogHandler.hpp \
    app/StretchImage.hpp \
    audio/features/ACFgraph.hpp \
    audio/features/PeaksOperations.hpp \
    audio/features/WindowFunction.hpp \
    audio/wave/AudioHandler.hpp \
    audio/wave/AudioReceiver.hpp \
    audio/wave/AudioSpeaker.hpp \
    audio/wave/AudioUtils.hpp \
    audio/features/FeatureExtractor.hpp \
    audio/spectrum/FrequencySpectrum.hpp \
    audio/spectrum/Spectrograph.hpp \
    audio/spectrum/SpectrumAnalyser.hpp \
    audio/wave/WavFile.hpp \
    audio/wave/WaveContour.hpp \
    audio/wave/WaveShape.hpp \
    audio/features/Yin.hpp \
    libs/cqt/CQBase.h \
    libs/cqt/CQInverse.h \
    libs/cqt/CQKernel.h \
    libs/cqt/CQParameters.h \
    libs/cqt/CQSpectrogram.h \
    libs/cqt/Chromagram.h \
    libs/cqt/ConstantQ.h \
    libs/cqt/Pitch.h \
    libs/cqt/dsp/FFT.h \
    libs/cqt/dsp/KaiserWindow.h \
    libs/cqt/dsp/MathUtilities.h \
    libs/cqt/dsp/Resampler.h \
    libs/cqt/dsp/SincWindow.h \
    libs/cqt/dsp/Window.h \
    libs/cqt/dsp/nan-inf.h \
    libs/cqt/dsp/pi.h \
    libs/fft/FFTRealFixed.hpp \
    libs/kiss/_kiss_fft_guts.h \
    libs/kiss/kfc.h \
    libs/kiss/kiss_fft.h \
    libs/kiss/kiss_fft_log.h \
    libs/kiss/kiss_fftnd.h \
    libs/kiss/kiss_fftndr.h \
    libs/kiss/kiss_fftr.h \
    libs/wavelet/conv.h \
    libs/wavelet/cwt.h \
    libs/wavelet/cwtmath.h \
    libs/wavelet/hsfft.h \
    libs/wavelet/real.h \
    libs/wavelet/wauxlib.h \
    libs/wavelet/wavefilt.h \
    libs/wavelet/wavefunc.h \
    libs/wavelet/wavelib.h \
    libs/wavelet/wtmath.h \
    log.hpp \
    midi/MidiEngine.hpp \
    midi/MidiFile.hpp \
    midi/MidiMessage.hpp \
    midi/MidiRender.hpp \
    midi/MidiTrack.hpp \
    midi/MidiUtils.hpp \
    midi/NBytesInt.hpp \
    libs/sf/tml.h \
    libs/sf/tsf.h \
    music/GraphicMap.hpp \
    music/Tapper.h


DISTFILES += \
    ConsoleLog.qml \
    OpenTab.qml \
    TODO \
    Tablature.qml \
    PianoMap.qml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    libs/cqt/COPYING \
    libs/cqt/README \
    libs/fft/license.txt \
    libs/fft/readme.txt \
    main.qml \
    qml/ACFQML.qml \
    qml/CQTQML.qml \
    qml/ConsoleLog.qml \
    qml/MidiRenderSettings.qml \
    qml/PianoMap.qml \
    qml/STFTQML.qml \
    qml/WaveshapeQML.qml \
    qml/audioHandler.qml \
    qml/consoleLog.qml \
    qml/main.qml \
    qml/midiPlayer.qml \
    qml/tapper.qml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

