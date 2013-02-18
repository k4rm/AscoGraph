//
// HTTPRequestHandler.h
//
// $Id: //poco/1.4/Net/include/Poco/Net/HTTPRequestHandler.h#1 $
//
// Library: Net
// Package: HTTPServer
// Module:  HTTPRequestHandler
//
// Definition of the HTTPRequestHandler class.
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef Net_HTTPRequestHandler_INCLUDED
#define Net_HTTPRequestHandler_INCLUDED


#include "Poco/Net/Net.h"


namespace Poco {
namespace Net {


class HTTPServerRequest;
class HTTPServerResponse;


class Net_API HTTPRequestHandler
	/// The abstract base class for HTTPRequestHandlers 
	/// created by HTTPServer.
	///
	/// Derived classes must override the handleRequest() method.
	/// Furthermore, a HTTPRequestHandlerFactory must be provided.
	///
	/// The handleRequest() method must perform the complete handling
	/// of the HTTP request connection. As soon as the handleRequest() 
	/// method returns, the request handler object is destroyed.
	///
	/// A new HTTPRequestHandler object will be created for
	/// each new HTTP request that is received by the HTTPServer.
{
public:
	HTTPRequestHandler();
		/// Creates the HTTPRequestHandler.

	virtual ~HTTPRequestHandler();
		/// Destroys the HTTPRequestHandler.

	virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) = 0;
		/// Must be overridden by subclasses.
		///
		/// Handles the given request.

private:
	HTTPRequestHandler(const HTTPRequestHandler&);
	HTTPRequestHandler& operator = (const HTTPRequestHandler&);
};


} } // namespace Poco::Net


#endif // Net_HTTPRequestHandler_INCLUDED
