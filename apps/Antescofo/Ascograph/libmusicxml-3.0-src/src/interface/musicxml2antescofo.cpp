/*

  MusicXML Library
  Copyright (C) 2003-2009  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/

#ifdef VC6
# pragma warning (disable : 4786)
#endif

#include <iostream>
#include "libmusicxml.h"
#include "xml.h"
#include "xmlfile.h"
#include "xmlreader.h"
#include "xml2antescofovisitor.h"
#include "antescofowriter.h"

using namespace std;

namespace MusicXML2 
{


	

//_______________________________________________________________________________
static xmlErr xml2antescofo(SXMLFile& xmlfile, bool generateBars, ostream& out, const char* file) 
{
	Sxmlelement st = xmlfile->elements();
	if (st) {
		antescofowriter w;

		xml2antescofovisitor v(w, true, true, generateBars);
		Santescofoelement as = v.convert(st);
		if (file) {
			out << "; Antescofo partition converted from '" << file << "'" << endl
				<< ";  using libmusicxml v." << musicxmllibVersionStr() << endl;
		}
		else out << ";  Antescofo code converted using libmusicxml v." << musicxmllibVersionStr() << endl;
		out << "; and the embedded xml2antescofo converter v." << musicxml2antescofoVersionStr() << endl;
		//out << as << endl;

		w.print();
		return kNoErr;
	}
	return kInvalidFile;
}

//_______________________________________________________________________________
EXP xmlErr musicxmlfile2antescofo(const char *file, bool generateBars, ostream& out) 
{
	xmlreader r;
	SXMLFile xmlfile;
	xmlfile = r.read(file);
	if (xmlfile) {
		return xml2antescofo(xmlfile, generateBars, out, file);
	}
	return kInvalidFile;
}

//_______________________________________________________________________________
EXP xmlErr musicxmlfd2antescofo(FILE * fd, bool generateBars, ostream& out) 
{
	xmlreader r;
	SXMLFile xmlfile;
	xmlfile = r.read(fd);
	if (xmlfile) {
		return xml2antescofo(xmlfile, generateBars, out, 0);
	}
	return kInvalidFile;
}

//_______________________________________________________________________________
EXP xmlErr musicxmlstring2antescofo(const char * buffer, bool generateBars, ostream& out) 
{
	xmlreader r;
	SXMLFile xmlfile;
	xmlfile = r.readbuff(buffer);
	if (xmlfile) {
		return xml2antescofo(xmlfile, generateBars, out, 0);
	}
	return kInvalidFile;
}

}
