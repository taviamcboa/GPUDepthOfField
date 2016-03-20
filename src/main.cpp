#include <math.h>
#include <GL/glew.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>

#include "Common.h"
#include "glVisionApp.h"
#include "glVisionAntTweakBar.h"
#include "glVisionUtility.h"
#include "TransformationWrapper.h"
#include "cameraModel.h"
#include "glVisionDriverBase.h"
#include "Mesh.h"
#include "LightModel.h"
#include "BaseLightingGLSLTechnique.h"


#define WINDOW_WIDTH  1024  
#define WINDOW_HEIGHT 768

class GPUDepthOfField : public AppInterface, public glVisionAntTweakBar, public glVisionApp
{
public:

	GPUDepthOfField(): glVisionAntTweakBar(), glVisionApp()
	{
		m_pGameCamera = NULL;
		m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
		m_directionalLight.AmbientIntensity = 0.66f;
		m_directionalLight.DiffuseIntensity = 1.0f;
		m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);

		m_persProjInfo.FOV = 60.0f;
		m_persProjInfo.Height = WINDOW_HEIGHT;
		m_persProjInfo.Width = WINDOW_WIDTH;
		m_persProjInfo.zNear = 1.0f;
		m_persProjInfo.zFar = 1000.0f;

		m_pipeline.SetPerspectiveProj(m_persProjInfo);
		m_pipeline.WorldPos(Vector3f(0.0f, 0.0f, 0.0f));
		m_pipeline.Scale(0.1f, 0.1f, 0.1f);
	}

	~GPUDepthOfField()
	{
		SAFE_DELETE(m_pGameCamera);
	}

	bool Init()
	{
		glVisionAntTweakBar::Init();

		Vector3f Pos(0.0f, 23.0f, -5.0f);
		Vector3f Target(-1.0f, 0.0f, 0.1f);
		Vector3f Up(0.0, 1.0f, 0.0f);

		m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
		if (!m_LightingTech.Init()) {
			GLVISION_ERROR("Error initializing the lighting technique\n");
			return false;
		}
		m_LightingTech.Enable();

		m_LightingTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
		m_LightingTech.SetDirectionalLight(m_directionalLight);
		m_LightingTech.SetMatSpecularIntensity(0.0f);
		m_LightingTech.SetMatSpecularPower(0);

		if (!m_mesh.LoadMesh("../Content/crytek_sponza/sponza.obj")) {
			return false;
		}

		TwBar *bar;
		bar = TwNewBar("NameOfMyTweakBar");

		TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.' "); // Message added to the help bar.

		double speed = 0.3; // Model rotation speed
							// Add 'speed' to 'bar': it is a modifable (RW) variable of type TW_TYPE_DOUBLE. Its key shortcuts are [s] and [S].
		TwAddVarRW(bar, "speed", TW_TYPE_DOUBLE, &speed,
			" label='Rot speed' min=0 max=2 step=0.01 keyIncr=s keyDecr=S help='Rotation speed (turns/second)' ");

		float g_Rotation[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		// Add 'g_Rotation' to 'bar': this is a variable of type TW_TYPE_QUAT4F which defines the object's orientation
		TwAddVarRW(bar, "ObjRotation", TW_TYPE_QUAT4F, &g_Rotation,
			" label='Object rotation' opened=true help='Change the object orientation.' ");

		return true;
	}


	void Run()
	{
		run(this);
	}


	virtual void RenderSceneCB()
	{
		glVisionAntTweakBar::RenderSceneCB();
		m_pGameCamera->OnRender();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_LightingTech.SetEyeWorldPos(m_pGameCamera->GetPos());

		m_pipeline.SetCamera(*m_pGameCamera);

		m_LightingTech.SetWVP(m_pipeline.GetWVPTrans());
		m_LightingTech.SetWorldMatrix(m_pipeline.GetWorldTrans());

		m_mesh.Render();

		CalcFPS();

		swapBuffers();
	}

	virtual void KeyboardCB(GLVISION_KEY key, GLVISION_KEY_STATE keystate)
	{
		glVisionAntTweakBar::KeyboardCB(key);
		
		switch (key) {
		case GLVISION_KEY_ESCAPE:
		case GLVISION_KEY_q:
			leaveMainLoop();
			break;
		default:
			m_pGameCamera->OnKeyboard(key);
		}
	}


	virtual void PassiveMouseCB(int x, int y)
	{
		glVisionAntTweakBar::PassiveMouseCB(x, y);
		m_pGameCamera->OnMouse(x, y);
	}

	virtual void ResizeCB(int width, int height)
	{
		glVisionAntTweakBar::ResizeCB(width, height); 
		m_pGameCamera->setWindowSize(width, height); 
		
		m_persProjInfo.Height = height; 
		m_persProjInfo.Width = width; 

		glViewport(0, 0, width, height);
	}


private:
	BasicLightingTechnique m_LightingTech;
	DirectionalLight m_directionalLight;
	Camera* m_pGameCamera;
	BasicMesh m_mesh;
	PersProjInfo m_persProjInfo;

	Pipeline m_pipeline;
};


int main(int argc, char** argv)
{
	initGLContext(GLVISION_DRIVER_TYPE_GLUT, argc, argv, true, false);

	if (!createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "GPU Depth of Field by Anisotropic Mip-map Interpolation")) {
		terminateGLContext();
		return 1;
	}

	SRANDOM;

	GPUDepthOfField* pApp = new GPUDepthOfField();

	if (!pApp->Init()) {
		delete pApp;
		terminateGLContext();
		return 1;
	}

	pApp->Run();

	delete pApp;

	terminateGLContext();

	return 0;
}
