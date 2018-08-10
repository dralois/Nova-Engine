#pragma once

#include <DXUT.h>
#include <DXUTcamera.h>
#include <d3dx11effect.h>
#include <DDSTextureLoader.h>

#include "Util.h"
#include "Effect2D.h"

#include "Debug.h"

class Skybox
{
public:
	// Constructor and destructor (does nothing)
	Skybox();
	~Skybox();

	// Create all required resources
	HRESULT create(ID3D11Device* pDevice, ID3DX11EffectPass* pass, const string& resourcePath);
	// Release resources again
	void destroy();

	// Render the given skybox
	void render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass,
				const CFirstPersonCamera& camera);

private:
	// Skybox texture and SRV
	ID3D11Resource*					m_pSkyboxTex;
	ID3D11ShaderResourceView*		m_pSkyboxSRV;
};