#include "Skybox.h"

#define SKYBOXTEX "skybox.DDS"

using namespace DirectX;

#pragma region Procedures

HRESULT Skybox::create(ID3D11Device* pDevice, ID3DX11EffectPass* pass, const string& resourcePath)
{
	HRESULT hr;

	// Create and load skybox texture
	wstring texPath = Util::toWString(resourcePath + SKYBOXTEX);
	V(DirectX::CreateDDSTextureFromFile(pDevice, texPath.c_str(), &m_pSkyboxTex, &m_pSkyboxSRV));

	return S_OK;
}

void Skybox::destroy()
{
	SAFE_RELEASE(m_pSkyboxTex);
	SAFE_RELEASE(m_pSkyboxSRV);
}

void Skybox::render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass,
					const CFirstPersonCamera& camera)
{
	HRESULT hr;

	// Tell the input assembler stage which primitive topology to use
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->IASetInputLayout(NULL);

	// Update camera position
	V(g_Effect2D.g_pCameraPos->SetFloatVector((float*)&camera.GetEyePt()));

	// Calculate inverse view projection matrix and apply
	XMMATRIX mViewProjInv = XMMatrixInverse(nullptr, camera.GetViewMatrix() * camera.GetProjMatrix());
	V(g_Effect2D.g_pViewProjectionMatrixInv->SetMatrix((float*)&mViewProjInv));

	// Set skybox cubemap
	V(g_Effect2D.g_pSkybox->SetResource(m_pSkyboxSRV));

	// Apply the changes
	V(pass->Apply(0, context));

	// Draw skybox
	context->Draw(1, 0);
}

#pragma endregion

#pragma region Constructor & Destructor

Skybox::Skybox():
	m_pSkyboxSRV(NULL),
	m_pSkyboxTex(NULL)
{
}

Skybox::~Skybox()
{
}

#pragma endregion