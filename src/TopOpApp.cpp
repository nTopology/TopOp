#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TopOpApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void TopOpApp::setup()
{
}

void TopOpApp::mouseDown( MouseEvent event )
{
}

void TopOpApp::update()
{
}

void TopOpApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( TopOpApp, RendererGl )
