/**
 * ofxTimeline
 * openFrameworks graphical timeline addon
 *
 * Copyright (c) 2011-2012 James George
 * Development Supported by YCAM InterLab http://interlab.ycam.jp/en/
 * http://jamesgeorge.org + http://flightphase.com
 * http://github.com/obviousjim + http://github.com/flightphase
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ofxTLAccompAudioTrack.h"
#include "ofxTimeline.h"

float playHeadX = 0.;
float fakeSpeed = 0;
float fakeEndIndex = 0;
unsigned long long fakeStart = 0;
float fakeRateMS = 1;
unsigned long long lastInstant = 0;
float boundsy = 0;
float boundsh = 0;

ofRange playheadBounds;

class AlignMarker {
public:
	AlignMarker(long ms_) : ms(ms_), selected(false) {}
	long ms;
	ofRectangle rect;
	bool selected;
};
vector<AlignMarker> markers;

ofxTLAccompAudioTrack::ofxTLAccompAudioTrack(){
	shouldRecomputePreview = false;
    soundLoaded = false;
	lastFFTPosition = 0;
	defaultFFTBins = 256;
	maxBinReceived = 0;
}

ofxTLAccompAudioTrack::~ofxTLAccompAudioTrack(){

}

bool ofxTLAccompAudioTrack::loadSoundfile(string filepath){
	soundLoaded = false;
	if(player.loadSound(filepath, false)){
    	soundLoaded = true;
		soundFilePath = filepath;
		shouldRecomputePreview = true;
    }
	return soundLoaded;
}
 
string ofxTLAccompAudioTrack::getSoundfilePath(){
	return soundFilePath;
}

bool ofxTLAccompAudioTrack::isSoundLoaded(){
	return soundLoaded;
}

float ofxTLAccompAudioTrack::getDuration(){
	return player.getDuration();
}

void ofxTLAccompAudioTrack::update()
{
	bool debug = false;
	//cout << "ofxTLAccompAudioTrack:: update" <<  player.getPosition() << endl;
	if (boundsy != bounds.y || boundsh != bounds.height) {
		boundsy = bounds.y;
		boundsh = bounds.height;
		computePreview();
	}

	// assign playHeadX
	//measure time elapsed since last update which will give us where we are
	unsigned long long nownow = ofGetSystemTime();
	unsigned long long dif = nownow - lastInstant;
	if (debug) cout << "update: dif=" << dif << " fakeSpeed:" << fakeSpeed << " fakeRateMS:" << fakeRateMS << " playHeadX:" << playHeadX << endl;
	if (!dif) return;
	playHeadX += dif * fakeSpeed * fakeRateMS;
	lastInstant = nownow;
	if (!playHeadX)
		playheadBounds = zoomBounds;
	else {
		ofRange oldz = playheadBounds;
		ofRange z = playheadBounds;
		float pos = playHeadX / fakeEndIndex; //timeline->screenXtoNormalizedX(playHeadX);
		//cout << endl << "pos:"<< pos <<" got zoomrange: "<< z.min << "->"<< z.max;
		// continuous scrolling : keep playhead on center
		float c = z.center(); 
		float d = pos  - c;

		z.min = ofClamp(z.min + d, 0, 1); z.max = ofClamp(z.max + d, 0, 1);
		if (z.min == .0 && z.span() < oldz.span())
			z.max = oldz.max - oldz.min;
		if (z.max == 1. && z.span() < oldz.span())
			z.min = z.max - oldz.max + oldz.min;

		if (debug) cout <<"update: headX:"<< playHeadX <<" playheadBounds: ["<< z.min << " - "<< z.max << "]"<<endl;
		playheadBounds = z;
	}
	/*
	else if (!playheadBounds.contains(playHeadX)) {
		float s = playheadBounds.span();
		float d = playHeadX - playheadBounds.center();
		playheadBounds.min -= d + 30;
		playheadBounds.max = playheadBounds.min + s;
	}
	*/

	/*
	float pos = player.getPosition();
	if (!zoomBounds.contains(pos) || playHeadX != pos) {
		//recomputePreview();
		playHeadX = pos;
	}
	*/
}

void ofxTLAccompAudioTrack::update(ofEventArgs& args){
	if(player.getPosition() < lastPercent){
		ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
		ofNotifyEvent(events().playbackLooped, args);
	}
	lastPercent = player.getPosition();
	/*
	
    //currently only supports timelines with duration == duration of player
	if(lastPercent < timeline->getInOutRange().min){
		player.setPosition(timeline->getInOutRange().min);
	}
	else if(lastPercent > timeline->getInOutRange().max){
		if(timeline->getLoopType() == OF_LOOP_NONE){
			player.setPosition(timeline->getInOutRange().max);
			cout << "ofxTLAccompAudioTrack::update stop()" << endl;
			stop();
		}
		else{
			player.setPosition(timeline->getInOutRange().min);
		}
	}
	*/
	
    //timeline->setCurrentTimeSeconds(player.getPosition() * player.getDuration());
}
 
void ofxTLAccompAudioTrack::draw(){
	
	if(!soundLoaded || player.getBuffer().size() == 0){
		ofPushStyle();
		ofSetColor(timeline->getColors().disabledColor);
		ofRectangle(bounds);
		ofPopStyle();
		return;
	}
		
	//if(shouldRecomputePreview || viewIsDirty){ recomputePreview(); }
	if(shouldRecomputePreview)// || viewIsDirty)
		computePreview();

	//cout << "ofxTLAccompAudioTrack::draw" << endl;

    ofSetColor(255, 255, 255, 205);
    ofFill();
    ofRect(bounds);
    ofPushStyle();
    //ofSetColor(timeline->getColors().keyColor);
    ofSetColor(0, 0 , 0, 255);
    ofNoFill();
    
    //for(int i = 0; i < previews.size(); i++){
    //for(int i = zoomBounds.min; i < previews.size() && i < zoomBounds.max; i++ ){
#if 0
    for(int i = playheadBounds.min; i < previews.size() && i < playheadBounds.max; i++ ){
        ofPushMatrix();
        //ofTranslate( normalizedXtoScreenX(computedZoomBounds.min, zoomBounds)  - normalizedXtoScreenX(zoomBounds.min, zoomBounds), 0, 0);
        ofTranslate( normalizedXtoScreenX(0, zoomBounds)  - normalizedXtoScreenX(zoomBounds.min, zoomBounds), 0, 0);
        //ofScale(computedZoomBounds.span()/zoomBounds.span(), 1, 1);
        ofScale(1/zoomBounds.span(), 1, 1);
        previews[i].draw();
        previews[i +(previews.size()/2)*(player.getNumChannels()-1)].draw();
        ofPopMatrix();
    }
#else
    //cout << "playheadBounds: ["<< playheadBounds.min << " - " << playheadBounds.max << "]"<<endl;
    for(int i = playheadBounds.min; i < previews.size() && i < playheadBounds.max; i++ ){
        ofPushMatrix();
        //ofTranslate( normalizedXtoScreenX(computedZoomBounds.min, zoomBounds)  - normalizedXtoScreenX(zoomBounds.min, zoomBounds), 0, 0);
        ofTranslate( normalizedXtoScreenX(0, playheadBounds)  - normalizedXtoScreenX(playheadBounds.min, playheadBounds), 0, 0);
        //ofScale(computedZoomBounds.span()/zoomBounds.span(), 1, 1);
        ofScale(1/playheadBounds.span(), 1, 1);
	//if (i >= bounds.x || i <= bounds.x + bounds.width)
		previews[i].draw();
        previews[i +(previews.size()/2)*(player.getNumChannels()-1)].draw();
        ofPopMatrix();
    }
#endif
    ofPopStyle();
	
	if(getIsPlaying() || timeline->getIsPlaying()){
		ofPushStyle();
		
		//will refresh fft bins for other calls too
		vector<float>& bins = getFFTSpectrum(defaultFFTBins);
		float binWidth = bounds.width / bins.size();
		//find max
		float averagebin = 0 ;
		for(int i = 0; i < bins.size(); i++){
			maxBinReceived = MAX(maxBinReceived, bins[i]);
			averagebin += bins[i];
		}
		averagebin /= bins.size();
		
		ofFill();
		ofSetColor(timeline->getColors().disabledColor, 120);
		for(int i = 0; i < bins.size(); i++){
			float height = bounds.height * bins[i]/maxBinReceived;
			float y = bounds.y + bounds.height - height;
			ofRect(bounds.x + i*binWidth, y, binWidth, height);
		}
		
		ofPopStyle();
	}

	// playhead 
	ofPushStyle();
	ofSetColor(0, 0, 0, 255);
	//float x = playHeadX - normalizedXtoScreenX(playheadBounds.min, zoomBounds);
	float x = normalizedXtoScreenX(playHeadX / fakeEndIndex, playheadBounds);
	ofSetLineWidth(2);
	ofLine(x, bounds.y, x, bounds.y + bounds.height);
	ofPopStyle();

	/*
	float pos = player.getPosition();
	if (pos) {
		ofxTLZoomer2D *zoom = (ofxTLZoomer2D*)timeline->getZoomer();
		ofRange z = zoom->getViewRange();
		ofRange oldz = z;
		float c = z.center(); 
		float d = pos - c;

		z.min = ofClamp(z.min + d, 0, 1); z.max = ofClamp(z.max + d, 0, 1);
		if (z.min == .0 && z.span() < oldz.span())
			z.max = oldz.max - oldz.min;
		if (z.max == 1. && z.span() < oldz.span())
			z.min = z.max - oldz.max + oldz.min;
	}*/




	// draw markers:
	for (vector<AlignMarker>::iterator m = markers.begin(); m != markers.end(); m++) {
		//float xn = screenXtoNormalizedX(millisToScreenX(m->ms), playheadBounds);
		float xn = millisToScreenX(m->ms) /  millisToScreenX(player.getDuration()*1000);
		//cout << "ms=" << m->ms << " xn = " << xn << endl;
		if (playheadBounds.contains(xn)) {
			float x = timeline->normalizedXtoScreenX(xn, playheadBounds);
			if (m->selected)
				ofSetColor(255, 0, 0, 255);
			else 
				ofSetColor(0, 0, 0, 255);
			ofLine(x, bounds.y, x, bounds.y+bounds.height);
			m->rect = ofRectangle(x - 5, bounds.y + bounds.height - 12, 10, 10);
			ofSetColor(10, 0, 200, 100);
			ofFill();
			ofRect(m->rect);

			//cout << "m:" << m->ms << endl;
			//ofSetColor(0, 0, 0, 255);
			//timeline->getFont().drawString(ofToString(m->ms), x+1, bounds.y + 30);
		}
	}
	
}

void ofxTLAccompAudioTrack::setMarkers(vector<float>& map_index, vector<float>& map_markers) {
	for (vector<float>:: iterator i = map_markers.begin(); i != map_markers.end(); i++) {
		markers.push_back(*i * 1000);
	}
}


void ofxTLAccompAudioTrack::computePreview(){
	
	previews.clear();
	
	cout << "recomputing view with zoom bounds of " << zoomBounds << endl;
	
	float normalizationRatio = timeline->getDurationInSeconds() / player.getDuration(); //need to figure this out for framebased...but for now we are doing time based
	float trackHeight = bounds.height/(1+player.getNumChannels());
	int numSamples = player.getBuffer().size() / player.getNumChannels();
	int pixelsPerSample = numSamples / bounds.width;
	for(int c = 0; c < player.getNumChannels(); c++){
		ofPolyline preview;
		int lastFrameIndex = 0;
		ofRange fullBounds(0, 1);
		//for(float i = bounds.x; i < bounds.x+bounds.width; i++){
		for(float i = 0; i < millisToScreenX(player.getDuration()*1000); i += 0.04){
			//float pointInTrack = screenXtoNormalizedX( i, zoomBounds ) * normalizationRatio; //will scale the screenX into wave's 0-1.0
			float pointInTrack = screenXtoNormalizedX( i, fullBounds ) * normalizationRatio; //will scale the screenX into wave's 0-1.0
			float trackCenter = bounds.y + trackHeight * (c+1);
			//cout << "pointInTrack: " << pointInTrack << " i:" << i << endl;
			if(pointInTrack <= 1.0){
				//draw sample at pointInTrack * waveDuration;
				int frameIndex = pointInTrack * numSamples;					
				float losample = 0;
				float hisample = 0;
				for(int f = lastFrameIndex; f < frameIndex; f++){
					int sampleIndex = f * player.getNumChannels() + c;
					float subpixelSample = player.getBuffer()[sampleIndex]/32565.0;
					if(subpixelSample < losample) {
						losample = subpixelSample;
					}
					if(subpixelSample > hisample) {
						hisample = subpixelSample;
					}
				}
				if(losample == 0 && hisample == 0){
					preview.addVertex(i, trackCenter);
				}
				else {
					if(losample != 0){
						preview.addVertex(i, trackCenter - losample * trackHeight);
					}
					if(hisample != 0){
						//ofVertex(i, trackCenter - hisample * trackHeight);
						preview.addVertex(i, trackCenter - hisample * trackHeight);
					}
				}
				lastFrameIndex = frameIndex;
			}
			else{
				preview.addVertex(i,trackCenter);
			}
		}
		preview.simplify();
		previews.push_back(preview);
	}
	//computedZoomBounds = zoomBounds;
	shouldRecomputePreview = false;
}



void ofxTLAccompAudioTrack::recomputePreview(){
	
	previews.clear();
	
	//cout << "recomputing view with zoom bounds of " << zoomBounds << endl;
	
	float normalizationRatio = timeline->getDurationInSeconds() / player.getDuration(); //need to figure this out for framebased...but for now we are doing time based
	float trackHeight = bounds.height/(1+player.getNumChannels());
	int numSamples = player.getBuffer().size() / player.getNumChannels();
	int pixelsPerSample = numSamples / bounds.width;
	for(int c = 0; c < player.getNumChannels(); c++){
		ofPolyline preview;
		int lastFrameIndex = 0;
		for(float i = bounds.x; i < bounds.x+bounds.width; i++){
			float pointInTrack = screenXtoNormalizedX( i, zoomBounds ) * normalizationRatio; //will scale the screenX into wave's 0-1.0
			float trackCenter = bounds.y + trackHeight * (c+1);
			if(pointInTrack <= 1.0){
				//draw sample at pointInTrack * waveDuration;
				int frameIndex = pointInTrack * numSamples;					
				float losample = 0;
				float hisample = 0;
				for(int f = lastFrameIndex; f < frameIndex; f++){
					int sampleIndex = f * player.getNumChannels() + c;
					float subpixelSample = player.getBuffer()[sampleIndex]/32565.0;
					if(subpixelSample < losample) {
						losample = subpixelSample;
					}
					if(subpixelSample > hisample) {
						hisample = subpixelSample;
					}
				}
				if(losample == 0 && hisample == 0){
					preview.addVertex(i, trackCenter);
				}
				else {
					if(losample != 0){
						preview.addVertex(i, trackCenter - losample * trackHeight);
					}
					if(hisample != 0){
						//ofVertex(i, trackCenter - hisample * trackHeight);
						preview.addVertex(i, trackCenter - hisample * trackHeight);
					}
				}
				lastFrameIndex = frameIndex;
			}
			else{
				preview.addVertex(i,trackCenter);
			}
		}
		preview.simplify();
		previews.push_back(preview);
	}
	computedZoomBounds = zoomBounds;
	shouldRecomputePreview = false;
}

int ofxTLAccompAudioTrack::getDefaultBinCount(){
//	cout << defaultFFTBins << endl;
	return defaultFFTBins;
}

bool ofxTLAccompAudioTrack::mousePressed(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return false;
	
	for (vector<AlignMarker>::iterator m = markers.begin(); m != markers.end(); m++) {
		if (m->rect.inside(args.x, args.y)) {
			cout << "ms selected:" << m->ms << endl;
			m->selected = true;
			return true;
		}
	}
	return false;
	// change playhead position
	timeline->setPercentComplete(screenXtoNormalizedX(args.x, zoomBounds));

	return false;
}

void ofxTLAccompAudioTrack::mouseMoved(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;

	for (vector<AlignMarker>::iterator m = markers.begin(); m != markers.end(); m++) {
		if (m->selected) {
			cout << "ms selected changin from " << m->ms << endl;
			m->ms = screenXToMillis(normalizedXtoScreenX(screenXtoNormalizedX(args.x, zoomBounds)));
			cout << " to: " << m->ms << endl;
		}
	}
}

void ofxTLAccompAudioTrack::mouseDragged(ofMouseEventArgs& args, long millis){
}

void ofxTLAccompAudioTrack::mouseReleased(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;

	for (vector<AlignMarker>::iterator m = markers.begin(); m != markers.end(); m++) {
		if (m->selected) {
			cout << "ms selected changin from " << m->ms << endl;
			m->ms = screenXToMillis(normalizedXtoScreenX(screenXtoNormalizedX(args.x, zoomBounds)));
			cout << " to: " << m->ms << endl;
			m->selected = false;
		}
	}
}

void ofxTLAccompAudioTrack::keyPressed(ofKeyEventArgs& args){
	if (args.key == OF_KEY_RETURN) {
		long ms = timeline->getCurrentTimeMillis();
		cout << "Marker taken:"<< ms << "ms" << endl;
		markers.push_back(AlignMarker(ms));
	} else if (args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE) {
		markers.clear();
	}
}

void ofxTLAccompAudioTrack::zoomStarted(ofxTLZoomEventArgs& args){
	//ofxTLTrack::zoomStarted(args);
//	shouldRecomputePreview = true;
	if (!playHeadX) playheadBounds = zoomBounds;
}

void ofxTLAccompAudioTrack::zoomDragged(ofxTLZoomEventArgs& args){
	//ofxTLTrack::zoomDragged(args);
	//shouldRecomputePreview = true;
}

void ofxTLAccompAudioTrack::zoomEnded(ofxTLZoomEventArgs& args){
	ofxTLTrack::zoomEnded(args);
	//shouldRecomputePreview = true;

	if (!playHeadX) playheadBounds = zoomBounds;
}

void ofxTLAccompAudioTrack::boundsChanged(ofEventArgs& args){
	cout << "ofxTLAccompAudioTrack::boundsChanged" << endl;
	computePreview();
	//shouldRecomputePreview = true;
}


void ofxTLAccompAudioTrack::fakeStop(){
	cout << "ofxTLAccompAudioTrack:: fakeStop"  << endl;
	fakeSpeed = 0.;

}

void ofxTLAccompAudioTrack::fakePlay(){
	float duration = player.getDuration();

	playHeadX = 0;
	cout << "ofxTLAccompAudioTrack:: fakePlay: duration:" << duration << endl;
	fakeSpeed = 1.;
	fakeEndIndex = millisToScreenX(player.getDuration()*1000);
	fakeRateMS = fakeEndIndex / (player.getDuration() * 1000); // speed: screenx per ms
	fakeStart = ofGetSystemTime();
	lastInstant = fakeStart;
	playheadBounds = zoomBounds;
	cout << "ofxTLAccompAudioTrack:: fakePlay: fakeEnd:" << fakeEndIndex << " fakeRateMS:" << fakeRateMS << " fakeStart:" << fakeStart << endl;
}

void ofxTLAccompAudioTrack::play(){
	cout << "ofxTLAccompAudioTrack:: play " << endl;
	if(!player.getIsPlaying()){
		
//		lastPercent = MIN(timeline->getPercentComplete() * timeline->getDurationInSeconds() / player.getDuration(), 1.0);
		//player.setLoop(timeline->getLoopType() == OF_LOOP_NORMAL);
		player.setLoop(timeline->getLoopType() == OF_LOOP_NONE);
		cout << "ofxTLAccompAudioTrack:: calling play on track " << endl;
		player.setPosition(0);//XXX timeline->getPercentComplete());
		player.play();
		ofAddListener(ofEvents().update, this, &ofxTLAccompAudioTrack::update);
		/* XXX
		ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
		ofNotifyEvent(events().playbackStarted, args);		
		*/
	} else {
		//cout << "ofxTLAccompAudioTrack:: calling NOT play on track " << endl;
		player.stop();
		play();
	}
}

void ofxTLAccompAudioTrack::stop(){
	if(player.getIsPlaying()){

//		cout << "calling stop on track " << endl;
		player.setPaused(true);
		ofRemoveListener(ofEvents().update, this, &ofxTLAccompAudioTrack::update);
		
		ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
		ofNotifyEvent(events().playbackEnded, args);
	}
}

bool ofxTLAccompAudioTrack::togglePlay(){
	cout << "ofxTLAccompAudioTrack::togglePlay" << endl;
	if(getIsPlaying()){
		stop();
	}
	else {
		play();
	}
	return getIsPlaying();
}

bool ofxTLAccompAudioTrack::getIsPlaying(){
    return player.getIsPlaying();
}

void ofxTLAccompAudioTrack::setFakeSpeed(float speed){
	//cout << "ofxTLAccompAudioTrack::setFakeSpeed:"<< speed << endl;
	fakeSpeed = speed;
}

void ofxTLAccompAudioTrack::setSpeed(float speed){
    player.setSpeed(speed);
}

float ofxTLAccompAudioTrack::getSpeed(){
    return player.getSpeed();
}

void ofxTLAccompAudioTrack::setVolume(float volume){
    player.setVolume(volume);
}

void ofxTLAccompAudioTrack::setPan(float pan){
    player.setPan(pan);
}

vector<float>& ofxTLAccompAudioTrack::getFFTSpectrum(int numBins){
	float fftPosition = player.getPosition();
	if(isSoundLoaded() && lastFFTPosition != fftPosition){
		if(defaultFFTBins != numBins){
			maxBinReceived = 0;
			defaultFFTBins = numBins;
		}
		lastFFTPosition = fftPosition;
		fftBins = player.getSpectrum(defaultFFTBins);
	}
	return fftBins;
}

string ofxTLAccompAudioTrack::getTrackType(){
    return "Audio";    
}

   
