#pragma once

#include "ofMain.h"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <CFNetwork/CFNetwork.h>

/*
 ofxBonjourIp created by Trent Brooks.
 - Uses CFNetServices (https://developer.apple.com/library/mac/#documentation/CoreFoundation/Reference/CFNetServiceRef/Reference/reference.html) instead of NSNetServices, as this is recommended for C and C++ applications. So no need to rename .cpp files to .mm or anything. 
 - ios examples needs the CFNetwork framework added to your project. Under TARGETS > BonjourIp, under the 'Build Phases' tab, where it says 'Link binary with libraries', click '+' and select the 'CFNetwork.framework'.
*/

/* TODO::
 - fix for multiple devices connecting to a server. Currently only single connection between 2 devices for 'pairing'. (just make a vector with all resolved ips's)
 */



// defaults for making device discoverable
#define BONJOUR_TYPE "_ofxBonjourIp._tcp."
#define BONJOUR_NAME "" // becomes device name
#define BONJOUR_PORT 7777
#define BONJOUR_DOMAIN "local"


class ofxBonjourIp {
	
public:
    
    ofxBonjourIp();
	~ofxBonjourIp();
    
    
    // BROADCAST SERVICE- MAKES DEVICE DISCOVERABLE
    void startService(); //uses defaults
    void startService( string type, string name, int port, string domain = "" );
    void stopService();
    CFNetServiceRef netService; // the CFNetwork service
    
    // returns name of this device- can use this to connect via osc
    string getDeviceHostName();
    string deviceHostName;
    
    // returns ip address of this device- can use this to connect via osc
    string getDeviceIp();
    string deviceIp;
    
    
    // CLIENT- FIND ANOTHER DEVICE (AUTO CONNECTS)
    void discoverService(); //uses defaults
    void discoverService( string type, string domain );
    void stopDiscoverService();
    bool connectedToService;
    bool isConnectedToService() { return connectedToService; }
    CFNetServiceBrowserRef netServiceBrowser; // the CFNetwork discovery browser
    
    // returns name of server- can use this to connect via osc
    string getServerHostName();
    string serverHostName;
    
    // returns ip address of server- can use this to connect via osc
    string getServerIp();
    string serverIp;

    // events
    ofEvent<string> publishedServiceEvent;
    ofEvent<string> discoveredServiceEvent;
    ofEvent<string> removedServiceEvent;
    template <class ListenerClass>
	void addEventListeners(ListenerClass * listener){
        ofAddListener(publishedServiceEvent,listener,&ListenerClass::onPublishedService);
        ofAddListener(discoveredServiceEvent,listener,&ListenerClass::onDiscoveredService);
        ofAddListener(removedServiceEvent,listener,&ListenerClass::onRemovedService);
    };
    template <class ListenerClass>
	void removeEventListeners(ListenerClass * listener){
        ofRemoveListener(publishedServiceEvent,listener,&ListenerClass::onPublishedService);
        ofRemoveListener(discoveredServiceEvent,listener,&ListenerClass::onDiscoveredService);
        ofRemoveListener(removedServiceEvent,listener,&ListenerClass::onRemovedService);
    };

    // static internal helper methods - don't call these
    static string GetMyIPAddress();
    static void NetServicePublishedCallBack(CFNetServiceRef theService,CFStreamError* error,void* info);
    static void NetServiceBrowserCallBack(CFNetServiceBrowserRef browser,CFOptionFlags flags,CFTypeRef domainOrService,CFStreamError* error,void* info);
    static void NetServiceResolvedCallBack(CFNetServiceRef theService, CFStreamError* error, void* info);

};

