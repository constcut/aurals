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
    libs/fft/fftreal_wrapper.cpp \
    libs/kiss/kfc.c \
    libs/kiss/kiss_fft.c \
    libs/kiss/kiss_fftnd.c \
    libs/kiss/kiss_fftndr.c \
    libs/kiss/kiss_fftr.c \
    libs/sf/tsf.cpp \
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
    audio/features/PeaksOperations.hpp \
    audio/wave/AudioHandler.hpp \
    audio/wave/AudioReceiver.hpp \
    audio/wave/AudioSpeaker.hpp \
    audio/wave/AudioUtils.hpp \
    audio/features/FeatureExtractor.hpp \
    audio/spectrum/FrequencySpectrum.hpp \
    audio/spectrum/Spectrograph.hpp \
    audio/spectrum/Spectrum.hpp \
    audio/spectrum/SpectrumAnalyser.hpp \
    audio/wave/WavFile.hpp \
    audio/wave/WaveContour.hpp \
    audio/wave/WaveShape.hpp \
    audio/features/Yin.hpp \
    libs/fft/FFTCommon.hpp \
    libs/fft/FFTunreal.hpp \
    libs/fft/fftreal_wrapper.h \
    libs/kiss/_kiss_fft_guts.h \
    libs/kiss/kfc.h \
    libs/kiss/kiss_fft.h \
    libs/kiss/kiss_fft_log.h \
    libs/kiss/kiss_fftnd.h \
    libs/kiss/kiss_fftndr.h \
    libs/kiss/kiss_fftr.h \
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
    libs/fft/license.txt \
    libs/fft/readme.txt \
    main.qml \
    qml/ACFQML.qml \
    qml/ConsoleLog.qml \
    qml/MidiRenderSettings.qml \
    qml/PianoMap.qml \
    qml/WaveshapeQML.qml \
    qml/audioHandler.qml \
    qml/consoleLog.qml \
    qml/main.qml \
    qml/midiPlayer.qml \
    qml/tapper.qml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

