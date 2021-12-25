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
    audio/featureextractor.cpp \
    audio/fft.cpp \
    audio/waveanalys.cpp \
    audio/yin.cpp \
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
    audio/featureextractor.h \
    audio/fft.h \
    audio/waveanalys.h \
    audio/yin.h \
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
    main.qml \
    math/cutoffs \
    current \
    qml/ConsoleLog.qml \
    qml/PianoMap.qml \
    qml/audioHandler.qml \
    qml/consoleLog.qml \
    qml/main.qml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

