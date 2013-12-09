/*
 * ofxHTTPServer.h
 *
 *  Created on: 16-may-2009
 *      Author: art
 */

#ifndef OFXHTTPSERVER_H_
#define OFXHTTPSERVER_H_

#include <cstdarg>
#include <set>

#if defined( __WIN32__ ) || defined( _WIN32 )
   #define MHD_PLATFORM_H
   #include <ws2tcpip.h>
   #include <stdint.h>
#else
   #include <sys/socket.h>
#endif

extern "C" {
	#include "microhttpd.h"
}

#include "Poco/Condition.h"
#include "ofMain.h"
#include <map>

class ofxHTTPServerResponse{
public:
	ofxHTTPServerResponse(){
		errCode=200;
	}
	std::string url;
	ofBuffer response;
	int errCode;
	std::string errStr;
	std::string location; // for redirections
	std::string contentType;  // for contents different than html or text
	std::string referer;

	std::map<std::string,std::string> requestFields;
	std::map<std::string,std::string> uploadedFiles;
};

class ofxHTTPServerListener{
public:
	virtual ~ofxHTTPServerListener(){}
	virtual void getRequest(ofxHTTPServerResponse & response)=0;
	virtual void postRequest(ofxHTTPServerResponse & response)=0;
	virtual void fileNotFound(ofxHTTPServerResponse & response)=0;
};


class ofxHTTPServer {
public:
	static ofxHTTPServer * getServer() {
		return &instance;
	}

	virtual ~ofxHTTPServer();


	void start(unsigned port = 8888, bool threaded=false);
	void stop();
	void setServerRoot(const std::string & fsroot);
	void setUploadDir(const std::string & uploadDir);
	void setCallbackExtensions(const std::string & cb_extensions);
	void setMaxNumberClients(unsigned num_clients);
	void setMaxNumberActiveClients(unsigned num_clients);
	unsigned getNumberClients();

	void setListener(ofxHTTPServerListener & listener);


	static int answer_to_connection(void *cls,
				struct MHD_Connection *connection, const char *url,
				const char *method, const char *version, const char *upload_data,
				size_t *upload_data_size, void **con_cls);

private:
	static ofxHTTPServer instance;

	ofxHTTPServer();
	struct MHD_Daemon *http_daemon;
	std::string fsRoot;
	std::string uploadDir;
	std::set<std::string> callbackExtensions;
	bool callbackExtensionSet;
	unsigned port;
	unsigned numClients;
	unsigned maxClients;
	unsigned maxActiveClients;
	Poco::Condition maxActiveClientsCondition;
	ofMutex maxActiveClientsMutex;
	ofxHTTPServerListener * listener;

	static int print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
	static int get_get_parameters (void *cls, enum MHD_ValueKind kind, const char *key, const char *value);

	static int send_page (struct MHD_Connection *connection, long length, const char* page, int status_code, std::string contentType="");
	static int send_redirect (struct MHD_Connection *connection, const char* location, int status_code);

	static void request_completed (void *cls, struct MHD_Connection *connection, void **con_cls,
	                        enum MHD_RequestTerminationCode toe);

	static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
							const char *filename, const char *content_type,
							const char *transfer_encoding, const char *data, uint64_t off, size_t size);


};

#endif /* OFXHTTPSERVER_H_ */
