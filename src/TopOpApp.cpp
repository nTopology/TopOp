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
	void keyDown(KeyEvent event)override;
	void update() override;
	void resetSim();

	void initMesh();
	void updateMesh();
	void draw() override;


private:
	std::unique_ptr<TopOpt> mTopOp;
	TriMeshRef			mTriMesh;		//! The 3D mesh.
	gl::BatchRef		mMeshBatch;
	gl::BatchRef		mWirePlane;
	gl::VboMeshRef	mVboMesh;

	TriMeshRef mBMesh;
private:
	CameraPersp					mCam;
	CameraUi					mCamUi;

	const int eX = 80;
	const int eY = 40;
	const double volFrac = .5;
	const double penal = 3.0;
	const double rmin = 3.0;
	const double maxChange = .01;

	double comp;
	double vol;
	double change;

	bool run;
};

void topOp::setup()
{

	run = false;
	// Set up the camera.
	mCam.lookAt(vec3(0, 0, 200.0f), vec3(0,0,0.f));
	//mCam.setOrtho(0, getWindowWidth(),0,getWindowHeight(),0,10.f);
	mCam.setPerspective(40.0f, getWindowAspectRatio(), 0.01f, 300.0f);
	mCamUi = CameraUi(&mCam, getWindow());


	mTopOp = std::make_unique<TopOpt>(eX, eY, volFrac, penal, rmin, .01);

	initMesh();
}

void topOp::mouseDown(MouseEvent event)
{
	if (event.isLeftDown()) {
		if (!run) {
			run = true;
		}
		else {
			run = false;
		}
	}
	else if (event.isRightDown()) {
		resetSim();
	}
}


void topOp::mouseMove(MouseEvent event)
{
}


void topOp::mouseDrag(MouseEvent event)
{
	//mCamUi.mouseDrag(event);
}

void topOp::keyDown(KeyEvent event)
{
	auto code = event.getCode();
	if (code == 32) {
		run = !run;
	}
}

void topOp::update()
{
	if (run) {
		mTopOp->step(comp, vol, change);
		updateMesh();
	}
}

void topOp::resetSim()
{
	mTopOp = std::make_unique<TopOpt>(eX, eY, volFrac, penal, rmin, .01);
	initMesh();
}

void topOp::initMesh()
{	
	std::vector<glm::vec3> verts;
	std::vector<glm::vec4> colors;
	auto& mat = mTopOp->getX();

	for (int i = 0; i < eX; ++i) {
		for (int j = 0; j < eY; ++j) {
			auto v0 = glm::vec3((float)i, (float)eY-j, 0.f);
			auto v1 = glm::vec3((float)i+1, (float)eY-j, 0.f);
			auto v2 = glm::vec3((float)i+1, (float)eY-j+1, 0.f);
			auto v3 = glm::vec3((float)i, (float)eY-j+1, 0.f);
		
			verts.push_back(v0);
			verts.push_back(v1);
			verts.push_back(v3);

			verts.push_back(v1);
			verts.push_back(v2);
			verts.push_back(v3);

			colors.push_back(glm::vec4{ 1.f,0.f,0.f,1.f });
			colors.push_back(glm::vec4{ 1.f,0.f,0.f,1.f });
			colors.push_back(glm::vec4{ 1.f,0.f,0.f,1.f });
			colors.push_back(glm::vec4{ 1.f,0.f,0.f,1.f });
			colors.push_back(glm::vec4{ 1.f,0.f,0.f,1.f });
			colors.push_back(glm::vec4{ 1.f,0.f,0.f,1.f });
		}
	}


	auto layout = gl::VboMesh::Layout{};
	//layout.interleave(false);
	layout.attrib(geom::POSITION, 3);
	layout.attrib(geom::COLOR, 4);

	auto vboTriangles = gl::VboMesh::create(verts.size(), GL_TRIANGLES, { layout });
	vboTriangles->bufferAttrib(geom::POSITION, sizeof(vec3) * verts.size(), verts.data());
	vboTriangles->bufferAttrib(geom::COLOR, sizeof(vec3) * colors.size(), colors.data());

	mMeshBatch = gl::Batch::create(vboTriangles, gl::getStockShader(gl::ShaderDef().color()));
}

void topOp::updateMesh()
{
	auto iter = mMeshBatch->getVboMesh()->mapAttrib4f(geom::COLOR, false);
	auto& mat = mTopOp->getX();

	for (int i = 0; i < mat.getColumns(); ++i) {
		for (int j = 0; j < mat.getRows(); ++j) {
			auto mVal = mat.get(j, i);
			cinder::Color c(cinder::ColorModel::CM_HSV, { mVal,mVal,mVal });
			auto val = glm::vec4{ c.r,c.g,c.b,1.0f };
			*iter = val;
			iter++;
			*iter = val;
			iter++;
			*iter = val;
			iter++;
			*iter = val;
			iter++;
			*iter = val;
			iter++;
			*iter = val;
			iter++;
		}
	}
	iter.unmap();
}

void topOp::draw()
{
	gl::clear(Color(0, 0, 0));

	// Set up the camera.
	gl::ScopedMatrices push;
	gl::setMatrices(mCam);

	// Enable depth buffer.
	gl::ScopedDepth depth(true);


	// Draw the mesh.
	{
		gl::ScopedGlslProg scopedGlslProg(gl::context()->getStockShader(gl::ShaderDef().color()));
		mMeshBatch->draw();
		gl::multModelMatrix(glm::scale(glm::vec3{ -1.f,1.f,1.f }));
		mMeshBatch->draw();
	}

}

CINDER_APP(topOp, RendererGl)
