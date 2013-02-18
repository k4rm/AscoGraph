/*
 *  ColorRect.h
 *  emptyExample
 *
 *  Created by lukasz karluk on 10/08/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"

class ColorRect
{
	
public:
	
    ColorRect ()
    {
        init( ofGetWidth(), ofGetHeight() );
    }
    
    ColorRect( float w, float h )
    {
        init( w, h );
    }
    
    void init ( float w, float h )
    {
        rectPoints[ 0 ]  = 0;
        rectPoints[ 1 ]  = 0;
        rectPoints[ 2 ]  = w;
        rectPoints[ 3 ]  = 0;
        rectPoints[ 4 ]  = w;
        rectPoints[ 5 ]  = h;
        rectPoints[ 6 ]  = 0;
        rectPoints[ 7 ]  = h;
        
        rectColors[ 0 ]  = 1;
        rectColors[ 1 ]  = 0;
        rectColors[ 2 ]  = 0;
        rectColors[ 3 ]  = 1;
        rectColors[ 4 ]  = 1;
        rectColors[ 5 ]  = 0;
        rectColors[ 6 ]  = 0;
        rectColors[ 7 ]  = 1;
        rectColors[ 8 ]  = 0;
        rectColors[ 9 ]  = 0;
        rectColors[ 10 ] = 1;
        rectColors[ 11 ] = 1;
        rectColors[ 12 ] = 0;
        rectColors[ 13 ] = 0;
        rectColors[ 14 ] = 1;
        rectColors[ 15 ] = 1;
    }
    
    void setCornerColor( const ofColor& c, int cornerIndex )
    {
        int i = ofClamp( cornerIndex, 0, 3 );
        
        rectColors[ i * 4 + 0 ] = c.r / 255.0;
        rectColors[ i * 4 + 1 ] = c.g / 255.0;
        rectColors[ i * 4 + 2 ] = c.b / 255.0;
        rectColors[ i * 4 + 3 ] = 1.0;
    }
    
    void draw ()
    {
        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_COLOR_ARRAY );
        
        glVertexPointer( 2, GL_FLOAT, 0, rectPoints );
        glColorPointer(  4, GL_FLOAT, 0, rectColors );
        
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
        
        glDisableClientState( GL_COLOR_ARRAY );
        glDisableClientState( GL_VERTEX_ARRAY );
    }
	
	GLfloat	rectPoints[ 8 ];
	GLfloat rectColors[ 16 ];

};