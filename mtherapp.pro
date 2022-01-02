QT += quick multimedia
CONFIG += c++17
#CONFIG += no_keywords #was used only for python interpreter
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += -std=c++17

android: QT += androidextras

SOURCES += \
    app/androidtools.cpp \
    audio/audiohandler.cpp \
    audio/audioreceiver.cpp \
    audio/audiospeaker.cpp \
    audio/envelop.cpp \
    audio/featureextractor.cpp \
    audio/findpeaks.cpp \
    audio/frequencyspectrum.cpp \
    audio/spectrograph.cpp \
    audio/spectrumanalyser.cpp \
    audio/utils.cpp \
    audio/wavecontour.cpp \
    audio/waveshape.cpp \
    audio/wavfile.cpp \
    audio/yin.cpp \
    libs/fft/fftreal_wrapper.cpp \
    main.cpp \
    app/loghandler.cpp \
    midi/MidiFile.cpp \
    midi/MidiMessage.cpp \
    midi/MidiTrack.cpp \
    midi/NBytesInt.cpp \
    music/graphicmap.cpp \
    music/midiengine.cpp \
    music/midifile.cpp \
    music/midirender.cpp \
    libs/sf/tsf.cpp \
    app/init.cpp

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
    app/androidtools.h \
    app/loghandler.h \
    audio/audiohandler.h \
    audio/audioreceiver.h \
    audio/audiospeaker.h \
    audio/envelop.h \
    audio/featureextractor.h \
    audio/findpeaks.hpp \
    audio/frequencyspectrum.h \
    audio/spectrograph.h \
    audio/spectrum.h \
    audio/spectrumanalyser.h \
    audio/utils.h \
    audio/wavecontour.h \
    audio/waveshape.h \
    audio/wavfile.h \
    audio/yin.h \
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
    midi/MidiFile.hpp \
    midi/MidiMessage.hpp \
    midi/MidiTrack.hpp \
    midi/NBytesInt.hpp \
    music/graphicmap.h \
    music/midiengine.h \
    music/midifile.h \
    music/midirender.h \
    libs/sf/tml.h \
    libs/sf/tsf.h \
    app/init.h


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

