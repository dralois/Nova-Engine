#pragma once

#include "ConfigParser.h"
#include "d3dx11effect.h"
#include "SDKmisc.h"
#include "DXUT.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Util.h"

struct Effect2D
{
	ID3DX11Effect*                          g_pEffect;					// The whole rendering effect
	ID3DX11EffectTechnique*                 g_pTechnique;				// One technique to render all the effects

	ID3DX11EffectPass*                      g_pSpritePass;				// Sprite rendering pass
	ID3DX11EffectPass*                      g_pSkyboxPass;				// Skybox rendering pass

	ID3DX11EffectMatrixVariable*			g_pViewProjectionMatrix;	// Matrix for sprite correction
	ID3DX11EffectVectorVariable*			g_pCameraRight;				// Right vector
	ID3DX11EffectVectorVariable*			g_pCameraUp;				// Up vector

	ID3DX11EffectShaderResourceVariable*    g_pTextureArray2D;			// Variable for the sprite texture

	bool									m_fIsCreated = false;		// Has the effect been created?

	// Deletes the effect!
	Effect2D() { ZeroMemory(this, sizeof(*this)); }

	HRESULT create(ID3D11Device* device, ConfigParser parser)
	{
		HRESULT hr;
		WCHAR path[MAX_PATH];

		// Find and load the rendering effect
		std::string shaderFolder = parser.GetShaderFolder();
		std::wstring shaderPath = std::wstring(shaderFolder.begin(), shaderFolder.end()) + L"2DShaders.fxo";
		V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, shaderPath.c_str()));
		std::ifstream is(path, std::ios_base::binary);
		is.seekg(0, std::ios_base::end);
		std::streampos pos = is.tellg();
		is.seekg(0, std::ios_base::beg);
		std::vector<char> effectBuffer((unsigned int)pos);
		is.read(&effectBuffer[0], pos);
		is.close();

		// Create the effect
		V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, device, &g_pEffect));
		assert(g_pEffect->IsValid());

		// Effect now valid
		m_fIsCreated = true;

		// Obtain the technique
		SAFE_GET_TECHNIQUE(g_pEffect, "Render", g_pTechnique);
		// Obtain the passes
		SAFE_GET_PASS(g_pTechnique, "P0_Sprite", g_pSpritePass);
		SAFE_GET_PASS(g_pTechnique, "P1_Skybox", g_pSkyboxPass);
		// Obtain general math vars
		SAFE_GET_MATRIX(g_pEffect, "g_ViewProjection", g_pViewProjectionMatrix);
		SAFE_GET_VECTOR(g_pEffect, "g_CameraRight", g_pCameraRight);
		SAFE_GET_VECTOR(g_pEffect, "g_CameraUp", g_pCameraUp);
		// Obtain texture vars
		SAFE_GET_RESOURCE(g_pEffect, "g_Textures", g_pTextureArray2D);

		return S_OK;
	}

	void destroy()
	{
		// Destroy only if effect was created
		if (m_fIsCreated)
			SAFE_RELEASE(g_pEffect);
	}
};

extern Effect2D g_Effect2D;