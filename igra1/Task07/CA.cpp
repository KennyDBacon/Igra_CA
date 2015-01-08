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
#include "Node.h"
#include "Vertex.h"	// the IGRA vertex structures

#include "GeometryGenerator.h"	// the IGRA vertex structures
# define M_PI           3.14159265358979323846  /* pi */
#define D3DXToRadian(degree) ((degree) * (M_PI / 180.0f))
#include "ModelObj.h"	// the IGRA model loader

using namespace DirectX::Colors;
using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace igra;

class Bullet : public DrawableNode
{
public:
	Vector3 mVelocity;
	float mLifetime;
	virtual void Update();
};

class MyApp:public App
{
	void Startup();
	void Update();
	void Draw();
	void Shutdown();	
	void UpdateCamera();

	void PlayScene();

	void PlayerMovement();

	void SetupPointLights(ShaderManager::cbLights &lights);

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;
	std::unique_ptr<PrimitiveBatch<ColouredVertex>> mpDraw3D;

	std::unique_ptr<SpriteBatch> mpSpriteBatch;
	std::unique_ptr<SpriteFont> mpSpriteFont;
	
	ModelObj mPlane,mBox;

	std::vector<DrawableNode*> mCrates;
	DrawableNode mGround;
	DrawableNode mTestCrate;
	bool mDiagnostics;
	void FireShot();
	void CheckCollisions();

	bool mFreeCamera, zoom, angle;
	CameraNode mCamera;

	float speed;

	// Scene variable
	bool play, pause;

	// Scene indicator
	bool sRotation; // Crates rotate in incremental radius
	bool sClump; // Crates clump together
	bool sExplode; // Crates pushed away from center

	float rotateRadius, rotateSpeed, rotateAngle;

	// DEBUG
	std::wstring radiusStr, timerStr;
	float timerC;
};

void Bullet::Update()
{
	mLifetime-= Timer::GetDeltaTime();
	mPos+=mVelocity*Timer::GetDeltaTime();
	if(mLifetime<0)
	{
		Kill();//Kills self
	}
}
void MyApp::FireShot()
{
	/*
	Bullet* ptr = FindDeadNode(mBullets);
	if(ptr== nullptr)return;

	ptr->mHealth=100;
	ptr->mLifetime= 3.0f;
	ptr->SetPos(mPlayer.GetPos()+mPlayer.RotateVector(Vector3(0,0.45f,1.2f)));
	ptr->mVelocity = mPlayer.RotateVector(Vector3(0,0,10));
	ptr->mScale=0.2f;*/
}
void MyApp::CheckCollisions()
{
	/*
	for(int b = 0;b<mBullets.size();b++)
	{
		if(mBullets[b]->IsAlive()==false) continue;
		
		BoundingSphere bs = mBullets[b]->GetBounds();

		for(int t = 0;t <mBoxes.size();t++)
		{
			if(mBoxes[t]->IsAlive()==false)continue;

			if(bs.Intersects(mBoxes[t]->GetBounds()))
			{
				mBullets[b]->Kill();
				mBoxes[t]->mHealth-=25;
			}
		}
	}*/
}

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"CA Kenny";

	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));
	
	// initial pos/tgt for camera
	//mCamera.Reset();
	
	mpSpriteBatch.reset(new SpriteBatch(GetContext()));
	mpSpriteFont.reset(new SpriteFont(GetDevice(), L"../Content/Times16.sprfont"));

	mpDraw3D.reset(new PrimitiveBatch<ColouredVertex>(GetContext()));

	// load models:
	mPlane.Load(GetDevice(),mpShaderManager.get(),L"../Content/plane.obj",true);
	mBox.Load(GetDevice(),mpShaderManager.get(),L"../Content/crate.obj");
	
	mDiagnostics = false;

	mGround.Init(&mPlane,Vector3(0,0,0));
	
	mTestCrate.Init(&mBox,Vector3(0,0.5,0));

	float boxPosX = 0.5;
	float boxPosZ = 0.5;
	float boxRot = -270;
	float boxInc = 1;
	int boxNum = 4;
	for(int i = 0;i < boxNum;i++)
	{
		/*
		DrawableNode* ptr = new DrawableNode();
		ptr->Init(&mBox);
		ptr->mPos.x=boxPosX;
		ptr->mPos.y=0.5f;
		ptr->mPos.z=boxPosZ;
		mCrates.push_back(ptr);

		boxPosX--;

		if(i + 1 == boxNum / 2)
		{
			boxPosX = 0.5;
			boxPosZ = -0.5;
		}*/

		DrawableNode* ptr = new DrawableNode();
		ptr->Init(&mBox);
		ptr->mPos.x=boxPosX;
		ptr->mPos.y= 0.5f;
		ptr->mPos.z=boxPosZ;
		ptr->mHpr.x=i*XMConvertToRadians(-90);
		mCrates.push_back(ptr);

		boxPosX = -0.5 * boxInc;

		if(i == (boxNum-1) / 2)
		{
			boxPosX = -0.5;
			boxPosZ = -0.5;
			boxInc = -1;
		}
	}

	speed = 5.0f;

	play = true;
	pause = true;

	sRotation = true;
	sClump = false;
	sExplode = false;

	rotateRadius = 2;
	rotateSpeed = 0;
	rotateAngle = 0;

	radiusStr = ToString("Radius: ", rotateRadius);
	timerC = 0;
	timerStr = ToString("Timer: ", timerC);

	zoom = 5;
	angle = 0;
	mFreeCamera = true;
	mCamera.Init(Vector3(0,2,-10));
	mCamera.LookAt(Vector3(0,0,0));
	mCamera.SetNearFar(0.05f,100.0f);
	mCamera.mPos= Vector3(0,5,-10);
	//mCamera.mHpr=Vector3(0,XMConvertToRadians(90),0);
}

void MyApp::SetupPointLights(ShaderManager::cbLights &lights)
{
	lights.gPointLights[0].Ambient  = Color(0.2f, 0.2f, 0.2f, 1.0f);
	lights.gPointLights[0].Diffuse  = Color(0.5f, 0.5f, 0.5f, 1.0f);
	lights.gPointLights[0].Specular = Color(0.3f, 0.3f, 0.3f, 1.0f);

	lights.gPointLights[1].Ambient  = Color(0.0f, 0.0f, 0.0f, 1.0f);
	lights.gPointLights[1].Diffuse  = Color(0.20f, 0.20f, 0.20f, 1.0f);
	lights.gPointLights[1].Specular = Color(0.15f, 0.15f, 0.15f, 1.0f);

	lights.gPointLights[2].Ambient  = Color(0.0f, 0.0f, 0.0f, 1.0f);
	lights.gPointLights[2].Diffuse  = Color(0.2f, 0.2f, 0.2f, 1.0f);
	lights.gPointLights[2].Specular = Color(0.0f, 0.0f, 0.0f, 1.0f);
}

void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),
                           D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(),
                           DirectX::Colors::SkyBlue);
	GetContext()->OMSetDepthStencilState(mpShaderManager->CommonStates()->DepthDefault(),0);

	// Set Primitive Topology
	GetContext()->IASetPrimitiveTopology(
                             D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ShaderManager::SetSampler(GetContext(),
		mpShaderManager->CommonStates()->AnisotropicWrap());
	// setup the matrixes
	Matrix world;
	Matrix view=XMMatrixLookAtLH(Vector3(0,3,-10), Vector3(0,0,0),
                                    Vector3(0,1,0));
	Matrix proj=XMMatrixPerspectiveFovLH(XM_PI/4,GetAspectRatio(),1,1000);

	if(mFreeCamera)
	{
		view = mCamera.GetViewMatrix();
		proj = mCamera.GetProjectionMatrix();
	}

	mGround.Draw(GetContext(),view,proj);
	//mTestCrate.Draw(GetContext(),view,proj);
	DrawAliveNodes(mCrates,GetContext(),view,proj);

	mpSpriteBatch->Begin();
	radiusStr = ToString("Crate 3 X: ", mCrates[3]->mPos.x);
	timerStr = ToString("Timer: ", timerC);
	mpSpriteFont->DrawString(mpSpriteBatch.get(), radiusStr.c_str(), Vector2(20,20), Colors::Black);
	mpSpriteFont->DrawString(mpSpriteBatch.get(), timerStr.c_str(), Vector2(20,40), Colors::Black);
	mpSpriteBatch->End();

	if(mDiagnostics)
	{
		Draw3DPrepare(GetContext(),mpShaderManager.get(),view,proj);
	}
	// Present the backbuffer to the screen
	GetSwapChain()->Present(0, 0);
}

void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();
	
	if(Input::KeyPress(VK_SPACE))
	{
		//FireShot();
		pause = !pause;
	}

	if(Input::KeyPress(VK_BACK))
	{
		play = !play;
	}

	if(Input::KeyPress(VK_TAB))
	{
		mDiagnostics=!mDiagnostics;
	}

	if(Input::KeyPress(VK_F1))
	{
		mFreeCamera = !mFreeCamera;
	}

	PlayScene();

	//CheckCollisions();
	PlayerMovement();
}

void MyApp::PlayerMovement()
{
	Vector3 move = GetKeyboardMovement(KBMOVE_WSADRF);
	Vector3 turn= GetKeyboardMovement(KBMOVE_CURSOR_HRP);
	
	if(mFreeCamera){
		const float SPEED = 5.0f;

		mCamera.Move(move*SPEED*Timer::GetDeltaTime());
		mCamera.Turn(turn*SPEED*Timer::GetDeltaTime());
	}
}

void MyApp::PlayScene()
{
	float playSpeed = 1;
	if(play)
		playSpeed = 1;
	else
		playSpeed = -1;

	if(!pause)
	{
		if(timerC + Timer::GetDeltaTime() * playSpeed >= 0 && timerC + Timer::GetDeltaTime() * playSpeed <= 20)
		{
			for(int i = 0; i < mCrates.size(); i++)
			{
				float rotateAngle=XM_2PI * i / mCrates.size();
				rotateAngle += rotateSpeed;
				rotateSpeed += playSpeed * Timer::GetDeltaTime();

				mCrates[i]->mPos.x = rotateRadius * sin(rotateAngle);
				mCrates[i]->mPos.z = rotateRadius * cos(rotateAngle);
			}
			
			timerC += Timer::GetDeltaTime() * playSpeed;
		}
	}
}

void MyApp::UpdateCamera()
{

}

void MyApp::Shutdown()
{
	//DeleteAllNodes(mBoxes);
	DeleteAllNodes(mCrates);
}

// in console C++ is was main()
// in Windows C++ its called WinMain()  (or sometimes wWinMain)
int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine,
	int nShowCmd)
{
	ENABLE_LEAK_DETECTION();
	MyApp app;
	return app.Go(hInstance);	// go!
}



