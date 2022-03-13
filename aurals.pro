QT += quick multimedia core widgets
CONFIG += c++17

#Doesn't work on windows
linux:Debug:CONFIG += sanitizer sanitize_address sanitize_memory sanitize_undefined
#sanitize_thread

QMAKE_CXXFLAGS_RELEASE += -O3


win32:LIBS += -lWinMM

#CONFIG += no_keywords #was used only for python interpreter
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += -Wall

android: QT += androidextras

QMAKE_LFLAGS += -v

#Wavelets are commented, yet no application for such amazing detalization

SOURCES += \
    app/AndroidTools.cpp \
    app/Config.cpp \
    app/Init.cpp \
    app/LogHandler.cpp \
    app/Regression.cpp \
    app/Tests.cpp \
    audio/features/ACFgraph.cpp \
    audio/spectrum/Cepstrumgraph.cpp \
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
    libs/filters/Biquad.cpp \
    libs/filters/Butterworth.cpp \
    libs/filters/Cascade.cpp \
    libs/filters/ChebyshevI.cpp \
    libs/filters/ChebyshevII.cpp \
    libs/filters/Custom.cpp \
    libs/filters/PoleFilter.cpp \
    libs/filters/RBJ.cpp \
    libs/kiss/kfc.c \
    libs/kiss/kiss_fft.c \
    libs/kiss/kiss_fftnd.c \
    libs/kiss/kiss_fftndr.c \
    libs/kiss/kiss_fftr.c \
    libs/sf/tsf.cpp \
    #libs/wavelet/conv.c \
    #libs/wavelet/cwt.c \
    #libs/wavelet/cwtmath.c \
    #libs/wavelet/hsfft.c \
    #libs/wavelet/real.c \
    #libs/wavelet/wavefilt.c \
    #libs/wavelet/wavefunc.c \
    #libs/wavelet/wavelib.c \
    #libs/wavelet/wtmath.c \
    main.cpp \
    midi/MidiEngine.cpp \
    midi/MidiFile.cpp \
    midi/MidiMessage.cpp \
    midi/MidiRender.cpp \
    midi/MidiTrack.cpp \
    midi/NBytesInt.cpp \
    music/GraphicMap.cpp \
    music/PatternLine.cpp \
    music/PatternReceiver.cpp \
    music/PianoRoll.cpp \
    music/Tapper.cpp \
    tab/Bar.cpp \
    tab/Beat.cpp \
    tab/Effects.cpp \
    tab/Note.cpp \
    tab/Tab.cpp \
    tab/Track.cpp \
    tab/tools/Base.cpp \
    tab/tools/Commands.cpp \
    tab/tools/GmyFile.cpp \
    tab/tools/GtpFiles.cpp \
    tab/tools/MidiExport.cpp \
    tab/tools/TabClipboard.cpp \
    tab/tools/TabLoader.cpp \
    tab/tools/Threads.cpp \
    tab/tools/tab_com.cpp \
    tab/tools/track_com.cpp \
    tab/ui/BarView.cpp \
    tab/ui/TabCommands.cpp \
    tab/ui/TabViews.cpp \
    tab/ui/TrackView.cpp

RESOURCES += qml.qrc \
    res/fonts.qrc \
    res/soundfonts.qrc \
    res/tests.qrc \
    res/icons.qrc \
    res/icons2.qrc


QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


HEADERS += \
    app/AndroidTools.hpp \
    app/Clipboard.hpp \
    app/Config.hpp \
    app/Init.hpp \
    app/LogHandler.hpp \
    app/Regression.hpp \
    app/StretchImage.hpp \
    app/Tests.hpp \
    app/log.hpp \
    audio/features/ACFgraph.hpp \
    audio/features/PeaksOperations.hpp \
    audio/features/WindowFunction.hpp \
    audio/spectrum/Cepstrumgraph.hpp \
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
    libs/filters/Biquad.h \
    libs/filters/Butterworth.h \
    libs/filters/Cascade.h \
    libs/filters/ChebyshevI.h \
    libs/filters/ChebyshevII.h \
    libs/filters/Common.h \
    libs/filters/Custom.h \
    libs/filters/Iir.h \
    libs/filters/Layout.h \
    libs/filters/MathSupplement.h \
    libs/filters/PoleFilter.h \
    libs/filters/RBJ.h \
    libs/filters/State.h \
    libs/filters/Types.h \
    libs/kiss/_kiss_fft_guts.h \
    libs/kiss/kfc.h \
    libs/kiss/kiss_fft.h \
    libs/kiss/kiss_fft_log.h \
    libs/kiss/kiss_fftnd.h \
    libs/kiss/kiss_fftndr.h \
    libs/kiss/kiss_fftr.h \
    #libs/wavelet/conv.h \
    #libs/wavelet/cwt.h \
    #libs/wavelet/cwtmath.h \
    #libs/wavelet/hsfft.h \
    #libs/wavelet/real.h \
    #libs/wavelet/wauxlib.h \
    #libs/wavelet/wavefilt.h \
    #libs/wavelet/wavefunc.h \
    #libs/wavelet/wavelib.h \
    #libs/wavelet/wtmath.h \
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
    music/PatternLine.hpp \
    music/PatternReceiver.hpp \
    music/PianoRoll.hpp \
    music/Tapper.hpp \
    tab/Bar.hpp \
    tab/Beat.hpp \
    tab/BeatStructs.hpp \
    tab/Chain.hpp \
    tab/Effects.hpp \
    tab/Note.hpp \
    tab/NoteStructs.hpp \
    tab/Tab.hpp \
    tab/TabStructs.hpp \
    tab/Track.hpp \
    tab/TrackStructs.hpp \
    tab/tools/Base.hpp \
    tab/tools/Commands.hpp \
    tab/tools/GmyFile.hpp \
    tab/tools/GtpFiles.hpp \
    tab/tools/MidiExport.hpp \
    tab/tools/TabClipboard.hpp \
    tab/tools/TabLoader.hpp \
    tab/tools/Threads.hpp \
    tab/ui/BarView.hpp \
    tab/ui/TabViews.hpp \
    tab/ui/TrackView.hpp


DISTFILES += \
    ConsoleLog.qml \
    LICENSE \
    OpenTab.qml \
    README.md \
    qml/config.qml \
    qml/patternInput.qml \
    res/TODO \
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
    libs/filters2/COPYING \
    libs/filters2/README.md \
    main.qml \
    qml/ACFQML.qml \
    qml/CQTQML.qml \
    qml/CepstrumQML.qml \
    qml/ConsoleLog.qml \
    qml/FiltersQML.qml \
    qml/MidiRenderSettings.qml \
    qml/PianoMap.qml \
    qml/STFTQML.qml \
    qml/WaveshapeQML.qml \
    qml/audioHandler.qml \
    qml/consoleLog.qml \
    qml/main.qml \
    qml/midiPlayer.qml \
    qml/tablature.qml \
    qml/tapper.qml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

