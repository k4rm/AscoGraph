# vim:set ft=cmake:syn on

set(ASCOGRAPH_VERSION_NUMBER "0.087")

set(FAUST 0)
set(USE_HTTPD 0)

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

cmake_minimum_required(VERSION 2.8)

PROJECT(Ascograph)

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

SET(BUILD_SHARED_LIBS OFF)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

###### 
get_filename_component(OPENFRAMEWORKS_CMAKE_DIR_ROOT ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component(OPENFRAMEWORKS_ROOT "${OPENFRAMEWORKS_CMAKE_DIR_ROOT}/../../../../" ABSOLUTE)
set(OPENFRAMEWORKS_ADDONS_ROOT ${OPENFRAMEWORKS_ROOT}/addons/)

set (ANTESCOFO_LIB_PATH ${OPENFRAMEWORKS_ROOT}/../antescofo_New )
set (ANTESCOFO_PARSER_INCL_PATH ${ANTESCOFO_LIB_PATH}/includes ${ANTESCOFO_LIB_PATH}/parser)

set (PATH_MACOS_SDK "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/")
set(CMAKE_OSX_SYSROOT ${PATH_MACOS_SDK})

message(STATUS "Using Antescofo lib path: ${ANTESCOFO_LIB_PATH}")

set (OFX_LIB_PATH ../../../../libs/openFrameworks)
set (OFX_LIB_INCL_PATH ../../../../libs/openFrameworks)
set (OFX_GLEW_INCL_PATH ../../../../libs/glew/include/)
set (OFX_TESS2_INCL_PATH ../../../../libs/tess2/include/)
set (OFX_POCO_INCL_PATH ../../../../libs/poco/include/)
set (OFX_CAIRO_INCL_PATH ../../../../libs/cairo/include/cairo/)
set (OFX_FMOD_INCL_PATH ../../../../libs/fmodex/include/)
set (OFX_FREEIMAGE_INCL_PATH ../../../../libs/FreeImage/include/)
set (OFX_FREETYPE_INCL_PATH ../../../../libs/freetype/include/ ../../../../libs/freetype/include/freetype2/)
set (OFX_RTAUDIO_INCL_PATH ../../../../libs/rtAudio/include/)
set (GST_INCL_PATH ../../../../../gstreamer-1.0.2/)
set (MUTANT_LIBS_PATH ${ANTESCOFO_LIB_PATH}/../../Mutant-libs)
set (SNDFILE_INCL_PATH ${MUTANT_LIBS_PATH}/libsndfile-1.0.25/Buildlib/i386/include/)

set (LIB_MUSICXML_PATH ../libmusicxml-3.0-src/src/)
set (OFX_MUSICXML_INCL_PATH 
		${LIB_MUSICXML_PATH}/interface/ 
		${LIB_MUSICXML_PATH}/elements/
		${LIB_MUSICXML_PATH}/factory/
		${LIB_MUSICXML_PATH}/lib/
		${LIB_MUSICXML_PATH}/visitors/
		${LIB_MUSICXML_PATH}/files/
		${LIB_MUSICXML_PATH}/parser/
		${LIB_MUSICXML_PATH}/antescofo/
)

include_directories( ${OFX_MUSICXML_INCL_PATH} ${OFX_GLEW_INCL_PATH}  ${OFX_LIB_INCL_PATH} 
					${OFX_POCO_INCL_PATH}
					${OFX_TESS2_INCL_PATH}
					${OFX_CAIRO_INCL_PATH}
					${OFX_FMOD_INCL_PATH}
					${OFX_FREEIMAGE_INCL_PATH}
					${OFX_FREETYPE_INCL_PATH}
					${OFX_RTAUDIO_INCL_PATH}
					${OFX_LIB_PATH}/3d/
					${OFX_LIB_PATH}/app/
					${OFX_LIB_PATH}/communication/
					${OFX_LIB_PATH}/events
					${OFX_LIB_PATH}/gl
					${OFX_LIB_PATH}/graphics
					${OFX_LIB_PATH}/math
					${OFX_LIB_PATH}/sound
					${OFX_LIB_PATH}/types
					${OFX_LIB_PATH}/utils
					${OFX_LIB_PATH}/video
					${ANTESCOFO_PARSER_INCL_PATH}
					${GST_INCL_PATH}
					${SNDFILE_INCL_PATH}
)

set (OPENFRAMEWORKS_LIB_SOURCES
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/3d/of3dUtils.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/3d/ofCamera.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/3d/ofEasyCam.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/3d/ofMesh.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/3d/ofNode.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/app/ofAppGlutWindow.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/app/ofAppRunner.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/communication/ofArduino.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/communication/ofSerial.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/events/ofEvents.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/gl/ofFbo.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/gl/ofGLRenderer.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/gl/ofLight.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/gl/ofMaterial.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/gl/ofShader.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/gl/ofTexture.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/gl/ofVbo.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/gl/ofVboMesh.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofBitmapFont.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofCairoRenderer.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofGraphics.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofImage.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofPath.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofPixels.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofPolyline.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofTessellator.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/graphics/ofTrueTypeFont.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/math/ofMath.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/math/ofMatrix3x3.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/math/ofMatrix4x4.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/math/ofQuaternion.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/math/ofVec2f.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/math/ofVec4f.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/sound/ofBaseSoundPlayer.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/sound/ofBaseSoundStream.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/sound/ofFmodSoundPlayer.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/sound/ofOpenALSoundPlayer.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/sound/ofPASoundStream.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/sound/ofRtAudioSoundStream.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/sound/ofSoundPlayer.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/sound/ofSoundStream.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/types/ofBaseTypes.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/types/ofColor.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/types/ofPoint.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/types/ofRectangle.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/utils/ofFileUtils.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/utils/ofLog.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/utils/ofSystemUtils.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/utils/ofThread.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/utils/ofURLFileLoader.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/utils/ofUtils.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofDirectShowGrabber.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQtUtils.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQuickTimeGrabber.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQuickTimePlayer.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQTKitGrabber.mm
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQTKitPlayer.mm
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofVideoGrabber.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofVideoPlayer.cpp
	${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQTKitMovieRenderer.m
)


SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/utils/ofSystemUtils.cpp
	PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQTKitGrabber.mm
	PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQTKitPlayer.mm
	PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ROOT}/libs/openFrameworks/video/ofQTKitMovieRenderer.m
	PROPERTIES COMPILE_FLAGS "-x objective-c++")

if (APPLE)
	set (app_sources_files ../src/mainMacOSX.mm ../src/ofxAntescofog.cpp ../src/ofxAntescofoSim.cpp)
endif(APPLE)

set (app_sources_files ${app_sources_files}
	${OPENFRAMEWORKS_LIB_SOURCES}
	${ANTESCOFO_PARSER_SOURCES}
	${LIB_MUSICXML_PATH}/antescofo/antescofotree.cpp
	${LIB_MUSICXML_PATH}/antescofo/antescofowriter.cpp
	${LIB_MUSICXML_PATH}/antescofo/xml2antescofovisitor.cpp
	${LIB_MUSICXML_PATH}/antescofo/xmlpart2antescofo.cpp
	${LIB_MUSICXML_PATH}/elements/factory.cpp
	${LIB_MUSICXML_PATH}/elements/unrolled_xml_tree_browser.cpp
	${LIB_MUSICXML_PATH}/elements/versions.cpp
	${LIB_MUSICXML_PATH}/elements/xml.cpp
	${LIB_MUSICXML_PATH}/elements/xml_tree_browser.cpp
	${LIB_MUSICXML_PATH}/factory/musicxmlfactory.cpp
	${LIB_MUSICXML_PATH}/factory/sortvisitor.cpp
	${LIB_MUSICXML_PATH}/files/xmlfile.cpp
	${LIB_MUSICXML_PATH}/files/xmlreader.cpp
	${LIB_MUSICXML_PATH}/interface/libmusicxml.cpp
	${LIB_MUSICXML_PATH}/interface/musicxml2antescofo.cpp
	${LIB_MUSICXML_PATH}/lib/conversions.cpp
	${LIB_MUSICXML_PATH}/lib/rational.cpp
	${LIB_MUSICXML_PATH}//operations/transposition.cpp
	${LIB_MUSICXML_PATH}//parser/xmlparse.cpp
	${LIB_MUSICXML_PATH}//visitors/clefvisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/clonevisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/keysignvisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/keyvisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/metronomevisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/midicontextvisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/notevisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/partsummary.cpp
	${LIB_MUSICXML_PATH}/visitors/timesignvisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/transposevisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/unrolled_clonevisitor.cpp
	${LIB_MUSICXML_PATH}/visitors/xmlvisitor.cpp
	# oscpack is commented here because it is compiled in libantescofo which uses it as well.
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//libs/oscpack/src/ip/IpEndpointName.cpp
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//libs/oscpack/src/ip/posix/NetworkingUtils.cpp
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//libs/oscpack/src/ip/posix/UdpSocket.cpp
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//libs/oscpack/src/osc/OscOutboundPacketStream.cpp
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//libs/oscpack/src/osc/OscPrintReceivedElements.cpp
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//libs/oscpack/src/osc/OscReceivedElements.cpp
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//libs/oscpack/src/osc/OscTypes.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//src/ofxOscBundle.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//src/ofxOscMessage.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//src/ofxOscReceiver.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc//src/ofxOscSender.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxColorPicker/ofxColorPicker.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLPage.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLPageTabs.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLTicker.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLTrack.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLTrackHeader.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLZoomer.cpp
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLAudioTrack.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTimeline.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLKeyframes.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLFlags.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/BeatKeyframes.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/Keyframes.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLAntescofoSim.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLAntescofoAction.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLAntescofoNote.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLBeatTicker.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLBeatJump.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/BeatCurve.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLBeatCurves.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLBeatKeyframes.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/Curves.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/Keyframes.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/BeatKeyframes.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLMultiCurves.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLZoomer2D.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLAccompAudioTrack.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxModifierKeys/src/ofxModifierKeys_impl_mac.mm
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTextInputField/src/ofxTextInputField.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMSAInteractiveObject/ofxMSAInteractiveObject.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxXmlSettings/src/ofxXmlSettings.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxXmlSettings/libs/tinyxml.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxXmlSettings/libs/tinyxmlerror.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxXmlSettings/libs/tinyxmlparser.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLVideoTrack.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLColorTrack.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLBangs.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxHotKeys_impl_mac.mm
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLCurves.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLLFO.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLImageTrack.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLVideoThumb.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxRemoveCocoaMenu.mm
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimecode/src/ofxTimecode.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/ofxBaseMidi.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/ofxMidi.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/ofxMidiIn.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/ofxMidiMessage.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/ofxMidiOut.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMSATimer/src/ofxMSATimer.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLSwitches.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLInOut.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLColors.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLImageSequence.cpp 
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src/ofxTLImageSequenceFrame.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/desktop/rtmidi/RtMidi.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/desktop/ofxRtMidiOut.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/desktop/ofxRtMidiIn.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasing.cpp 
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingExpo.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingBack.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingLinear.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingBounce.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingQuad.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingCirc.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingQuart.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingCubic.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingQuint.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingElastic.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings/ofxEasingSine.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/ofxTween.cpp 
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/GLView.mm
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/GLWindow.mm
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/ofxCocoaDelegate.mm
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/ofxCocoaWindow.mm
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCodeEditor/src/ofxCodeEditor.mm
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxConsole/src/ofxConsole.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/libs/ofOpenALSoundPlayer_TimelineAdditions/ofOpenALSoundPlayer_TimelineAdditions.cpp
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/libs/kiss/src/kiss_fft.c
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/libs/kiss/src/kiss_fftr.c
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidiParser/ofxMidiParser.mm
	#${OPENFRAMEWORKS_ADDONS_ROOT}/ofxHttpServer//src/ofxHTTPServer.cpp
	#${OPENFRAMEWORKS_ADDONS_ROOT}/
)

SET_SOURCE_FILES_PROPERTIES(../src/mainMacOSX.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(../src/ofxAntescofog.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(../src/ofxAntescofoSim.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLAntescofoSim.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLAntescofoAction.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLAntescofoNote.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLBeatTicker.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/ofxTLBeatJump.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidiParser/ofxMidiParser.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")

# ofxCocoaWindow rocks (compared to others ofxCocoa*)
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/GLView.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/GLWindow.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/ofxCocoaDelegate.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
SET_SOURCE_FILES_PROPERTIES(${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/ofxCocoaWindow.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")

# scintilla
file ( GLOB_RECURSE SCINTILLA_SOURCES
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCodeEditor/src/scintilla/*/*.cxx
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCodeEditor/src/scintilla/*/*.mm
)

set(OPENFRAMEWORKS_INCLUDES_SEARCH_PATHS 
	../src/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxRange/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMSATimer/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimecode/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxXmlSettings/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxXmlSettings/libs
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTween/src/Easings
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTextInputField/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimelineTracks/ofxAntescofoNote/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/libs/ofOpenALSoundPlayer_TimelineAdditions/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/libs/kiss/include
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/libs/sndfile/include
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidi/src/desktop/rtmidi/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxColorPicker
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxUI/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMSAInteractiveObject/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc/libs/oscpack/src/osc/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc/libs/oscpack/src/ip
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxOsc/libs/oscpack/src/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxModifierKeys/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCocoaWindow/src/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxConsole/src/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCodeEditor/src/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxConsole/src/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCodeEditor/src/scintilla/src
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCodeEditor/src/scintilla/include
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCodeEditor/src/scintilla/lexlib
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxCodeEditor/src/scintilla/cocoa
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidiParser/
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxHttpServer/libs/microhttpd/include
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxHttpServer/src/

	${OPENFRAMEWORKS_ROOT}/libs/glu/include
	${OPENFRAMEWORKS_ROOT}/libs/glut/include
	/usr/include/
	CACHE STRING "OpenFrameworks includes search path")

if(APPLE)
    set(OPENFRAMEWORKS_LIBRARIES_SEARCH_PATHS 
    				     ${OPENFRAMEWORKS_ROOT}/libs/openFrameworksCompiled/lib/osx/
                                     ${OPENFRAMEWORKS_ROOT}/libs/fmodex/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/cairo/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/FreeImage/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/freetype/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/glee/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/glu/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/glu/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/glew/lib/osx
			   	     ${OPENFRAMEWORKS_ROOT}/libs/glut/lib/osx
				     ${OPENFRAMEWORKS_ROOT}/libs/glut/lib/osx/GLUT.framework/
                                     ${OPENFRAMEWORKS_ROOT}/libs/gstappsink/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/poco/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/tess2/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/rtAudio/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/unicap/lib/osx
                                     ${OPENFRAMEWORKS_ROOT}/libs/videoInput/lib/osx
                                     ${OPENFRAMEWORKS_ADDONS_ROOT}/ofxTimeline/libs/sndfile/lib/osx/${SUBLIBDIRSUFFIX}
				     ${OPENFRAMEWORKS_ADDONS_ROOT}/ofxHttpServer/libs/

                                     CACHE STRING "OpenFrameworks library search paths")

elseif( "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)
  if ( ${ARCH} MATCHES "x86_64" )
    set (SUBLIBDIRSUFFIX "linux64")
  else()
    set(SUBLIBDIRSUFFIX "linux")
  endif()

     set(OPENFRAMEWORKS_LIBRARIES_SEARCH_PATHS 
                                     ${OPENFRAMEWORKS_ROOT}/libs/fmodex/lib/${SUBLIBDIRSUFFIX}
                                     ${OPENFRAMEWORKS_ROOT}/libs/glu/lib/${SUBLIBDIRSUFFIX}
				     ${OPENFRAMEWORKS_ROOT}/libs/glut/lib/${SUBLIBDIRSUFFIX}
                                     ${OPENFRAMEWORKS_ROOT}/libs/gstappsink/lib/${SUBLIBDIRSUFFIX}
                                     ${OPENFRAMEWORKS_ROOT}/libs/rtAudio/lib/${SUBLIBDIRSUFFIX}
                                     /usr/lib/
                                     CACHE STRING "OpenFrameworks library search paths")
endif() 

if ( APPLE )
    set (CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
    set (CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".a" ".lib" ".so")
    #set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-m64")
    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-m32 -arch i386")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -mmacosx-version-min=10.6")
    set (CMAKE_OSX_ARCHITECTURES "i386")
    #set (CMAKE_OSX_ARCHITECTURES "x86_64")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG=1 -DTARGET_MAX=1 -DTARGET_PD=2 -DTARGET_STANDALONE_FILE=3 -DTARGET_MAXSDK -DARCHI_MAC_OS=4 -DARCHI_LINUX=5 -DARCHI_WINDOWS=6 -DTARGET_ASCOGRAPH=7 -DANTESCOFO_TARGET=TARGET_ASCOGRAPH -DANTESCOFO_ARCHI=ARCHI_MAC_OS")
elseif( "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set (CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
    set (CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
endif ()

set (CMAKE_LIBRARY_PATH ${OPENFRAMEWORKS_LIBRARIES_SEARCH_PATHS} ${CMAKE_LIBRARY_PATH} /usr/lib /usr/local/lib ${ANTESCOFO_LIB_PATH})
set (CMAKE_INCLUDE_PATH ${OPENFRAMEWORKS_INCLUDES_SEARCH_PATHS} /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain//usr/include/ /usr/include /usr/local/include )

message(STATUS "Looking for libraries in : ${OPENFRAMEWORKS_LIBRARIES_SEARCH_PATHS}")

include_directories(${CMAKE_INCLUDE_PATH})

#######################################################
# Find library and include 
#
macro(of_find_include includefile)
  find_path(${includefile}_INCPATH NAMES ${includefile} PATHS ${OPENFRAMEWORKS_INCLUDES_SEARCH_PATHS})
  if ( ${${includefile}_INCPATH} STREQUAL "${includefile}_INCPATH-NOTFOUND")
    message(FATAL_ERROR "Cannot find ${includefile}")
  else()
    message(STATUS "Found ${includefile} in ${${includefile}_INCPATH}")
  endif()
  set (OPENFRAMEWORKS_INCLUDES ${OPENFRAMEWORKS_INCLUDES} ${${includefile}_INCPATH})
endmacro(of_find_include)

macro(of_find_library libname)
  string(TOLOWER ${libname} libnamelow)
     
  find_library(${libname}_LIB NAMES ${libname} PATHS ${OPENFRAMEWORKS_LIBRARIES_SEARCH_PATHS})
  if (NOT ${libname}_LIB)
    find_library(${libname}_LIB NAMES ${libnamelow} PATHS ${OPENFRAMEWORKS_LIBRARIES_SEARCH_PATHS})
  endif()
  
  if (NOT ${libname}_LIB)
    message(FATAL_ERROR "Cannot find '${libname}' library")
  else()
    message(STATUS "Found '${libname}' library in ${${libname}_LIB}")
    set(OPENFRAMEWORKS_LIBRARIES ${OPENFRAMEWORKS_LIBRARIES} ${${libname}_LIB})
  endif()
endmacro()

macro(of_find_lib_and_include libname incname)
  of_find_library(${libname})
  of_find_include(${incname})
endmacro(of_find_lib_and_include)


#######################################################
# Find glut library
#
macro(of_find_glut)
  # find GLUT 
  include(FindGLUT)
  if ( GLUT_FOUND )
      message ( STATUS "Found GLUT ${GLUT_LIBRARIES}")
      set(OPENFRAMEWORKS_INCLUDES ${OPENFRAMEWORKS_INCLUDES} ${GLUT_INCLUDE_DIR})
      set(OPENFRAMEWORKS_LIBRARIES ${OPENFRAMEWORKS_LIBRARIES} ${GLUT_LIBRARIES})
  else ( GLUT_FOUND )
      message ( FATAL_ERROR  "Cannot find GLUT")
  endif (GLUT_FOUND)
endmacro(of_find_glut)


if ( APPLE )
    of_find_library("quicktime")
    of_find_library("accelerate")
    of_find_library("veclib")
    of_find_library("qtkit")
    of_find_library("coreaudio")
    of_find_library("corevideo")
    of_find_library("coremidi")
    of_find_library("opengl")
    of_find_library("carbon")
    of_find_library("quartzcore")
    of_find_library("cocoa")
    of_find_library("PocoFoundation")
    of_find_library("PocoCrypto")
    of_find_library("PocoUtil")
    of_find_library("PocoNet")
    of_find_library("PocoNetSSL")
    of_find_library("PocoData")
    of_find_library("PocoXML")
    of_find_library("PocoZIp")
    of_find_library("PocoDataODBC")
    of_find_library("PocoDataSQLite")
    of_find_library("fmodex")
    of_find_library("FreeImage")
    of_find_library("cairo")
    of_find_library("cairo-script-interpreter")
    of_find_library("pixman-1")
    of_find_library("freetype")
    of_find_library("tess2")
    of_find_library("glew")
    of_find_library("glut")
    of_find_library("rtAudio")
    of_find_library("sndfile")
    set (CMAKE_FIND_LIBRARY_SUFFIXES ".a" )
    #of_find_library("microhttpd")
    of_find_library("antescofo")
    set (CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".a" ".lib" ".so")
    of_find_library("openal")
    of_find_library("stdc++")
endif(APPLE)


	
of_find_glut()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSCI_NAMESPACE -DSCI_LEXER ")

set( MACOSX_RESOURCE_FILES
	${CMAKE_SOURCE_DIR}/../bin/data/info_bar_bg.png
	${CMAKE_SOURCE_DIR}/../bin/data/note.png
	${CMAKE_SOURCE_DIR}/../bin/data/logo_inria.png
	${CMAKE_SOURCE_DIR}/../bin/data/logo_ircam.png
	${CMAKE_SOURCE_DIR}/../bin/data/Icon.icns
	${CMAKE_SOURCE_DIR}/../bin/data/DroidSansMono.ttf
)

set( MACOSX_RESOURCE_FILES_FONTS
	"${CMAKE_SOURCE_DIR}/../bin/data/GUI/NewMedia Fett.ttf"
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/Ascograph.xml
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/defaultColors.xml
	${CMAKE_SOURCE_DIR}/../bin/data/_Page_One_trackPositions.xml 
	${CMAKE_SOURCE_DIR}/../bin/data/timeline0_Page_One_trackPositions.xml 
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/stop_.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/stop_down.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/stop_over.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/stop_on.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/play_.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/play_on.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/play_over.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/play_down.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/prev_.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/prev_on.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/prev_down.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/prev_over.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/next_.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/next_on.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/next_over.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/next_down.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/start_.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/start_on.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/start_over.png
	${CMAKE_SOURCE_DIR}/../bin/data/GUI/start_down.png
	)


# create an executable with all source files
add_executable( ${PROJECT_NAME}
		MACOSX_BUNDLE
		${app_sources_files}
		${OFX_SOURCE_FILES} 
		${SCINTILLA_SOURCES} 
		${OFX_INCLUDES_DIRS}
		${MACOSX_RESOURCE_FILES}
		${MACOSX_RESOURCE_FILES_FONTS}
)

SET_SOURCE_FILES_PROPERTIES(
	${MACOSX_RESOURCE_FILES}
	PROPERTIES
	MACOSX_PACKAGE_LOCATION Resources
)
SET_SOURCE_FILES_PROPERTIES(
	${MACOSX_RESOURCE_FILES_FONTS}
	PROPERTIES
	MACOSX_PACKAGE_LOCATION Resources/GUI/
)
# WTF is this? set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS " -sectcreate __TEXT __info_plist Info.plist ")

set( MACOSX_BUNDLE_ICON_FILE Icon.icns )

set (ANTESCOFO_VERSION "")
execute_process(COMMAND sh -c "cd ${ANTESCOFO_PARSER_PATH} && PROJECT_DIR=${ANTESCOFO_PARSER_PATH} ./script_gitversion.sh" OUTPUT_VARIABLE ANTESCOFO_VERSION)
message("Building with Antescofo ${ANTESCOFO_VERSION}")

set_target_properties(${PROJECT_NAME} PROPERTIES
	MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} Version ${ASCOGRAPH_VERSION_NUMBER}, Copyright 2013 Thomas Coffy."
	MACOSX_BUNDLE_ICON_FILE ${MACOSX_BUNDLE_ICON_FILE}
	MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_NAME}"
	MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} Version ${ASCOGRAPH_VERSION_NUMBER}"
	MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${ASCOGRAPH_VERSION_NUMBER}
	MACOSX_BUNDLE_BUNDLE_VERSION ${ASCOGRAPH_VERSION_NUMBER}
	MACOSX_BUNDLE_COPYRIGHT "Copyright 2013, Thomas Coffy\nSee http://repmus.ircam.fr/mutant/ascograph\n\nbuilt with Antescofo ${ANTESCOFO_VERSION}"
)

message("fmodex: ${OPENFRAMEWORKS_ROOT}/libs/fmodex/lib/osx/libfmodex.dylib")
message("resources: ${MACOSX_RESOURCE_FILES}")

message(STATUS "CFLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "LDLAGS: ${CMAKE_LD_FLAGS}")

SET_SOURCE_FILES_PROPERTIES(
	${OPENFRAMEWORKS_ROOT}/libs/fmodex/lib/osx/libfmodex.dylib
	PROPERTIES
	MACOSX_PACKAGE_LOCATION Frameworks
)

SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH @executable_path/../Frameworks/libfmodex.dylib)

#set (${OPENFRAMEWORKS_LIBRARIES} ${OPENFRAMEWORKS_LIBRARIES}) ${OPENFRAMEWORKS_ADDONS_ROOT}/ofxHttpServer/libs/libmicrohttpd.a )

target_link_libraries(${PROJECT_NAME} ${OPENFRAMEWORKS_LIBRARIES})

# link with cv libraries and openFrameworks dependencies
#set ( libs ${ofx_libs} ${cv_LIBRARIES})
#target_link_libraries(t ${libs} )

#add_library(fmodlib SHARED ${OPENFRAMEWORKS_ROOT}/libs/fmodex/lib/osx/libfmodex.dylib)
#install (TARGETS fmodlib DESTINATION Frameworks)
#install(TARGETS  BUNDLE DESTINATION ../bin/Contents/Frameworks)

ADD_CUSTOM_COMMAND(
#TARGET ${PROJECT_NAME} PRE_BUILD COMMAND "grep GIT_LAST_REPO_TAG ${ANTESCOFO_PARSER_PATH}/includes/gitDataAutoGenerated.h | sed 's/.*@\"\([a-z0-9].*\)\"/\\1/' > /tmp/out"
    TARGET ${PROJECT_NAME}
    POST_BUILD
		COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/../bin/${PROJECT_NAME}.app/Contents/Frameworks/
		COMMAND cp -f ${CMAKE_SOURCE_DIR}/../../../../libs/fmodex/lib/osx/libfmodex.dylib ${CMAKE_SOURCE_DIR}/../bin/${PROJECT_NAME}.app/Contents/Frameworks
		COMMAND install_name_tool -change ./libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib ${CMAKE_SOURCE_DIR}/../bin/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}
		COMMAND sed "s/ASCOGRAPHVERSION/${ASCOGRAPH_VERSION_NUMBER}/g" Info.plist_orig > Info.plist
		COMMAND codesign -f -v --deep -s "Thomas Coffy" bin/Ascograph.app/Contents/Frameworks/libfmodex.dylib --all-architectures
		COMMAND codesign -f -v --deep -s "Thomas Coffy" bin/Ascograph.app/Contents/MacOS/Ascograph  --all-architectures
)


###################################################################
SET_SOURCE_FILES_PROPERTIES(../src/midiconvert.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
find_library(MIDICONVERT_LIB NAMES cocoa PATHS ${OPENFRAMEWORKS_LIBRARIES_SEARCH_PATHS})
add_executable( midi2antescofo
	${OPENFRAMEWORKS_ADDONS_ROOT}/ofxMidiParser/ofxMidiParser.mm
	../src/midiconvert.cpp
	${MIDICONVERT_LIB}
)

target_link_libraries(midi2antescofo ${MIDICONVERT_LIB})
