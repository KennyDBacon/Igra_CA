/******************************
* Task6a.cpp
* Written by The IGRA team
******************************/
#include "DxCommon.h"	// common DX stuff
#include <DirectXColors.h>	// DirectX::Colors

#include "IgraApp.h"	// the IGRA 'game' class
#include "IgraUtils.h"	// useful functions
#include "ShaderManager.h"	// holds all the shaders
#include "ToString.h"
#include "SimpleMathToString.h"
#include "SpriteBatch.h" 
#include "SpriteFont.h"
#include "Draw3D.h"

#include "Vertex.h"	// the IGRA vertex structures

#include "GeometryGenerator.h"	// the IGRA vertex structures
# define M_PI           3.14159265358979323846  /* pi */
#define D3DXToRadian(degree) ((degree) * (M_PI / 180.0f))
#include "ModelObj.h"	// the IGRA model loader

using namespace DirectX::Colors;
using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace igra;

class MyApp:public App
{
	void Startup();
	void Update();
	void Draw();
	//void Shutdown();	

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;
	std::unique_ptr<PrimitiveBatch<ColouredVertex>> mpDraw3D;
	
	ModelObj mTeapot,mPlane,mCrate;
	
	ArcBallCamera mCamera;

	int mBlendUsed;
	CComPtr<ID3D11BlendState> mpTestBlend;
};

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Task 6a: Model Loading";

	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));
	
	// initial pos/tgt for camera
	mCamera.Reset();

	// load models:
	mTeapot.Load(GetDevice(),mpShaderManager.get(),L"../Content/crate.obj",
                         true);
	mPlane.Load(GetDevice(),mpShaderManager.get(),L"../Content/plane.obj",true);
	mCrate.Load(GetDevice(),mpShaderManager.get(),L"../Content/crate.obj",true);

	mTeapot.mMaterial.mMaterial.gMaterial.Diffuse = Color(1,1,1,1);
	mTeapot.mMaterial.mpTexture.Attach(
		CreateTextureResourceWIC(GetDevice(),L"../content/smiley.png"));

	mBlendUsed=0;	// which blend state to use
	// create a blend state:
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc,sizeof(desc));
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;
	desc.RenderTarget[0].BlendEnable = true; // enable blend
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc.RenderTarget[0].BlendOp = desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA; // source * Alpha
	desc.RenderTarget[0].DestBlend = desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA; // background * (1 - Alpha)

	GetDevice()->CreateBlendState(&desc,&mpTestBlend);

	ShowCursor(false);
	
}
void MyApp::Draw()
{
	// Set common rendering flags
	const float BLEND_FACTOR[] = {0.0f, 0.0f, 0.0f, 0.0f};
	const unsigned BLEND_MASK=0xffffffff;

	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),
                           D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(),
                           DirectX::Colors::SkyBlue);

	// Set Primitive Topology
	GetContext()->IASetPrimitiveTopology(
                             D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ShaderManager::SetSampler(GetContext(),
		mpShaderManager->CommonStates()->AnisotropicWrap());
	// setup the matrixes
	Matrix world;
	Matrix view=mCamera.GetViewMatrix();
	Matrix proj=mCamera.GetDefaultProjectionMatrix();

	// draw the main object
	if (mBlendUsed==0)
		GetContext()->OMSetBlendState(
                     mpShaderManager->CommonStates()->Opaque(),
                     BLEND_FACTOR,BLEND_MASK);
	if (mBlendUsed==1)
		GetContext()->OMSetBlendState(mpTestBlend,
                                BLEND_FACTOR,BLEND_MASK);
	
	world = Matrix::CreateTranslation(0,0,0);
	mPlane.mMaterial.FillMatrixes(world,view,proj);
	mPlane.mMaterial.Apply(GetContext());
	mPlane.Draw(GetContext());

	// teapot
	world=Matrix::CreateTranslation(2,1,0);
	mTeapot.mMaterial.FillMatrixes(world,view,proj);
	mTeapot.mMaterial.Apply(GetContext());
	mTeapot.Draw(GetContext());

	// Reset the blend state
	GetContext()->OMSetBlendState(mpShaderManager->CommonStates()->Opaque(),
                                      BLEND_FACTOR,BLEND_MASK);
	
	Draw3DPrepare(GetContext(),mpShaderManager.get(),view,proj);

	// Present the backbuffer to the screen
	GetSwapChain()->Present(0, 0);
}

void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();
	mCamera.Update();

	if (Input::KeyPress(VK_F1)) mBlendUsed=0;
	if (Input::KeyPress(VK_F2)) mBlendUsed=1;
	// if we need more just add more
}


// in console C++ is was main()
// in Windows C++ its called WinMain()  (or sometimes wWinMain)
int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine,
	int nShowCmd)
{
	MyApp app;
	return app.Go(hInstance);	// go!
}



