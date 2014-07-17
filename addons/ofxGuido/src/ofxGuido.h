#include "GuidoComponent.h"
#include "openFrameworksDevice.h"

/*
 * Simple wrapper for Guido engine library
 * http://guidolib.sourceforge.net/
 */

class ofxGuido
{
public:
	ofxGuido(GuidoLayoutSettings* layoutSettings);
	~ofxGuido() {}

	bool compile_string(const string& gstr);
	void getPageFormat(GuidoPageFormat& format);
	void draw_cache(int x, int y);
	void draw(int x, int y, int w, int h);

	void setWidth(int w) { if (guido) guido->setWidth(w); }
	void setHeight(int h) { if (guido) guido->setHeight(h); }
	int getWidth() { if (guido) return guido->getWidth(); else return 0; }
	int getHeight() { if (guido) return guido->getHeight(); else return 0; }
	openFrameworksDevice* getDevice() { return guido->getDevice(); }
	void setSize(int w, int h);
	void setScale(float x, float y) { guido->setScale(x, y); }
	GuidoComponent* guido;
};
