#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include <map>

#include "dxut.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"

#include "d3dx11effect.h"

#include "Mesh.h"
#include "Terrain.h"
#include "GameEffect.h"
#include "ConfigParser.h"

#include "debug.h"

// Help macros
#define DEG2RAD( a ) ( (a) * XM_PI / 180.f )

using namespace std;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

// Camera
struct CAMERAPARAMS {
	float   fovy;
	float   aspect;
	float   nearPlane;
	float   farPlane;
}                                       g_cameraParams;
float                                   g_cameraMoveScaler = 1000.f;
float                                   g_cameraRotateScaler = 0.01f;
CFirstPersonCamera                      g_camera;               // A first person camera

																// User Interface
CDXUTDialogResourceManager              g_dialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                         g_settingsDlg;          // Device settings dialog
CDXUTTextHelper*                        g_txtHelper = NULL;
CDXUTDialog                             g_hud;                  // dialog for standard controls
CDXUTDialog                             g_sampleUI;             // dialog for sample specific controls

																//ID3D11InputLayout*                      g_terrainVertexLayout; // Describes the structure of the vertex buffer to the input assembler stage

bool                                    g_terrainSpinning = true;
XMMATRIX                                g_terrainWorld; // object- to world-space transformation


														// Scene information
XMVECTOR                                g_lightDir;
Terrain									g_terrain;

GameEffect								g_gameEffect; // CPU part of Shader

													  // Config information
ConfigParser							g_configParser;

// Cockpit mesh
map<string, Mesh*>						g_Meshes;
bool									g_enableCameraFly = false;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLESPIN          4
#define IDC_RELOAD_SHADERS		101

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *, UINT, const CD3D11EnumDeviceInfo *,
	DXGI_FORMAT, bool, void*);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext);

void InitApp();
void DeinitApp();
void RenderText();

void ReleaseShader();
HRESULT ReloadShader(ID3D11Device* pd3dDevice);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Set DXUT callbacks
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);

	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	//DXUTSetIsInGammaCorrectMode(false);

	InitApp();
	DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(true, true);
	DXUTCreateWindow(L"Game"); // You may change the title

	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 1920, 1080);

	DXUTMainLoop(); // Enter into the DXUT render loop

					// Shutdown the app
	DXUTShutdown();
	DeinitApp();

	return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	HRESULT hr;
	WCHAR path[MAX_PATH];

	// Parse the config file
	V(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"game.cfg"));
	char pathA[MAX_PATH];
	size_t size;
	wcstombs_s(&size, pathA, path, MAX_PATH);
	g_configParser.Load(pathA);

	// Save locally
	map <string, ConfigParser::Mesh> l_Meshes = g_configParser.GetMeshes();

	// Load the meshes	
	for (auto it = l_Meshes.begin(); it != l_Meshes.end(); it++) {
		ConfigParser::Mesh l_Mesh = it->second;
		g_Meshes[it->first] = new Mesh("resources\\" + l_Mesh.File, "resources\\" + l_Mesh.Diffuse,
			"resources\\" + l_Mesh.Specular, "resources\\" + l_Mesh.Glow);
	}

	// Intialize the user interface

	g_settingsDlg.Init(&g_dialogResourceManager);
	g_hud.Init(&g_dialogResourceManager);
	g_sampleUI.Init(&g_dialogResourceManager);

	g_hud.SetCallback(OnGUIEvent);
	int iY = 30;
	int iYo = 26;
	g_hud.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22);
	g_hud.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3);
	g_hud.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2);

	g_hud.AddButton(IDC_RELOAD_SHADERS, L"Reload shaders (F5)", 0, iY += 24, 170, 22, VK_F5);

	g_sampleUI.SetCallback(OnGUIEvent); iY = 10;
	iY += 24;
	g_sampleUI.AddCheckBox(IDC_TOGGLESPIN, L"Toggle Spinning", 0, iY += 24, 125, 22, g_terrainSpinning);
}

//--------------------------------------------------------------------------------------
// Deletes everything before shutdown
//--------------------------------------------------------------------------------------
void DeinitApp() {
	// Delete all meshes
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) {
		SAFE_DELETE(it->second);
	}
	// Clear map
	g_Meshes.clear();
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
	g_txtHelper->Begin();
	g_txtHelper->SetInsertionPos(5, 5);
	g_txtHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
	g_txtHelper->DrawTextLine(DXUTGetFrameStats(true)); //DXUTIsVsyncEnabled() ) );
	g_txtHelper->DrawTextLine(DXUTGetDeviceStats());
	g_txtHelper->End();
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *, UINT, const CD3D11EnumDeviceInfo *,
	DXGI_FORMAT, bool, void*)
{
	return true;
}

//--------------------------------------------------------------------------------------
// Specify the initial device settings
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pDeviceSettings);
	UNREFERENCED_PARAMETER(pUserContext);

	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if (pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			DXUTDisplaySwitchingToREFWarning();
		}
	}
	//// Enable anti aliasing
	pDeviceSettings->d3d11.sd.SampleDesc.Count = 4;
	pDeviceSettings->d3d11.sd.SampleDesc.Quality = 1;

	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pBackBufferSurfaceDesc);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext(); // http://msdn.microsoft.com/en-us/library/ff476891%28v=vs.85%29
	V_RETURN(g_dialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	V_RETURN(g_settingsDlg.OnD3D11CreateDevice(pd3dDevice));
	g_txtHelper = new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &g_dialogResourceManager, 15);

	V_RETURN(ReloadShader(pd3dDevice));

	// Create the terrain
	V_RETURN(g_terrain.create(pd3dDevice, g_configParser));

	// Initialize the camera
	XMVECTOR vEye = XMVectorSet(0.0f, g_terrain.GetCameraHeight() * g_configParser.GetTerrainHeight() +
		g_configParser.GetTerrainHeight() * 0.5f, 0.0f, 0.0f);		// Camera eye is here
	XMVECTOR vAt = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);		// ... facing at this position
	g_camera.SetViewParams(vEye, vAt); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);

	// Adjust camera spinning
	g_terrainSpinning = fabs(g_configParser.GetSpinning()) > 0.0f;

	// Create input layout
	V_RETURN(Mesh::createInputLayout(pd3dDevice, g_gameEffect.meshPass1));

	// Create the meshes
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) {
		V_RETURN(it->second->create(pd3dDevice));
	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);

	g_dialogResourceManager.OnD3D11DestroyDevice();
	g_settingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	// Destroy the terrain
	g_terrain.destroy();

	// Destroy input layout
	Mesh::destroyInputLayout();

	// Destroy meshes
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) {
		it->second->destroy();
	}

	SAFE_DELETE(g_txtHelper);
	ReleaseShader();
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pSwapChain);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	// Intialize the user interface

	V_RETURN(g_dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(g_settingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	g_hud.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_hud.SetSize(170, 170);
	g_sampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
	g_sampleUI.SetSize(170, 300);

	// Initialize the camera

	g_cameraParams.aspect = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_cameraParams.fovy = 0.785398f;
	g_cameraParams.nearPlane = 1.f;
	g_cameraParams.farPlane = 5000.f;

	g_camera.SetProjParams(g_cameraParams.fovy, g_cameraParams.aspect, g_cameraParams.nearPlane, g_cameraParams.farPlane);
	g_camera.SetEnablePositionMovement(g_enableCameraFly);
	g_camera.SetRotateButtons(true, false, false);
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
	g_camera.SetDrag(true);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);
	g_dialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Loads the effect from file
// and retrieves all dependent variables
//--------------------------------------------------------------------------------------
HRESULT ReloadShader(ID3D11Device* pd3dDevice)
{
	assert(pd3dDevice != NULL);

	HRESULT hr;

	ReleaseShader();
	V_RETURN(g_gameEffect.create(pd3dDevice));

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	g_gameEffect.destroy();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);

	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_dialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Pass messages to settings dialog if its active
	if (g_settingsDlg.IsActive())
	{
		g_settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_hud.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;
	*pbNoFurtherProcessing = g_sampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Use the mouse weel to control the movement speed
	if (uMsg == WM_MOUSEWHEEL) {
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_cameraMoveScaler *= (1 + zDelta / 500.0f);
		if (g_cameraMoveScaler < 0.1f)
			g_cameraMoveScaler = 0.1f;
		g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
	}

	// Pass all remaining windows messages to camera so it can respond to user input
	g_camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	UNREFERENCED_PARAMETER(nChar);
	UNREFERENCED_PARAMETER(bKeyDown);
	UNREFERENCED_PARAMETER(bAltDown);
	UNREFERENCED_PARAMETER(pUserContext);
	// Reenable camera movement for debug purposes
	if (nChar == 'C' && bKeyDown) {
		g_enableCameraFly = !g_enableCameraFly;
		g_camera.SetEnablePositionMovement(g_enableCameraFly);
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	UNREFERENCED_PARAMETER(nEvent);
	UNREFERENCED_PARAMETER(pControl);
	UNREFERENCED_PARAMETER(pUserContext);

	switch (nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen(); break;
	case IDC_TOGGLEREF:
		DXUTToggleREF(); break;
	case IDC_CHANGEDEVICE:
		g_settingsDlg.SetActive(!g_settingsDlg.IsActive()); break;
	case IDC_TOGGLESPIN:
		g_terrainSpinning = g_sampleUI.GetCheckBox(IDC_TOGGLESPIN)->GetChecked();
		break;
	case IDC_RELOAD_SHADERS:
		ReloadShader(DXUTGetD3D11Device());
		break;
	}
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);
	// Update the camera's position based on user input 
	g_camera.FrameMove(fElapsedTime);

	// Initialize the terrain world matrix
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb206365%28v=vs.85%29.aspx

	// Start with identity matrix
	g_terrainWorld = XMMatrixIdentity();

	// Create scale matrix
	XMMATRIX terrainScale = XMMatrixScaling(
		g_configParser.GetTerrainWidth(),
		g_configParser.GetTerrainHeight(),
		g_configParser.GetTerrainDepth()
	);

	// Apply scaling
	g_terrainWorld = XMMatrixMultiply(g_terrainWorld, terrainScale);

	if (g_terrainSpinning)
	{
		// If spinng enabled, rotate the world matrix around the y-axis
		g_terrainWorld *= XMMatrixRotationY(30.0f * DEG2RAD((float)fTime)); // Rotate around world-space "up" axis
	}

	// Set the light vector
	g_lightDir = XMVectorSet(1, 1, 1, 0); // Direction to the directional light in world space    
	g_lightDir = XMVector3Normalize(g_lightDir);
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pd3dDevice);
	UNREFERENCED_PARAMETER(fTime);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if (g_settingsDlg.IsActive())
	{
		g_settingsDlg.OnRender(fElapsedTime);
		return;
	}

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	pd3dImmediateContext->ClearRenderTargetView(pRTV, clearColor);

	if (g_gameEffect.effect == NULL) {
		g_txtHelper->Begin();
		g_txtHelper->SetInsertionPos(5, 5);
		g_txtHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
		g_txtHelper->DrawTextLine(L"SHADER ERROR");
		g_txtHelper->End();
		return;
	}

	// Clear the depth stencil
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	// Update variables that change once per frame
	XMMATRIX const view = g_camera.GetViewMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	XMMATRIX const proj = g_camera.GetProjMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb147302%28v=vs.85%29.aspx
	XMMATRIX worldViewProj = g_terrainWorld * view * proj;
	// Transpose and inverse the world matrix
	XMMATRIX worldNormal = XMMatrixTranspose(XMMatrixInverse(nullptr, g_terrainWorld));

	// Save in shader
	V(g_gameEffect.worldEV->SetMatrix((float*)&g_terrainWorld));
	V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&worldViewProj));
	V(g_gameEffect.worldNormalsMatrix->SetMatrix((float*)&worldNormal));
	V(g_gameEffect.lightDirEV->SetFloatVector((float*)&g_lightDir));

	// Input layout isn't needed anymore
	pd3dImmediateContext->IASetInputLayout(nullptr);

	// Perform rendering
	g_terrain.render(pd3dImmediateContext, g_gameEffect.pass0);

	vector<ConfigParser::RenderObject> renderObjs = g_configParser.GetRenderObjs();

	// Loop over all the meshes to be rendered for the cockpit
	for (auto it = renderObjs.begin(); it != renderObjs.end(); ++it) {

		// Create matrices for cockpit mesh transformation
		XMMATRIX mTrans, mScale, mRot;
		mRot = XMMatrixRotationRollPitchYaw(DEG2RAD(it->RotationX), DEG2RAD(it->RotationY),DEG2RAD(it->RotationZ));
		mTrans = XMMatrixTranslation(it->TranslationX, it->TranslationY, it->TranslationZ);
		mScale = XMMatrixScaling(it->Scale, it->Scale, it->Scale);

		// Object to world space for cockpit in correct order (for lighting)
		XMMATRIX mWorld =  mRot * mTrans * mScale * (it->Classification == "Cockpit" ? g_camera.GetWorldMatrix() : XMMatrixIdentity());
		// Object to clip space (for rendering)
		XMMATRIX mWorldViewProj = mWorld * g_camera.GetViewMatrix() * g_camera.GetProjMatrix();
		// Normals transformation matrix (inverse transposed of world)
		XMMATRIX mWorldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));
		// Store camera position
		XMVECTOR mcameraPosWorld = g_camera.GetEyePt();

		// Save in shader
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldNormalsMatrix->SetMatrix((float*)&mWorldNormals))
			V(g_gameEffect.cameraPosWorldEV->SetFloatVector((float*)&mcameraPosWorld));

		// Render the cockpit accordingly
		g_Meshes[it->Identifier]->render(pd3dImmediateContext, g_gameEffect.meshPass1, g_gameEffect.diffuseEV,
			g_gameEffect.specularEV, g_gameEffect.glowEV);
	}


	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	V(g_hud.OnRender(fElapsedTime));
	V(g_sampleUI.OnRender(fElapsedTime));
	RenderText();
	DXUT_EndPerfEvent();

	static DWORD dwTimefirst = GetTickCount();
	if (GetTickCount() - dwTimefirst > 5000)
	{
		OutputDebugString(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
		OutputDebugString(L"\n");
		dwTimefirst = GetTickCount();
	}
}