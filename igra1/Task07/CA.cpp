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

	bool mNodeCamera;
	
	CameraNode mCamera;

	float speed;

	// Scene variable
	bool play, pause;

	// Scene indicator
	bool sRotation; // Crates rotate in incremental radius
	bool sClump; // Crates clump together
	bool sExplode; // Crates pushed away from center

	float rotateTimer, rotateRadius, rotateHeight, rotateAngle;

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
	mpSpriteFont.reset(new SpriteFont(GetDevice(), L"../Content/Times12.sprfont"));

	mpDraw3D.reset(new PrimitiveBatch<ColouredVertex>(GetContext()));

	// load models:
	mPlane.Load(GetDevice(),mpShaderManager.get(),L"../Content/plane.obj",true);
	mBox.Load(GetDevice(),mpShaderManager.get(),L"../Content/crate.obj");
	
	mDiagnostics = false;

	mGround.Init(&mPlane,Vector3(0,0,0));
	
	mTestCrate.Init(&mBox,Vector3(0,0.5,0));

	//float boxL = cos(XMConvertToRadians(45));
	float boxPosX = 0.5;
	float boxPosZ = 0.5;
	//float boxAngle = 45;
	//float boxPosIn = 1;
	int boxNum = 4;
	for(int i = 0;i < boxNum;i++)
	{
		
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
		}

		/*
		DrawableNode* ptr = new DrawableNode();
		ptr->Init(&mBox);
		ptr->mPos.x=boxPosX;
		ptr->mPos.y=0.5f;
		ptr->mPos.z=boxPosZ;
		ptr->mHpr.x=XMConvertToRadians(boxAngle);
		mCrates.push_back(ptr);

		boxPosX = 0;
		boxPosZ = boxL * boxPosIn;
		boxAngle -= 90;

		if(i == (boxNum-1) / 2)
		{
			boxPosX = -boxL;
			boxPosZ = 0;
			boxPosIn = -1;
		}*/
	}

	/*
	for(int i = 0;i<10;i++)
	{
		Bullet* ptr = new Bullet();
		ptr->Init(&mYellowBallObj);
		ptr->Kill();
		mBullets.push_back(ptr);
	}*/

	speed = 5.0f;

	play = true;
	pause = true;

	sRotation = true;
	sClump = false;
	sExplode = false;

	rotateTimer = 0;
	rotateRadius = 0.5f;
	rotateHeight = 0.1f;
	rotateAngle = 45;

	radiusStr = ToString("Radius: ", rotateRadius);
	timerC = 0;
	timerStr = ToString("Timer: ", timerC);

	mNodeCamera = false;
	mCamera.Init(Vector3(0,2,-10));
	mCamera.LookAt(Vector3(0,0,0));
	mCamera.SetNearFar(0.05f,100.0f);
	mCamera.mPos= Vector3(0,2,-10);
	mCamera.mHpr=Vector3(0,XMConvertToRadians(90),0);
	//mAxis.mScale=3;
	//mAxis.mHpr.x=XMConvertToRadians(45);
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

	if(mNodeCamera)
	{
		view = mCamera.GetViewMatrix();
		proj = mCamera.GetProjectionMatrix();
	}
	/*
	// teapot
	world=Matrix::CreateTranslation(2,1,0);
	mTeapot.mMaterial.FillMatrixes(world,view,proj);
	mTeapot.mMaterial.Apply(GetContext());
	mTeapot.Draw(GetContext());

	world = Matrix::CreateTranslation(0,0,0);
	mPlane.mMaterial.FillMatrixes(world,view,proj);
	mPlane.mMaterial.Apply(GetContext());
	mPlane.Draw(GetContext());

	Draw3DPrepare(GetContext(),mpShaderManager.get(),view,proj);

	*/

	mGround.Draw(GetContext(),view,proj);
	//mTestCrate.Draw(GetContext(),view,proj);
	DrawAliveNodes(mCrates,GetContext(),view,proj);

	mpSpriteBatch->Begin();
	radiusStr = ToString("Radius: ", mCrates[0]->mPos.y);
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
		mNodeCamera = !mNodeCamera;
	}

	const float MOVE_SPEED = 5,TURN_SPEED=XMConvertToRadians(90),SPIN_SPEED = XMConvertToRadians(60);

	/*
	for(unsigned i = 0;i<mBoxes.size();i++)
	{
		mBoxes[i]->Yaw(SPIN_SPEED*Timer::GetDeltaTime());
	}*/

	PlayScene();

	CheckCollisions();
	
	Vector3 move = GetKeyboardMovement(KBMOVE_WSADRF);
	Vector3 turn=GetKeyboardMovement(KBMOVE_CURSOR_HRP);
	UpdateCamera();
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
		if(timerC + Timer::GetDeltaTime() * playSpeed >= 0 && timerC + Timer::GetDeltaTime() * playSpeed <= 10)
		{
			timerC += Timer::GetDeltaTime() * playSpeed;

			speed += playSpeed * Timer::GetDeltaTime();
			for(int i = 0; i < mCrates.size(); i++)
			{
				
				mCrates[i]->mPos=Vector3(0,0.7f,0);
				float angle=XM_2PI * i / mCrates.size();
				angle+=speed;
				mCrates[i]->mPos.x = 3 * sin(angle);
				mCrates[i]->mPos.z = 3 * cos(angle);

			}

			/*
			for(int i = 0; i < mCrates.size(); i++)
			{
				if(sRotation)
				{
					if(mCrates[i]->mHpr.x <= 0 && mCrates[i]->mHpr.x <= XMConvertToRadians((i * -90) +45))
					{
						mCrates[i]->Yaw(XMConvertToRadians(rotateAngle) * Timer::GetDeltaTime() * playSpeed);
					}
					else if(timerC + Timer::GetDeltaTime() * playSpeed <= 3)
					{
						mCrates[i]->Move(Vector3(0, rotateHeight , rotateRadius) * Timer::GetDeltaTime() * playSpeed);
						//mCrates[i]->Move(Vector3(rotateRadius, 0, 0) * Timer::GetDeltaTime() * playSpeed);
						//mCrates[i]->Yaw(XMConvertToRadians(rotateAngle) * Timer::GetDeltaTime());
					}
				}
				else if(sClump)
				{

				}
				else
				{

				}
			}

			*/
		}
	}

	//mTestCrate.Move(Vector3(1,1,0)*Timer::GetDeltaTime());
	//mTestCrate.Pitch(XMConvertToRadians(30)*Timer::GetDeltaTime());
}

void MyApp::UpdateCamera()
{
	//mCamera.LookAt(mPlayer);
	/*
	Vector3 move = GetKeyboardMovement(KBMOVE_WSADRF);
	Vector3 turn=GetKeyboardMovement(KBMOVE_CURSOR_HRP);
	//mPlayer.mPos+=move*(MOVE_SPEED*Timer::GetDeltaTime());
	mCamera.mHpr+=turn*(XMConvertToRadians(90)*Timer::GetDeltaTime());
	mCamera.Move(move*(5*Timer::GetDeltaTime()));

	POINT mouseMove = Input::GetMouseDelta();
	
	mCamera.mHpr.x += mouseMove.x *0.01;
	mCamera.mHpr.y += mouseMove.y *0.01;
	Input::SetMousePos(320,240,GetWindow()); 

	*/
	/*
	mCamera.SetPos(mPlayer.GetPos()+mPlayer.RotateVector(Vector3(0.4f,0.9f,-1.3f)));
	//mCamera.LookAt(mPlayer.GetPos()+mPlayer.RotateVector(Vector3(1.5,0,0)));
	mCamera.mHpr = mPlayer.mHpr;
	*/
	/*
	Vector3 tgt = mPlayer.GetPos()+mPlayer.RotateVector(Vector3(0,2,-5.0f));
	tgt = Vector3::Lerp(mCamera.GetPos(),tgt,5.0f*Timer::GetDeltaTime());

	mCamera.SetPos(tgt);
	mCamera.LookAt(mPlayer);
	*/

	if(mNodeCamera){
		const float SPEED = 3.0f;
		Vector3 move(0,0,0);

		float moveDir = 0;
		if(Input::KeyDown('A')) moveDir=1;
		if(Input::KeyDown('D')) moveDir=-1;

		if(Input::KeyDown('W')) move.z++;
		if(Input::KeyDown('S')) move.z--;

		mCamera.LookAt(Vector3(0,0,0));
		mCamera.mPos+=move*SPEED*Timer::GetDeltaTime();
	}
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



