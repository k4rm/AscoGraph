/*
 * ofxHTTPServer.cpp
 *
 *  Created on: 16-may-2009
 *      Author: art
 */

#include "ofxHTTPServer.h"
#include <cstring>
#include <fstream>
#include <map>

using namespace std;

ofxHTTPServer ofxHTTPServer::instance;

// Helper functions and structures only used internally by the server
//------------------------------------------------------
static const int GET = 0;
static const int POST = 1;
static const unsigned POSTBUFFERSIZE = 4096;
static const char* CONTENT_TYPE = "Content-Type";

class connection_info{
	static int id;
public:
	connection_info(){
		conn_id = ++id;
	}

	map<string,string> fields;
	map<string,FILE*> file_fields;
	map<string,string> file_to_path_index;
	map<string,string> file_to_key_index;
	int connectiontype;
	bool connection_complete;
	struct MHD_PostProcessor *postprocessor;
	int conn_id;
	char new_content_type[1024];

};

int connection_info::id=0;


// private methods

int ofxHTTPServer::print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
  ofLogVerbose ("ofxHttpServer") << ofVAArgsToString("%s = %s\n", key, value);
  return MHD_YES;
}


int ofxHTTPServer::get_get_parameters (void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	connection_info *con_info = (connection_info*) cls;
	if(key!=NULL && value!=NULL)
	con_info->fields[key] = value;
	return MHD_YES;
}

int ofxHTTPServer::iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                  const char *filename, const char *content_type,
                  const char *transfer_encoding, const char *data, uint64_t off, size_t size)
{
	connection_info *con_info = (connection_info*) coninfo_cls;

	ofLogVerbose("ofxHttpServer") << "processing connection " << con_info->conn_id << endl;
	if (size > 0){
		ofLogVerbose("ofxHttpServer") << "processing post field of size: "<< size <<","<< endl;
		if(transfer_encoding)
			ofLogVerbose("ofxHttpServer") << " transfer encoding: "<< transfer_encoding<< endl;
		if(content_type)
			ofLogVerbose("ofxHttpServer") << ", content: " << content_type<< endl;
		if(filename)
			ofLogVerbose("ofxHttpServer") << ", filename: "<< filename<< endl;

		ofLogVerbose("ofxHttpServer") << ", "  << key << endl;//": " <<data << endl;

		if(!filename){
			 char * aux_data = new char[off+size+1];
			 memset(aux_data,0,off+size+1);
			 if(off > 0)
				memcpy(aux_data,con_info->fields[key].c_str(),off);

			 memcpy(aux_data+off*sizeof(char),data,size);
			 con_info->fields[key] = aux_data;
		}else{
			ofLogVerbose("ofxHttpServer") << "ofxHttpServer:" << "received file" << endl;
			if(con_info->file_fields.find(filename)==con_info->file_fields.end()){
				con_info->file_fields[filename] = NULL;
				string uploadFolder(ofFilePath::join(instance.uploadDir,ofGetTimestampString()));
				ofDirectory (uploadFolder).create();
				string path = ofFilePath::join(uploadFolder, filename);
				con_info->file_fields[filename] = fopen (string(path).c_str(), "ab");
				if(con_info->file_fields[filename] == NULL){
					con_info->file_fields.erase(filename);
					return MHD_NO;
				}
				con_info->file_to_key_index[filename]=key;
				con_info->file_to_path_index[filename] = path;

			}
			if(size>0){
				if (!fwrite (data, size, sizeof(char), con_info->file_fields[filename])){
					ofLogVerbose("ofxHttpServer") << "ofxHttpServer:" << "error on writing" << endl;
					con_info->file_fields.erase(filename);
					return MHD_NO;
				}
			}

		}
	}
	return MHD_YES;


}

void ofxHTTPServer::request_completed (void *cls, struct MHD_Connection *connection, void **con_cls,
                        enum MHD_RequestTerminationCode toe)
{
  connection_info *con_info = (connection_info*) *con_cls;


  if (NULL == con_info){
	  ofLogWarning("ofxHttpServer") << "request completed NULL connection";
	  return;
  }

  if (con_info->connectiontype == POST){
      MHD_destroy_post_processor (con_info->postprocessor);
  }



  delete con_info;
  *con_cls = NULL;

  instance.maxActiveClientsMutex.lock();
  instance.numClients--;
  instance.maxActiveClientsCondition.signal();
  instance.maxActiveClientsMutex.unlock();
}

int ofxHTTPServer::send_page (struct MHD_Connection *connection, long length, const char* page, int status_code, string contentType)
{
  int ret;
  struct MHD_Response *response;


  response = MHD_create_response_from_data (length, (void*) page, MHD_NO, MHD_YES);
  if (!response) return MHD_NO;

  if(contentType!=""){
	  MHD_add_response_header (response,"Content-Type",contentType.c_str());
  }

  ret = MHD_queue_response (connection, status_code, response);
  MHD_destroy_response (response);

  return ret;
}

int ofxHTTPServer::send_redirect (struct MHD_Connection *connection, const char* location, int status_code)
{
  int ret;
  struct MHD_Response *response;


  char data[]="";
  response = MHD_create_response_from_data (0,data, MHD_NO, MHD_YES);
  if (!response) return MHD_NO;

  MHD_add_response_header (response, "Location", location);

  ret = MHD_queue_response (connection, status_code, response);
  MHD_destroy_response (response);

  return ret;
}



// public methods
//------------------------------------------------------
ofxHTTPServer::ofxHTTPServer() {
	callbackExtensionSet = false;
	maxClients = 100;
	numClients = 0;
	maxActiveClients = 4;
	uploadDir = ofToDataPath("",true);
	http_daemon = NULL;
	port = 8888;
	listener = NULL;
}

ofxHTTPServer::~ofxHTTPServer() {
	// TODO Auto-generated destructor stub
}



int ofxHTTPServer::answer_to_connection(void *cls,
			struct MHD_Connection *connection, const char *url,
			const char *method, const char *version, const char *upload_data,
			size_t *upload_data_size, void **con_cls) {

	string strmethod = method;

	connection_info  * con_info;

	// to process post we need several iterations, first we set a connection info structure
	// and return MHD_YES, that will make the server call us again
	if(NULL == *con_cls){
		con_info = new connection_info;

		instance.maxActiveClientsMutex.lock();
		instance.numClients++;
		if(instance.numClients >= instance.maxClients){
			instance.maxActiveClientsMutex.unlock();
			ofFile file503("503.html");
			ofBuffer buf;
			file503 >> buf;
			return send_page(connection, buf.size(), buf.getBinaryBuffer(), MHD_HTTP_SERVICE_UNAVAILABLE);
		}

		if(instance.numClients > instance.maxActiveClients){
			instance.maxActiveClientsCondition.wait(instance.maxActiveClientsMutex);
		}
		instance.maxActiveClientsMutex.unlock();

		// super ugly hack to manage poco multipart post connections as it sets boundary between "" and
		// libmicrohttpd doesn't seem to support that
		string contentType;
		if(MHD_lookup_connection_value(connection, MHD_HEADER_KIND, CONTENT_TYPE)!=NULL)
			contentType = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, CONTENT_TYPE);
		if ( contentType.size()>31 && contentType.substr(0,31) == "multipart/form-data; boundary=\""){
			contentType = "multipart/form-data; boundary="+contentType.substr(31,contentType.size()-32);
			ofLogVerbose("ofxHttpServer") << "changing content type: " << contentType << endl;
			strcpy(con_info->new_content_type,contentType.c_str());
			MHD_set_connection_value(connection,MHD_HEADER_KIND,CONTENT_TYPE,con_info->new_content_type);
		}
		MHD_get_connection_values (connection, MHD_HEADER_KIND, print_out_key, NULL);

		if(strmethod=="GET"){
			con_info->connectiontype = GET;
			MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, get_get_parameters, con_info);
		}else if (strmethod=="POST"){


			con_info->postprocessor = MHD_create_post_processor (connection, POSTBUFFERSIZE, iterate_post, (void*) con_info);

			if (NULL == con_info->postprocessor)
			{
				ofLogVerbose("ofxHttpServer") << "error creating postprocessor" << endl;
			  delete con_info;
			  return MHD_NO;
			}

			con_info->connectiontype = POST;
		}

		*con_cls = (void*) con_info;
		return MHD_YES;
	}else{
		con_info = (connection_info*) *con_cls;
	}


	// second and next iterations
	string strurl = url;
	int ret = MHD_HTTP_SERVICE_UNAVAILABLE;


	// if the extension of the url is that set to the callback, call the events to generate the response
	string extension = ofFilePath::getFileExt(strurl);
	if(instance.callbackExtensionSet && instance.callbackExtensions.find(extension)!=instance.callbackExtensions.end()){

		ofLogVerbose("ofxHttpServer") << method << " serving from callback: " << url << endl;

		ofxHTTPServerResponse response;
		response.url = strurl;
		const char * referer = MHD_lookup_connection_value(connection,MHD_HEADER_KIND,MHD_HTTP_HEADER_REFERER);
		if(referer){
			response.referer = referer;
		}


		if(strmethod=="GET"){
			response.requestFields = con_info->fields;
			if(instance.listener) instance.listener->getRequest(response);
			//ofNotifyEvent(instance.getEvent,response);
			if(response.errCode>=300 && response.errCode<400){
				ret = send_redirect(connection, response.location.c_str(), response.errCode);
			}else{
				ret = send_page(connection, response.response.size(), response.response.getBinaryBuffer(), response.errCode, response.contentType);
			}
		}else if (strmethod=="POST"){
			if (*upload_data_size != 0){
				ret = MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);
				*upload_data_size = 0;

			}else{
				ofLogVerbose("ofxHttpServer") << "upload_data_size =  0" << endl;
				response.requestFields = con_info->fields;
				map<string,string>::iterator it_f;
				for(it_f=con_info->fields.begin();it_f!=con_info->fields.end();it_f++){
					ofLogVerbose("ofxHttpServer") << it_f->first << ", " << it_f->second;
				}
				map<string,FILE*>::iterator it;
				for(it=con_info->file_fields.begin();it!=con_info->file_fields.end();it++){
					  if(it->second!=NULL){
						  fflush(it->second);
						  fclose(it->second);
						  response.uploadedFiles[con_info->file_to_key_index[it->first]]=con_info->file_to_path_index[it->first];
					  }
				}
				//ofNotifyEvent(instance.postEvent,response);
				if(instance.listener) instance.listener->postRequest(response);
				if(response.errCode>=300 && response.errCode<400){
					ret = send_redirect(connection, response.location.c_str(), response.errCode);
				}else{
					ret = send_page(connection, response.response.size(), response.response.getBinaryBuffer(), response.errCode, response.contentType);
				}
			}

		}

	// if the extension of the url is any other try to serve a file
	}else{
		ofLogVerbose("ofxHttpServer") << method << " serving from filesystem: " << strurl << endl;
		if(strurl=="/") strurl = "/index.html";

		ofFile file(instance.fsRoot + strurl,ofFile::ReadOnly,true);
		if(!file.exists()){
			ofxHTTPServerResponse response;
			response.errCode = 404;
			response.url = strurl;

			if(instance.listener) instance.listener->fileNotFound(response);
			if(response.errCode>=300 && response.errCode<400){
				ret = send_redirect(connection, response.location.c_str(), response.errCode);
			}else if(response.errCode!=404){
				ret = send_page(connection, response.response.size(), response.response.getBinaryBuffer(), response.errCode);
			}else{

				cerr << "Error: file could not be opened trying to serve 404.html" << endl;
				ofFile file404("404.html");
				ofBuffer buf;
				file404 >> buf;
				send_page(connection, buf.size(), buf.getBinaryBuffer(), MHD_HTTP_NOT_FOUND);
			}

		}else{
			ofBuffer buf;
			file >> buf;
			ofLogVerbose("ofxHttpServer") << "response: file " << instance.fsRoot << url << " of size " << buf.size() << endl;
			ret = send_page(connection, buf.size(), buf.getBinaryBuffer(), MHD_HTTP_OK);
		}
	}

	return ret;

}


void ofxHTTPServer::start(unsigned _port, bool threaded) {
	port = _port;
	/*if(threaded){
		http_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
				_port, NULL, NULL,
				&answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED,
	            &request_completed, NULL,
	            MHD_OPTION_THREAD_POOL_SIZE,100,
	            MHD_OPTION_END);
	}else{*/
		http_daemon = MHD_start_daemon(threaded?MHD_USE_THREAD_PER_CONNECTION:MHD_USE_SELECT_INTERNALLY,
				_port, NULL, NULL,
				&answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED,
	            &request_completed, NULL, MHD_OPTION_END);
	//}
}

void ofxHTTPServer::stop(){
	MHD_stop_daemon(http_daemon);
}

void ofxHTTPServer::setServerRoot(const string & fsroot){
	fsRoot = ofToDataPath(fsroot,true);
	ofDirectory(fsRoot).create();
	ofLogNotice("ofxHttpServer", "serving files at " + fsRoot );
}


void ofxHTTPServer::setUploadDir(const string & uploaddir){
	uploadDir = ofToDataPath(uploaddir,true);
	ofDirectory(uploadDir).create();
	ofLogNotice("ofxHttpServer", "uploading posted files to " + uploadDir);
}

void ofxHTTPServer::setCallbackExtensions(const string & cb_extensions){
	callbackExtensionSet = true;
	vector<string> extensions = ofSplitString(cb_extensions," ");
	callbackExtensions.insert(extensions.begin(),extensions.end());
}


void ofxHTTPServer::setMaxNumberClients(unsigned num_clients){
	maxClients = num_clients;
}

void ofxHTTPServer::setMaxNumberActiveClients(unsigned num_clients){
	maxActiveClients = num_clients;
}

unsigned ofxHTTPServer::getNumberClients(){
	return numClients;
}

void ofxHTTPServer::setListener(ofxHTTPServerListener & _listener){
	listener = &_listener;
}
