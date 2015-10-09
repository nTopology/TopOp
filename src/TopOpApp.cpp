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

class topOp : public App {
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

void topOp::setup()
{
	ObjLoader loadMesh(loadFile("../bunny_lr.obj"));
	mTriMesh = TriMesh::create(loadMesh);
	mTriMesh->recalculateNormals(false);

	//mTriMesh = TriMesh::create(geom::Teapot().subdivisions(6));
	auto lambertShader = gl::getStockShader(gl::ShaderDef().color().lambert());
	auto colorShader = gl::getStockShader(gl::ShaderDef().color());

	mMesh = gl::Batch::create(*mTriMesh, lambertShader);
	mWirePlane = gl::Batch::create(geom::WirePlane().size(vec2(10)).subdivisions(ivec2(10)), colorShader);

	// Set up the camera.
	mCam.lookAt(vec3(150.0f, 3.0f, 1.0f), vec3(0));
	mCam.setPerspective(40.0f, getWindowAspectRatio(), 0.01f, 300.0f);
	mCamUi = CameraUi(&mCam, getWindow());


	mTopOp = std::make_unique<TopOpt>(160, 80, .33, 1.5, 1.5, .01);
}

void topOp::mouseDown(MouseEvent event)
{
	mCamUi.mouseDown(event);
}


void topOp::mouseMove(MouseEvent event)
{
}


void topOp::mouseDrag(MouseEvent event)
{
	mCamUi.mouseDrag(event);
}

void topOp::update()
{


}

void topOp::draw()
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

CINDER_APP(topOp, RendererGl)
