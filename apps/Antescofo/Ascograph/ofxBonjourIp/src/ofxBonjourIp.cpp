#include "ofxBonjourIp.h"



ofxBonjourIp::ofxBonjourIp(){
    deviceHostName = "";
    deviceIp = "";
    serverIp = "";
    serverHostName = "";
    connectedToService = false;
    netService = NULL;
    netServiceBrowser = NULL;
}

ofxBonjourIp::~ofxBonjourIp(){
    
    stopService();
    stopDiscoverService();
}


void ofxBonjourIp::startService() {
    
    startService(BONJOUR_TYPE, BONJOUR_NAME, BONJOUR_PORT, BONJOUR_DOMAIN);
}


void ofxBonjourIp::startService( string type, string name, int port, string domain ){
    
    // format parameters
    CFStringRef serviceType = CFStringCreateWithCString(kCFAllocatorDefault, type.c_str(), kCFStringEncodingUTF8);
    CFStringRef serviceName =  CFStringCreateWithCString(kCFAllocatorDefault, name.c_str(), kCFStringEncodingUTF8); // if empty becomes device name
    SInt32 chosenPort = (SInt32) port;
    CFStringRef serviceDomain = CFStringCreateWithCString(kCFAllocatorDefault, domain.c_str(), kCFStringEncodingUTF8);
    
    // start service- async
    netService = CFNetServiceCreate(kCFAllocatorDefault, serviceDomain, serviceType, serviceName, chosenPort);
    CFNetServiceScheduleWithRunLoop(netService, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    
    // creat a client context for a callback function when service successfully publishes
    CFNetServiceClientContext clientContext = { 0, this, NULL, NULL, NULL };
    CFNetServiceSetClient(netService, ofxBonjourIp::NetServicePublishedCallBack, &clientContext); //CFNetServiceSetClient(netService, registerCallback, &clientContext);
    
    
    if (!CFNetServiceRegisterWithOptions(netService, kCFNetServiceFlagNoAutoRename, NULL)) {
        stopService();
        ofLog() << "Could not register Bonjour service";
    }
    
    // do i need to do this?
    CFRelease(serviceType);
    CFRelease(serviceName);
    CFRelease(serviceDomain);
}

void ofxBonjourIp::stopService(){
    
    if(netService) {
        CFNetServiceCancel(netService);
        CFNetServiceUnscheduleFromRunLoop(netService, CFRunLoopGetCurrent(),kCFRunLoopCommonModes);
        CFNetServiceSetClient(netService, NULL, NULL);
        CFRelease(netService);
        netService = NULL;
    }
}

string ofxBonjourIp::getDeviceHostName() {
    
    //return ofxNSStringToString( [[NSProcessInfo processInfo] hostName] );
    return deviceHostName;
}

string ofxBonjourIp::getDeviceIp() {
    
    return deviceIp;
}



// CLIENT - connects to a service automatically
void ofxBonjourIp::discoverService() {
    discoverService(BONJOUR_TYPE, BONJOUR_DOMAIN);
}

void ofxBonjourIp::discoverService( string type, string domain ){
    
    connectedToService = false;
    
    // client will need to know own address
    deviceIp = ofxBonjourIp::GetMyIPAddress();//ofxNSStringToString( [self getIPAddress] );
    
    
    CFStringRef serviceType = CFStringCreateWithCString(kCFAllocatorDefault, type.c_str(), kCFStringEncodingUTF8);
    CFStringRef serviceDomain = CFStringCreateWithCString(kCFAllocatorDefault, domain.c_str(), kCFStringEncodingUTF8);
    
    //
    CFNetServiceClientContext clientContext = { 0, this, NULL, NULL, NULL };
    
    // start browsing service- async
    netServiceBrowser = CFNetServiceBrowserCreate(kCFAllocatorDefault, ofxBonjourIp::NetServiceBrowserCallBack, &clientContext);
    CFNetServiceBrowserScheduleWithRunLoop(netServiceBrowser, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    
    if(!CFNetServiceBrowserSearchForServices(netServiceBrowser, serviceDomain, serviceType, NULL)) {
        ofLog() << "Error browsing for service: " << type << ", " << domain;
        stopDiscoverService();
    }
    
    CFRelease(serviceType);
    CFRelease(serviceDomain);
}

void ofxBonjourIp::stopDiscoverService(){
    
    if(netServiceBrowser) {
        CFNetServiceBrowserInvalidate(netServiceBrowser);
        CFNetServiceBrowserUnscheduleFromRunLoop(netServiceBrowser, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFRelease(netServiceBrowser);
        //CFNetService(netServiceBrowser);
        //CFNetServiceSetClient(netService, NULL, NULL);
        netServiceBrowser = NULL;//
        
        /*CFNetServiceUnscheduleFromRunLoop(netService, CFRunLoopGetCurrent(),kCFRunLoopCommonModes);
         CFNetServiceSetClient(netService, NULL, NULL);
         CFRelease(netService);
         netService = NULL;*/
    }
}

string ofxBonjourIp::getServerHostName() {
    
    return serverHostName;
}

string ofxBonjourIp::getServerIp() {
    
    return serverIp;
}



// http://stackoverflow.com/questions/7072989/iphone-ipad-how-to-get-my-ip-address-programmatically
string ofxBonjourIp::GetMyIPAddress()
{
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    string networkAddress = "";
    string cellAddress = "";
    
    // retrieve the current interfaces - returns 0 on success
    if(!getifaddrs(&interfaces)) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            sa_family_t sa_type = temp_addr->ifa_addr->sa_family;
            if(sa_type == AF_INET || sa_type == AF_INET6) {
                string name = temp_addr->ifa_name; //en0
                string addr = inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr); // pdp_ip0
                
                // ignore localhost "lo0" addresses 127.0.0.1, and "0.0.0.0"
                //if(!ofIsStringInString(name, "lo") && addr != "0.0.0.0") {
                if(addr != "127.0.0.1" && addr != "0.0.0.0") {
                    
                    // can assume here it's "en0" or "en3" or "wlan0" or "pdp_ip0" (cell address)
                    // may need to add in a check to match the name (used to be matched to "en0")
                    ofLog() << "interface name / ip address: " << name << " / " << addr;                    
                    if(name == "pdp_ip0") {
                        // Interface is the cell connection on the iPhone
                        cellAddress = addr;
                    } else {
                        // if(name == "en0") - ignoring the name as this can be different
                        networkAddress = addr;
                    }
                }
                
            }
            temp_addr = temp_addr->ifa_next;
        }
        // Free memory
        freeifaddrs(interfaces);
    }
    
    // will return 0.0.0.0 of it hasn't found address
    string address = (networkAddress != "") ? networkAddress : cellAddress;
    return (address != "") ? address : "0.0.0.0";
}


// netservice callback
void ofxBonjourIp::NetServicePublishedCallBack(CFNetServiceRef theService, CFStreamError* error, void* info) {
    
    if(error->error != 0) ofLog() << "Error: " << error->error;
    
    const char *type = CFStringGetCStringPtr(CFNetServiceGetType(theService), kCFStringEncodingMacRoman); //_ofxBonjourIp._tcp.
    ofLog() << "type: " << type;
    const char *domain = CFStringGetCStringPtr(CFNetServiceGetDomain(theService), kCFStringEncodingMacRoman); //local.
    ofLog() << "domain: " << domain;
    int port = CFNetServiceGetPortNumber(theService); //7777
    ofLog() << "port: " << port;
    const char *name = CFStringGetCStringPtr(CFNetServiceGetName(theService), kCFStringEncodingMacRoman);
    ofLog() << "name: " << name; // name is "" if not defined. should become device name once resolved?
    
    // info has a reference to the class object
    if(info != NULL) {
        
        // set the device ip
        ofxBonjourIp* bonjour = (ofxBonjourIp*)info;
        bonjour->deviceIp = ofxBonjourIp::GetMyIPAddress();
        ofLog() << "device ip: " << bonjour->getDeviceIp();
        
        // set the device name
        char szHostName[255];
        gethostname(szHostName, 255);
        bonjour->deviceHostName = szHostName;
        ofLog() << "device host name: " << bonjour->getDeviceHostName();
        
        ofNotifyEvent(bonjour->publishedServiceEvent,bonjour->deviceIp,bonjour);
    }
    
}


// netservice browser callback - calls whenever service is discovered
void ofxBonjourIp::NetServiceBrowserCallBack(CFNetServiceBrowserRef browser,CFOptionFlags flags,CFTypeRef domainOrService,CFStreamError* error,void* info) {
    ofLog() << "----------------------";
    
    if(error->error != 0) {
        ofLog() << "Error: " << error->error;
        return;
    }
    
    // unresovled still... but can get type and domain (pointless)
    CFNetServiceRef netServiceRef = (CFNetServiceRef) domainOrService; // casting to this thing
    
    
    //service removed/closed
    if (flags & kCFNetServiceFlagRemove) {
        ofLog() << "Service was removed.";
        if(info != NULL) {
            
            // notify service has been removed
            ofxBonjourIp* bonjour = (ofxBonjourIp*)info;
            ofNotifyEvent(bonjour->removedServiceEvent,bonjour->serverIp,bonjour); 
            
            // reset
            bonjour->serverHostName = "";
            bonjour->serverIp = "";
            bonjour->connectedToService = false;
            
            // dont know if i need this? maybe causing a crash after long period of time. I don't think it's been added to the run loop yet either
            //CFNetServiceUnscheduleFromRunLoop(netServiceRef, CFRunLoopGetCurrent(),kCFRunLoopCommonModes);
            //CFNetServiceSetClient(netServiceRef, NULL, NULL);
        }
        return;
    }
    
    // the 'flags' property = 8 when it closes, and 0 when it opens. let's use that for now?
    /*if(flags != 0) {
     ofLog() << "Flag error/ service closed elsewhere (8): " << flags;
     return;
     }*/
    
    
    
    // resolve the service
    CFNetServiceClientContext clientContext = { 0, info, NULL, NULL, NULL };
    CFNetServiceSetClient(netServiceRef, ofxBonjourIp::NetServiceResolvedCallBack, &clientContext);
    CFNetServiceScheduleWithRunLoop(netServiceRef, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    
    if(!CFNetServiceResolveWithTimeout(netServiceRef, 0, NULL)) {
        ofLog() << "Error resolving service";

        CFNetServiceUnscheduleFromRunLoop(netServiceRef, CFRunLoopGetCurrent(),kCFRunLoopCommonModes);
        CFNetServiceSetClient(netServiceRef, NULL, NULL);
    }
}

void ofxBonjourIp::NetServiceResolvedCallBack(CFNetServiceRef theService, CFStreamError* error, void* info) {
    
    //ofLog() << "netService resolved";
    bool serviceResolved = false;
    if(error->error != 0) ofLog() << "Error: " << error->error;
    
    CFArrayRef addresses = CFNetServiceGetAddressing(theService);
    struct sockaddr * socketAddress = NULL;
    
    for(int i=0; i < CFArrayGetCount(addresses); i++) {
        
        // error stopped here once!
        socketAddress = (struct sockaddr *) CFDataGetBytePtr((CFDataRef)CFArrayGetValueAtIndex(addresses, i));
        
        /* Only continue if this is an IPv4 address. */
        //|| socketAddress->sa_family == AF_INET6 ) == 0.0.0.0
        if (socketAddress && socketAddress->sa_family == AF_INET && info != NULL) {
            string addr = inet_ntoa(((struct sockaddr_in *)socketAddress)->sin_addr); // pdp_ip0
            int port = ntohs(((struct sockaddr_in *)socketAddress)->sin_port);
            
            // don't connect to self or 127.0.0.1 or 0.0.0.0
            ofxBonjourIp* bonjour = (ofxBonjourIp*)info;
            if(addr != "0.0.0.0" && addr != "127.0.0.1" && addr != bonjour->deviceIp) {
                
                serviceResolved = true;
                ofLog() << "* Successful connection: " << addr << ", " << port;
                // info has a reference to the class object
                
                // set the server ip
                bonjour->serverIp = addr;
                bonjour->serverHostName = CFStringGetCStringPtr(CFNetServiceGetTargetHost(theService), kCFStringEncodingMacRoman);
                bonjour->connectedToService = true;
                
                ofNotifyEvent(bonjour->discoveredServiceEvent,bonjour->serverIp,bonjour);
                
            } else {
                ofLog() << "Not connecting to self: " << addr << ", " << port;
            }
            
            
        }
    }
    
    // all the service details
    /*const char *host = CFStringGetCStringPtr(CFNetServiceGetTargetHost(theService), kCFStringEncodingMacRoman); //trents-MacBook-Pro.local
     ofLog() << "host: " << host;
     const char *type = CFStringGetCStringPtr(CFNetServiceGetType(theService), kCFStringEncodingMacRoman); //_ofxBonjourIp._tcp.
     ofLog() << "type: " << type;
     const char *domain = CFStringGetCStringPtr(CFNetServiceGetDomain(theService), kCFStringEncodingMacRoman); //local.
     ofLog() << "domain: " << domain;
     int port = CFNetServiceGetPortNumber(theService); //7777
     ofLog() << "port: " << port;
     const char *name = CFStringGetCStringPtr(CFNetServiceGetName(theService), kCFStringEncodingMacRoman);
     ofLog() << "name: " << name; // name is "" if not defined. should become device name once resolved?*/
    
    
    // release stuff (loop and callback)
    if(!serviceResolved) {
        CFNetServiceUnscheduleFromRunLoop(theService, CFRunLoopGetCurrent(),kCFRunLoopCommonModes);
        CFNetServiceSetClient(theService, NULL, NULL);
    }
    //CFNetServiceUnscheduleFromRunLoop(theService, CFRunLoopGetCurrent(),kCFRunLoopCommonModes);
    //CFNetServiceSetClient(theService, NULL, NULL);
    //CFRelease(theService); breaks stuff
    //CFRelease(addresses);
}
