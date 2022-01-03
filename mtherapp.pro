QT += quick multimedia core
CONFIG += c++17

Debug:CONFIG += sanitizer sanitize_address sanitize_memory sanitize_undefined
#sanitize_thread

#CONFIG += no_keywords #was used only for python interpreter
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += -std=c++17

android: QT += androidextras

SOURCES += \
    app/AndroidTools.cpp \
    app/Init.cpp \
    app/LogHandler.cpp \
    audio/AudioHandler.cpp \
    audio/AudioReceiver.cpp \
    audio/AudioSpeaker.cpp \
    audio/AudioUtils.cpp \
    audio/FeatureExtractor.cpp \
    audio/FindPeaks.cpp \
    audio/FrequencySpectrum.cpp \
    audio/Spectrograph.cpp \
    audio/SpectrumAnalyser.cpp \
    audio/WavFile.cpp \
    audio/WaveContour.cpp \
    audio/WaveShape.cpp \
    audio/Yin.cpp \
    libs/fft/fftreal_wrapper.cpp \
    main.cpp \
    midi/MidiEngine.cpp \
    midi/MidiFile.cpp \
    midi/MidiMessage.cpp \
    midi/MidiRender.cpp \
    midi/MidiTrack.cpp \
    midi/NBytesInt.cpp \
    music/GraphicMap.cpp

RESOURCES += qml.qrc \
    fonts.qrc \
    soundfonts.qrc

windows:DEFINES += __WINDOWS_MM__
windows:LIBS += -lWINMM

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
    audio/AudioHandler.hpp \
    audio/AudioReceiver.hpp \
    audio/AudioSpeaker.hpp \
    audio/AudioUtils.hpp \
    audio/FeatureExtractor.hpp \
    audio/FindPeaks.hpp \
    audio/FrequencySpectrum.hpp \
    audio/Spectrograph.hpp \
    audio/Spectrum.hpp \
    audio/SpectrumAnalyser.hpp \
    audio/WavFile.hpp \
    audio/WaveContour.hpp \
    audio/WaveShape.hpp \
    audio/Yin.hpp \
    libs/fft/Array.h \
    libs/fft/Array.hpp \
    libs/fft/DynArray.h \
    libs/fft/DynArray.hpp \
    libs/fft/FFTReal.h \
    libs/fft/FFTReal.hpp \
    libs/fft/FFTRealFixLen.h \
    libs/fft/FFTRealFixLen.hpp \
    libs/fft/FFTRealFixLenParam.h \
    libs/fft/FFTRealPassDirect.h \
    libs/fft/FFTRealPassDirect.hpp \
    libs/fft/FFTRealPassInverse.h \
    libs/fft/FFTRealPassInverse.hpp \
    libs/fft/FFTRealSelect.h \
    libs/fft/FFTRealSelect.hpp \
    libs/fft/FFTRealUseTrigo.h \
    libs/fft/FFTRealUseTrigo.hpp \
    libs/fft/OscSinCos.h \
    libs/fft/OscSinCos.hpp \
    libs/fft/def.h \
    libs/fft/fftreal_wrapper.h \
    libs/stft/HannWindow.h \
    libs/stft/Ooura_FFT.h \
    libs/stft/PostProcessor.h \
    libs/stft/STFT.h \
    libs/stft/WAV.h \
    log.hpp \
    midi/MidiEngine.hpp \
    midi/MidiFile.hpp \
    midi/MidiMessage.hpp \
    midi/MidiRender.hpp \
    midi/MidiTrack.hpp \
    midi/NBytesInt.hpp \
    libs/sf/tml.h \
    libs/sf/tsf.h \
    music/GraphicMap.hpp


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
    math/cutoffs \
    current \
    qml/ConsoleLog.qml \
    qml/PianoMap.qml \
    qml/WaveshapeQML.qml \
    qml/audioHandler.qml \
    qml/consoleLog.qml \
    qml/main.qml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

