/*

  MusicXML Library
  Copyright � Grame 2007

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

    Grame Research Laboratory, 9, rue du Garet 69001 Lyon - France
    research@grame.fr

*/

#ifndef __xml2antescofovisitor__
#define __xml2antescofovisitor__

#include <ostream>
#include <stack>
#include <map>
#include <string>

#include "exports.h"
#include "antescofotree.h"
#include "typedefs.h"
#include "visitor.h"
#include "xml.h"
//#include "xmlpart2antescofo.h"
#include "antescofowriter.h"

namespace MusicXML2 
{
/*!
\ingroup visitors antescofo
@{
*/

//______________________________________________________________________________
typedef struct {
	S_movement_title		fTitle;
	std::vector<S_creator>	fCreators;
} scoreHeader;

typedef struct {
	S_part_name		fPartName;
} partHeader;
typedef std::map<std::string, partHeader> partHeaderMap;


/*!
\brief A score visitor to produce a antescofo representation.
*/
//______________________________________________________________________________
class EXP xml2antescofovisitor : 
	public visitor<S_score_partwise>,
	public visitor<S_movement_title>,
	public visitor<S_creator>,
	public visitor<S_score_part>,
	public visitor<S_part_name>,
	public visitor<S_part>
{
	// the antescofo elements stack
	std::stack<Santescofoelement>	fStack;
	bool	fGenerateComments, fGenerateStem, fGenerateBars, fGeneratePositions;
	
	scoreHeader		fHeader;		// musicxml header elements (should be flushed at the beginning of the first voice)
	partHeaderMap	fPartHeaders;	// musicxml score-part elements (should be flushed at the beginning of each part)
	std::string		fCurrentPartID;
	int				fCurrentStaffIndex;		// the index of the current antescofo staff

	void start (Santescofoelement& elt)		{ fStack.push(elt); }
	void add (Santescofoelement& elt)		{ fStack.top()->add(elt); }
	void push (Santescofoelement& elt)		{ add(elt); fStack.push(elt); }
	void pop ()							{ fStack.pop(); }


	void flushHeader	 ( scoreHeader& header );
	void flushPartHeader ( partHeader& header );

	protected:

		virtual void visitStart( S_score_partwise& elt);
		virtual void visitStart( S_movement_title& elt);
		virtual void visitStart( S_creator& elt);
		virtual void visitStart( S_score_part& elt);
		virtual void visitStart( S_part_name& elt);
		virtual void visitStart( S_part& elt);
		Santescofoelement& current ()				{ return fStack.top(); }

    public:
				 xml2antescofovisitor(antescofowriter& w, bool generateComments, bool generateStem, bool generateBar=true);
		virtual ~xml2antescofovisitor() {}

		Santescofoelement convert (const Sxmlelement& xml);

		antescofowriter& w;

		// this is to control exact positionning of elements when information is present
		// ie converts relative-x/-y into dx/dy attributes
		void generatePositions (bool state)		{ fGeneratePositions = state; }

	static void addPosition	 ( Sxmlelement elt, Santescofoelement& tag, int yoffset);
};


} // namespace MusicXML2


#endif
