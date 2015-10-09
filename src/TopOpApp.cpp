#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/Batch.h"
#include "cinder/ObjLoader.h"

#include "TopOpt.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class geodesicApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void mouseMove(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;

	void update() override;
	void draw() override;

private:
	std::unique_ptr<TopOpt> mTopOp;
	TriMeshRef			mTriMesh;		//! The 3D mesh.
	gl::BatchRef		mMesh;
	gl::BatchRef		mWirePlane;
	gl::VboMeshRef	mVboMesh;

	TriMeshRef mBMesh;
private:
	CameraPersp					mCam;
	CameraUi					mCamUi;
};

void geodesicApp::setup()
{
	ObjLoader loadMesh(loadFile("bunny.obj"));
	mTriMesh = TriMesh::create(loadMesh);
	mTriMesh->recalculateNormals(true);

	//mTriMesh = TriMesh::create(geom::Teapot().subdivisions(6));
	auto lambertShader = gl::getStockShader(gl::ShaderDef().color().lambert());
	auto colorShader = gl::getStockShader(gl::ShaderDef().color());

	mMesh = gl::Batch::create(*mTriMesh, lambertShader);
	mWirePlane = gl::Batch::create(geom::WirePlane().size(vec2(10)).subdivisions(ivec2(10)), colorShader);

	// Set up the camera.
	mCam.lookAt(vec3(2.0f, 3.0f, 1.0f), vec3(0));
	mCam.setPerspective(40.0f, getWindowAspectRatio(), 0.01f, 100.0f);
	mCamUi = CameraUi(&mCam, getWindow());



}

void geodesicApp::mouseDown(MouseEvent event)
{
	mCamUi.mouseDown(event);
}


void geodesicApp::mouseMove(MouseEvent event)
{
}


void geodesicApp::mouseDrag(MouseEvent event)
{
	mCamUi.mouseDrag(event);
}

void geodesicApp::update()
{


}

void geodesicApp::draw()
{
	gl::clear(Color(0, 0, 0));

	// Set up the camera.
	gl::ScopedMatrices push;
	gl::setMatrices(mCam);

	// Enable depth buffer.
	gl::ScopedDepth depth(true);

	// Draw the grid on the floor.
	{
		gl::ScopedColor color(Color::gray(0.2f));
		mWirePlane->draw();
	}

	// Draw the mesh.
	{
		gl::ScopedColor color(Color::white());

		gl::ScopedModelMatrix model;
		//	gl::multModelMatrix(mTransform);

		mMesh->draw();
	}

}

CINDER_APP(geodesicApp, RendererGl)
