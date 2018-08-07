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
#include <list>
#include <time.h>

#include "dxut.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"

#include "d3dx11effect.h"

#include "Mesh.h"
#include "Shield.h"
#include "Terrain.h"
#include "3DRenderer.h"
#include "ConfigParser.h"
#include "SpriteRenderer.h"

#include "debug.h"

#define DEG2RAD( a ) ( (a) * XM_PI / 180.f )

#define PROJECTILELIFETIME 10.0f
#define PARTVEL 100.0f

using namespace std;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

// Gun info
struct GunInstance {
	bool IsFiring;
	float CooldownLeft;
	ConfigParser::GunType Type;
};

// Projectile info
struct SpriteInstance {
	SpriteVertex Sprite;
	XMFLOAT3 Velocity;
	XMFLOAT3 Gravity;
	float LifeTime;
	int Damage;
};

// Enemy instance info
struct EnemyInstance {
	string Identifier;
	XMFLOAT3 Position;
	XMFLOAT3 Velocity;
	string DestroyAnim;
	string DestroyPart;
	XMFLOAT3 Hits[10] = {};
	int currHit = 0;
	int Hitpoints;
	float Radius;
	float Scale;
};

// Camera
struct CAMERAPARAMS {
	float   fovy;
	float   aspect;
	float   nearPlane;
	float   farPlane;
}										g_cameraParams;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

float                                   g_cameraMoveScaler = 1000.f;
float                                   g_cameraRotateScaler = 0.01f;
CFirstPersonCamera                      g_camera;               // A first person camera

																// User Interface
CDXUTDialogResourceManager              g_dialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                         g_settingsDlg;          // Device settings dialog
CDXUTTextHelper*                        g_txtHelper = NULL;
CDXUTDialog                             g_hud;                  // dialog for standard controls
CDXUTDialog                             g_sampleUI;             // dialog for sample specific controls

bool                                    g_terrainSpinning = true;
XMMATRIX                                g_terrainWorld; // object- to world-space transformation


// Scene information
XMVECTOR                                g_lightDir;
Terrain									g_terrain;

GameEffect								g_3DRenderEffect; // CPU part of Shader

// Config information
ConfigParser							g_configParser;

// Meshes
map<string, Mesh*>						g_Meshes;
Shield*									g_EnemyShield;
bool									g_enableCameraFly = false;

// Enemies
list<EnemyInstance>						g_enemyInstances;
list<SpriteInstance>					g_Explosions;
float									g_spawnTimer = 0.0f;

// Sprite renderer
SpriteRenderer*							g_spriteRenderer;

// Guns
vector<GunInstance>						g_Guns;
list<SpriteInstance>					g_Projectiles;

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
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo*, UINT, const CD3D11EnumDeviceInfo*, DXGI_FORMAT, bool, void*);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext);

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

	// Seed generator
	srand(static_cast<int>(time(NULL)));

	// Save configs locally
	map <string, ConfigParser::Mesh> l_Meshes = g_configParser.GetMeshes();

	// Load the meshes	
	for (auto it = l_Meshes.begin(); it != l_Meshes.end(); it++) {
		ConfigParser::Mesh l_Mesh = it->second;
		g_Meshes[it->first] = new Mesh(	g_configParser.GetResourceFolder() + l_Mesh.File,
										g_configParser.GetResourceFolder() + l_Mesh.Diffuse,
										g_configParser.GetResourceFolder() + l_Mesh.Specular,
										g_configParser.GetResourceFolder() + l_Mesh.Glow,
										g_configParser.GetResourceFolder() + l_Mesh.Normal);
	}

	// Initialize the sprite renderer
	g_spriteRenderer = new SpriteRenderer(g_configParser);

	// Initialize the shield renderer
	g_EnemyShield = new Shield();

	// Initialize the guns
	map <string, ConfigParser::GunType> l_Guns = g_configParser.GetGunTypes();

	// Save them in the vector
	for (auto it = l_Guns.begin(); it != l_Guns.end(); it++) {
		GunInstance l_New;
		l_New.CooldownLeft = 0;
		l_New.IsFiring = false;
		l_New.Type = it->second;
		g_Guns.push_back(l_New);
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
void DeinitApp()
{
	// Delete all meshes
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) {
		SAFE_DELETE(it->second);
	}
	// Clear mesh map
	g_Meshes.clear();
	// Delete sprite renderer
	SAFE_DELETE(g_spriteRenderer);
	// Delete shield renderer
	SAFE_DELETE(g_EnemyShield);
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
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *, UINT,
	const CD3D11EnumDeviceInfo *, DXGI_FORMAT, bool, void*)
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
	XMVECTOR vEye = XMVectorSet(0.0f, g_terrain.GetHeightAtXY(0.5, 0.5) * g_configParser.GetTerrainInfo().Height +
		g_configParser.GetTerrainInfo().Height * 0.5f, 0.0f, 0.0f);		// Camera eye is here
	XMVECTOR vAt = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);		// ... facing at this position
	g_camera.SetViewParams(vEye, vAt); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);

	// Adjust camera spinning
	g_terrainSpinning = fabs(g_configParser.GetTerrainInfo().SpinSpeed) > 0.0f;

	// Create input layouts
	V_RETURN(Mesh::createInputLayout(pd3dDevice, g_3DRenderEffect.g_pMeshPass1));
	V_RETURN(Shield::createInputLayout(pd3dDevice, g_3DRenderEffect.g_pShieldPass2));

	// Create the meshes
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) {
		V_RETURN(it->second->create(pd3dDevice));
	}

	// Create the shield
	V_RETURN(g_EnemyShield->create(pd3dDevice, 10));

	// Create sprite renderer resources
	V_RETURN(g_spriteRenderer->create(pd3dDevice));

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

	// Destroy input layouts etc.
	Mesh::destroyInputLayout();
	Shield::destroyInputLayout();
	Shield::destroyDepthBuffer();

	// Destroy the terrain
	g_terrain.destroy();

	// Destroy meshes
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) {
		it->second->destroy();
	}

	// Destroy shield renderer
	g_EnemyShield->destroy();

	// Destroy sprite renderer
	g_spriteRenderer->destroy();

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

	V_RETURN(g_dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(g_settingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc))

	// Intialize the user interface
	g_hud.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_hud.SetSize(170, 170);
	g_sampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
	g_sampleUI.SetSize(170, 300);

	// Update depth buffer
	Shield::destroyDepthBuffer();	
	Shield::createDepthBuffer(pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

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
	V_RETURN(g_3DRenderEffect.create(pd3dDevice, g_configParser));

	// Reload sprite renderer
	V_RETURN(g_spriteRenderer->reloadShader(pd3dDevice));

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	// Release game effect stuff
	g_3DRenderEffect.destroy();
	// Release sprite renderer
	g_spriteRenderer->releaseShader();
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

	// Loop guns and update firing bool
	for (auto it = g_Guns.begin(); it != g_Guns.end(); it++) {
		if (nChar == it->Type.Hotkey) {
			it->IsFiring = bKeyDown;
		}
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
		g_configParser.GetTerrainInfo().Width,
		g_configParser.GetTerrainInfo().Height,
		g_configParser.GetTerrainInfo().Depth
	);

	// Apply scaling
	g_terrainWorld = XMMatrixMultiply(g_terrainWorld, terrainScale);

	if (g_terrainSpinning)
	{
		// If spinng enabled, rotate the world matrix around the y-axis
		g_terrainWorld *= XMMatrixRotationY(30.0f * DEG2RAD((float)fTime)); // Rotate around world-space "up" axis
	}

	//--------------------------------------------------------------------------------------
	// Enemy spawning
	//--------------------------------------------------------------------------------------

	// Update timer
	g_spawnTimer -= fElapsedTime;

	// If it drops below zero spawn a new enemy and reset
	if (g_spawnTimer < 0) {
		g_spawnTimer += g_configParser.GetSpawnInfo().Interval;
		// Random angles
		float alphaOuter = (rand() * XM_PI / RAND_MAX) * 2;
		float alphaInner = (rand() * XM_PI / RAND_MAX) * 2;
		// Random heights
		float heightOuter = g_configParser.GetSpawnInfo().MinHeight +
			((rand() * 1.0f / RAND_MAX) * (g_configParser.GetSpawnInfo().MaxHeight - g_configParser.GetSpawnInfo().MinHeight)) * 
			g_configParser.GetTerrainInfo().Height;
		float heightInner = g_configParser.GetSpawnInfo().MinHeight +
			((rand() * 1.0f / RAND_MAX) * (g_configParser.GetSpawnInfo().MaxHeight - g_configParser.GetSpawnInfo().MinHeight)) * 
			g_configParser.GetTerrainInfo().Height;
		// Calculate positions
		XMVECTOR s1 = XMVectorSet(
			g_configParser.GetSpawnInfo().OuterCircleRadius * sin(alphaOuter),
			heightOuter,
			g_configParser.GetSpawnInfo().OuterCircleRadius * cos(alphaOuter),
			1);
		XMVECTOR s2 = XMVectorSet(
			g_configParser.GetSpawnInfo().InnerCircleRadius * sin(alphaInner),
			heightInner,
			g_configParser.GetSpawnInfo().InnerCircleRadius * cos(alphaInner),
			1);
		// Create random enemy
		EnemyInstance l_NewEnemy;
		// Get beginning of the map
		map<string, ConfigParser::EnemyType> enemyTypes = g_configParser.GetEnemyTypes();
		auto enemyIt = enemyTypes.begin();
		// Move a random amount of positions
		unsigned int distance = (unsigned int)((rand() * 1.0f / RAND_MAX) * g_configParser.GetEnemyTypes().size());		
		advance(enemyIt, distance);
		// Get the random type
		ConfigParser::EnemyType randEnemy = enemyIt->second;
		// Adjust new instance
		l_NewEnemy.Hitpoints = randEnemy.Hitpoints;
		l_NewEnemy.Identifier = randEnemy.Identifier;
		l_NewEnemy.DestroyAnim = randEnemy.DestroyAnim;
		l_NewEnemy.DestroyPart = randEnemy.DestroyPart;
		XMStoreFloat3(&l_NewEnemy.Position, s1);
		XMStoreFloat3(&l_NewEnemy.Velocity, randEnemy.Speed * XMVector4Normalize(s2 - s1));
		l_NewEnemy.Radius = randEnemy.Size * (1.0F / randEnemy.Scale);
		l_NewEnemy.Scale = randEnemy.Scale;
		// Store in the list
		g_enemyInstances.push_back(l_NewEnemy);
	}

	//--------------------------------------------------------------------------------------
	// Enemy movement
	//--------------------------------------------------------------------------------------

	// Update enemy positions and remove if too far out
	for (auto it = g_enemyInstances.begin(); it != g_enemyInstances.end(); )
	{
		XMVECTOR currVel = XMLoadFloat3(&it->Velocity);
		XMStoreFloat3(&it->Position, XMVectorAdd(XMLoadFloat3(&it->Position), XMVectorScale(currVel, fElapsedTime)));
		XMVECTOR currPos = XMLoadFloat3(&it->Position);
		// Store seperately b/c it's modified further down
		float x = it->Position.x;
		float z = it->Position.z;
		// If outside of the radius
		if (sqrtf(powf(x, 2) + powf(z, 2)) > g_configParser.GetSpawnInfo().RemoveCircleRadius)
		{
			// Safe remove element
			it = g_enemyInstances.erase(it);
		}
		else
		{
			// Adjust position if in terrain and too low
			x += (g_configParser.GetTerrainInfo().Width / 2);
			x /= g_configParser.GetTerrainInfo().Width;
			z += (g_configParser.GetTerrainInfo().Depth / 2);
			z /= g_configParser.GetTerrainInfo().Depth;
			// Only if in terrain..
			if (x >= 0.0f && x <= 1.0f && z >= 0.0f && z <= 1.0f) {
				float y = g_terrain.GetHeightAtXY(x, z) * g_configParser.GetTerrainInfo().Height * 1.1f;
				// Adjust position accordingly
				if(y > it->Position.y)
					// Lerp to avoid jumping movement
					XMStoreFloat3(&it->Position, XMVectorLerp(currPos, XMVectorSetY(currPos, y), 0.1f));			
			}
			// Increase iterator
			it++;
		}
	}

	//--------------------------------------------------------------------------------------
	// Guns
	//--------------------------------------------------------------------------------------

	// Update cooldowns and fire guns
	for (auto it = g_Guns.begin(); it != g_Guns.end(); it++) {
		// Reduce cooldown left
		if (it->CooldownLeft > 0) {
			it->CooldownLeft -= fElapsedTime;
		}
		// If cooldown is zero and fire is pressed
		else if (it->IsFiring && it->CooldownLeft <= 0) {
			// Reset the timer
			it->CooldownLeft = it->Type.FireRate;
			// Spawn a projectile
			SpriteInstance l_NewProj;
			// Save in sprite vector
			SpriteVertex l_NewSprite;
			// Use it to calculate the spawn position
			XMVECTOR l_vPos = XMVector3Transform(XMVectorSet(it->Type.TranslationX, it->Type.TranslationY, 
				it->Type.TranslationZ, 1), XMMatrixInverse(nullptr, g_camera.GetViewMatrix()));
			// Set projectile information
			XMStoreFloat3(&l_NewProj.Velocity, XMVectorScale(XMVector4Normalize(g_camera.GetWorldAhead()), it->Type.ProjectileSpeed));
			XMStoreFloat3(&l_NewProj.Gravity, XMVectorSet(0, it->Type.ParticleMass, 0, 0));
			l_NewProj.LifeTime = PROJECTILELIFETIME;
			l_NewProj.Damage = it->Type.Damage;
			// Set particle information
			l_NewSprite.TextureIndex = g_spriteRenderer->getSpriteID(it->Type.SpriteName);
			XMStoreFloat3(&l_NewSprite.Position, l_vPos);
			l_NewSprite.Radius = it->Type.SpriteRadius;
			l_NewSprite.Progress = 0.0f;
			l_NewSprite.Alpha = 1.0f;
			// Save in projectile
			l_NewProj.Sprite = l_NewSprite;
			// Save in vector
			g_Projectiles.push_back(l_NewProj);
		}
	}

	//--------------------------------------------------------------------------------------
	// Sprites
	//--------------------------------------------------------------------------------------

	// Update projectiles
	for (auto it = g_Projectiles.begin(); it != g_Projectiles.end(); ) {
		// Remove the ones that have exceeded their lifetime
		if (it->LifeTime < 0) {
			// Safe remove element
			it = g_Projectiles.erase(it);
		}
		else {
			// Update lifetime
			it->LifeTime -= fElapsedTime;
			XMVECTOR currVel = XMLoadFloat3(&it->Velocity);
			// Subtract scaled gravity
			XMStoreFloat3(&it->Velocity, XMVectorSubtract(currVel, XMVectorScale(XMLoadFloat3(&it->Gravity), fElapsedTime)));
			// Update position afterwards based on velocity
			XMStoreFloat3(&it->Sprite.Position, XMVectorAdd(XMVectorSet(it->Sprite.Position.x,
																		it->Sprite.Position.y, 
																		it->Sprite.Position.z, 1),
																XMVectorScale(currVel, fElapsedTime)));
			// Next particle
			it++;
		}
	}

	// Loop explosions
	for (auto expl = g_Explosions.begin(); expl != g_Explosions.end(); ) {
		// Update animation progress
		expl->Sprite.Progress = ((expl->Sprite.Progress * expl->LifeTime) + fElapsedTime) / expl->LifeTime;
		// If animation finished remove it
		if (expl->Sprite.Progress >= 1.0f) {
			expl = g_Explosions.erase(expl);
		}
		else {
			// Store velocity
			XMVECTOR currVel = XMLoadFloat3(&expl->Velocity);
			// Subtract scaled gravity
			XMStoreFloat3(&expl->Velocity, XMVectorSubtract(currVel, XMVectorScale(XMLoadFloat3(&expl->Gravity), fElapsedTime)));
			// Update position afterwards based on velocity
			XMStoreFloat3(&expl->Sprite.Position, XMVectorAdd(XMVectorSet(expl->Sprite.Position.x,
																			expl->Sprite.Position.y,
																			expl->Sprite.Position.z, 1),
																XMVectorScale(currVel, fElapsedTime)));
			// Next explosion particle
			expl++;
		}
	}
	
	//--------------------------------------------------------------------------------------
	// Check for collisions
	//--------------------------------------------------------------------------------------

	// Loop enemies
	for (auto enemy = g_enemyInstances.begin(); enemy != g_enemyInstances.end(); ) {
		// Loop projectiles
		for (auto proj = g_Projectiles.begin(); proj != g_Projectiles.end(); ) {		
			// Compare the distance between projectile and enemy against their bounding sphere radii
			if (!XMVector4Greater(XMVector4Length(XMVectorSubtract(XMLoadFloat3(&enemy->Position), XMLoadFloat3(&(proj->Sprite.Position)))),
												XMVectorSet((enemy->Radius * enemy->Scale) + proj->Sprite.Radius, 
															(enemy->Radius * enemy->Scale) + proj->Sprite.Radius,
															(enemy->Radius * enemy->Scale) + proj->Sprite.Radius, 0))) {
				// Apply damage
				enemy->Hitpoints -= proj->Damage;
				// Save hits
				enemy->Hits[enemy->currHit] = proj->Sprite.Position;
				enemy->currHit = enemy->currHit++ % 10;
				// Remove the projectile
				proj = g_Projectiles.erase(proj);
				// If health hits zero
				if (enemy->Hitpoints <= 0) {
					// Create animation and sprite
					SpriteInstance l_NewExpl;
					SpriteVertex l_ExplSprite;
					// Get animation description
					ConfigParser::Animation destroyAnim = g_configParser.GetAnimations()[enemy->DestroyAnim];
					ConfigParser::Animation destroyPart = g_configParser.GetAnimations()[enemy->DestroyPart];
					// Save in struct
					l_ExplSprite.TextureIndex = g_spriteRenderer->getSpriteID(destroyAnim.SpriteName);
					l_ExplSprite.Position = enemy->Position;
					l_ExplSprite.Radius = destroyAnim.SpriteRadius;
					l_ExplSprite.Progress = 0.0f;
					l_ExplSprite.Alpha = 1.0f;
					// Save in animation
					l_NewExpl.Sprite = l_ExplSprite;
					l_NewExpl.LifeTime = destroyAnim.Duration;
					l_NewExpl.Gravity = XMFLOAT3(0, 0, 0);
					l_NewExpl.Velocity = XMFLOAT3(0, 0, 0);
					// Save in explosions
					g_Explosions.push_back(l_NewExpl);
					// Random number of explosion particles (10-20)
					int partAmount = rand() % 10 + 10;
					// Create instances
					for (int i = 0; i < partAmount; i++) {
						// Random directional values
						float rndA = DEG2RAD(rand() % 360);
						float rndZ = DEG2RAD(rand() % 180);
						// Create animation and sprite
						SpriteInstance l_NewPart;
						SpriteVertex l_PartSprite;
						// Save in struct
						l_PartSprite.TextureIndex = g_spriteRenderer->getSpriteID(destroyPart.SpriteName);
						l_PartSprite.Position = enemy->Position;
						l_PartSprite.Radius = destroyPart.SpriteRadius;
						l_PartSprite.Progress = 0.0f;
						l_PartSprite.Alpha = 1.0f;
						// Random velocity and gravity
						l_NewPart.Velocity = XMFLOAT3(sinf(rndZ) * cosf(rndA) * PARTVEL,
							sinf(rndZ) * sinf(rndA) * PARTVEL, cosf(rndZ) * PARTVEL);
						l_NewPart.Gravity = XMFLOAT3(0, (float) (rand() % (int)PARTVEL), 0);
						l_NewPart.LifeTime = destroyPart.Duration;
						l_NewPart.Sprite = l_PartSprite;
						// Save in explosions
						g_Explosions.push_back(l_NewPart);
					}
					// Remove enemy
					enemy = g_enemyInstances.erase(enemy);
				}
			}
			else {
				// Just increment
				proj++;
			}
		}
		// Increment enemy if the end isn't reached yet
		if(enemy != g_enemyInstances.end())
			enemy++;
	}

	//--------------------------------------------------------------------------------------

	// Set the light vector
	g_lightDir = XMVectorSet(1, 1, 1, 0);    
	g_lightDir = XMVector3Normalize(g_lightDir);
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	double fTime, float fElapsedTime, void* pUserContext)
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

	// Get RTV and clear with backcolor
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	pd3dImmediateContext->ClearRenderTargetView(pRTV, clearColor);

	// Error handling
	if (g_3DRenderEffect.g_pEffect == NULL) {
		g_txtHelper->Begin();
		g_txtHelper->SetInsertionPos(5, 5);
		g_txtHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
		g_txtHelper->DrawTextLine(L"SHADER ERROR");
		g_txtHelper->End();
		return;
	}

	// Depth buffer rendering first
	pd3dImmediateContext->OMSetRenderTargets(0, NULL, Shield::g_pDepthStencilView);
	pd3dImmediateContext->ClearDepthStencilView(Shield::g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//--------------------------------------------------------------------------------------
	// Terrain
	//--------------------------------------------------------------------------------------

	// Update variables that change once per frame
	XMMATRIX worldViewProj = g_terrainWorld * g_camera.GetViewMatrix() * g_camera.GetProjMatrix();
	// Transpose and inverse the world matrix
	XMMATRIX worldNormal = XMMatrixTranspose(XMMatrixInverse(nullptr, g_terrainWorld));

	// Save in shader
	V(g_3DRenderEffect.g_pWorldMatrix->SetMatrix((float*)&g_terrainWorld));
	V(g_3DRenderEffect.g_pWorldViewProjMatrix->SetMatrix((float*)&worldViewProj));
	V(g_3DRenderEffect.g_pWorldNormalMatrix->SetMatrix((float*)&worldNormal));
	V(g_3DRenderEffect.g_pLightDirVector->SetFloatVector((float*)&g_lightDir));
	V(g_3DRenderEffect.g_pFarPlaneDist->SetFloat(g_camera.GetFarClip()));

	// Perform rendering
	g_terrain.render(pd3dImmediateContext, g_3DRenderEffect.g_pTerrainPass0);

	// Clear the depth stencil and set the normal view
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	// Set the depth texture
	V(g_3DRenderEffect.g_pDepthBuffer2D->SetResource(Shield::g_pDepthSRV));

	// Perform rendering again
	g_terrain.render(pd3dImmediateContext, g_3DRenderEffect.g_pTerrainPass0);

	//--------------------------------------------------------------------------------------
	// Cockpit and ground objects
	//--------------------------------------------------------------------------------------

	// Save objects locally
	vector<ConfigParser::RenderObject> renderObjs = g_configParser.GetRenderObjs();

	// Loop over all the meshes to be rendered
	for (auto it = renderObjs.begin(); it != renderObjs.end(); ++it)
	{
		// Create matrices for mesh transformation
		XMMATRIX mTrans, mScale, mRot;
		mRot = XMMatrixRotationRollPitchYaw(DEG2RAD(it->RotationX), DEG2RAD(it->RotationY), DEG2RAD(it->RotationZ));
		mTrans = XMMatrixTranslation(it->TranslationX, it->TranslationY, it->TranslationZ);
		mScale = XMMatrixScaling(it->Scale, it->Scale, it->Scale);

		// Object to world space for mesh in correct order (for lighting)
		XMMATRIX mWorld = mRot * mTrans * mScale * (it->Classification == "Cockpit" ? g_camera.GetWorldMatrix() : XMMatrixIdentity());
		// Object to clip space (for rendering)
		XMMATRIX mWorldViewProj = mWorld * g_camera.GetViewMatrix() * g_camera.GetProjMatrix();
		// Normals transformation matrix (inverse transposed of world)
		XMMATRIX mWorldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));
		// Store camera position
		XMVECTOR mcameraPosWorld = g_camera.GetEyePt();

		// Save in shader
		V(g_3DRenderEffect.g_pWorldMatrix->SetMatrix((float*)&mWorld));
		V(g_3DRenderEffect.g_pWorldViewProjMatrix->SetMatrix((float*)&mWorldViewProj));
		V(g_3DRenderEffect.g_pWorldNormalMatrix->SetMatrix((float*)&mWorldNormals))
			V(g_3DRenderEffect.g_pCameraPosWorld->SetFloatVector((float*)&mcameraPosWorld));

		// Render the mesh accordingly
		g_Meshes[it->Identifier]->render(pd3dImmediateContext, g_3DRenderEffect.g_pMeshPass1, g_3DRenderEffect.g_pDiffuseTexture2D,
			g_3DRenderEffect.g_pSpecularTexture2D, g_3DRenderEffect.g_pGlowTexture2D, g_3DRenderEffect.g_pNormalTexture2D);
	}

	//--------------------------------------------------------------------------------------
	// Enemies
	//--------------------------------------------------------------------------------------

	// Loop over all the enemy instances
	for (auto it = g_enemyInstances.begin(); it != g_enemyInstances.end(); it++)
	{
		// Get enemy type
		ConfigParser::EnemyType enemyType = g_configParser.GetEnemyTypes()[it->Identifier];
		// Create matrices
		XMMATRIX mTrans, mScale, mRot;
		mRot = XMMatrixRotationRollPitchYaw(DEG2RAD(enemyType.RotationX), DEG2RAD(enemyType.RotationY), DEG2RAD(enemyType.RotationZ));
		mTrans = XMMatrixTranslation(enemyType.TranslationX, enemyType.TranslationX, enemyType.TranslationZ);
		mScale = XMMatrixScaling(enemyType.Scale, enemyType.Scale, enemyType.Scale);
		// Object view transformation
		XMMATRIX mWorld = mScale * mRot *  mTrans;
		// Rotation
		XMMATRIX mRotAnim = XMMatrixRotationY(atan2f(it->Velocity.x, it->Velocity.z));
		// Offset
		XMMATRIX mTransAnim = XMMatrixTranslationFromVector(XMLoadFloat3(&it->Position));
		// Movement matrix
		XMMATRIX mAnim = mRotAnim * mTransAnim;
		// Object to clip space (for rendering)
		XMMATRIX mWorldViewProj = mWorld * mAnim * g_camera.GetViewMatrix() * g_camera.GetProjMatrix();
		// Normals transformation matrix (inverse transposed of world)
		XMMATRIX mWorldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));
		// Store camera position
		XMVECTOR mCameraPosWorld = g_camera.GetEyePt();

		// Save in shader
		V(g_3DRenderEffect.g_pWorldMatrix->SetMatrix((float*)&mWorld));
		V(g_3DRenderEffect.g_pWorldViewProjMatrix->SetMatrix((float*)&mWorldViewProj));
		V(g_3DRenderEffect.g_pWorldNormalMatrix->SetMatrix((float*)&mWorldNormals))
		V(g_3DRenderEffect.g_pCameraPosWorld->SetFloatVector((float*)&mCameraPosWorld));

		// Render the enemy accordingly		
		g_Meshes[enemyType.Mesh]->render(pd3dImmediateContext, g_3DRenderEffect.g_pMeshPass1, g_3DRenderEffect.g_pDiffuseTexture2D,
			g_3DRenderEffect.g_pSpecularTexture2D, g_3DRenderEffect.g_pGlowTexture2D, g_3DRenderEffect.g_pNormalTexture2D);

		// Update matrices to have scaling * animation as world transformation matrix
		mWorld = XMMatrixScaling(enemyType.Size, enemyType.Size, enemyType.Size) * mRotAnim * mTransAnim;
		mWorldViewProj = mWorld * g_camera.GetViewMatrix() * g_camera.GetProjMatrix();
		mWorldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));
		XMVECTOR mCameraLookDir = g_camera.GetLookAtPt();

		// Save in shader
		V(g_3DRenderEffect.g_pWorldMatrix->SetMatrix((float*)&mWorld));
		V(g_3DRenderEffect.g_pWorldViewProjMatrix->SetMatrix((float*)&mWorldViewProj));
		V(g_3DRenderEffect.g_pWorldNormalMatrix->SetMatrix((float*)&mWorldNormals));
		V(g_3DRenderEffect.g_pCameraPosWorld->SetFloatVector((float*)&mCameraLookDir))
		V(g_3DRenderEffect.g_pHitArray->SetFloatVectorArray((float*)&it->Hits, 0, 10));
		
		// Render the shield around the enemy
		g_EnemyShield->render(pd3dImmediateContext, g_3DRenderEffect.g_pShieldPass2, g_3DRenderEffect.g_pDiffuseTexture2D);
	}

	// Unbind the srv to prevent it from being both read/written to
	V(g_3DRenderEffect.g_pDepthBuffer2D->SetResource(NULL));
	g_3DRenderEffect.g_pShieldPass2->Apply(0, pd3dImmediateContext);

	//--------------------------------------------------------------------------------------
	// Render sprites last (transparent after opaque)
	//--------------------------------------------------------------------------------------

	// If there are projectiles
	if (!g_Projectiles.empty()) {
		// Push sprites into a vector
		vector<SpriteVertex> l_CurrSprites;
		for (auto it : g_Projectiles) {
			l_CurrSprites.push_back(it.Sprite);
		}
		for (auto it : g_Explosions) {
			l_CurrSprites.push_back(it.Sprite);
		}
		// Sort based on distance to camera
		sort(l_CurrSprites.begin(), l_CurrSprites.end(),
			[](SpriteVertex i, SpriteVertex j)
		{
			return XMVector4Greater(XMVector3Dot(g_camera.GetWorldAhead(), XMLoadFloat3(&i.Position)),
				XMVector3Dot(g_camera.GetWorldAhead(), XMLoadFloat3(&j.Position)));
		});

		// Render them
		g_spriteRenderer->renderSprites(pd3dImmediateContext, l_CurrSprites, g_camera);
	}

	//--------------------------------------------------------------------------------------

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