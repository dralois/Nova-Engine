#pragma once

#include "ConfigParser.h"
#include "d3dx11effect.h"
#include "SDKmisc.h"
#include "DXUT.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// Convenience macros for safe effect variable retrieval
#define SAFE_GET_PASS(Technique, name, var)   {assert(Technique!=NULL); var = Technique->GetPassByName( name );						assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var) {assert(effect!=NULL); var = effect->GetTechniqueByName( name );						assert(var->IsValid());}
#define SAFE_GET_SCALAR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsScalar();			assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();			assert(var->IsValid());}
#define SAFE_GET_MATRIX(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();			assert(var->IsValid());}
#define SAFE_GET_SAMPLER(effect, name, var)   {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsSampler();			assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)  {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();	assert(var->IsValid());}

struct GameEffect
{
	// A D3DX rendering effect
	ID3DX11Effect*                          g_pEffect;					// The whole rendering effect
	ID3DX11EffectTechnique*                 g_pTechnique;				// One technique to render all the effects

	ID3DX11EffectPass*                      g_pTerrainPass0;			// Terrain rendering pass
	ID3DX11EffectPass*                      g_pMeshPass1;				// Mesh rendering pass
	ID3DX11EffectPass*						g_pShieldPass2;				// Shield rendering pass
	
	ID3DX11EffectMatrixVariable*            g_pWorldMatrix;				// World transformation matrix effect variable
	ID3DX11EffectMatrixVariable*			g_pWorldNormalMatrix;		// Matrix for normal tranformation
	ID3DX11EffectMatrixVariable*            g_pWorldViewProjMatrix;		// WorldViewProjection matrix effect variable
	ID3DX11EffectVectorVariable*            g_pLightDirVector;			// Light direction in world space
	ID3DX11EffectVectorVariable*            g_pCameraPosWorld;			// Camera position in world space
	
	ID3DX11EffectShaderResourceVariable*    g_pDiffuseTexture2D;		// Texture for the diffuse color
	ID3DX11EffectShaderResourceVariable*    g_pSpecularTexture2D;		// Texture for the specular color
	ID3DX11EffectShaderResourceVariable*    g_pGlowTexture2D;			// Texture for the glow color
	ID3DX11EffectShaderResourceVariable*	g_pNormalTexture2D;			// Texture for normals

	ID3DX11EffectVectorVariable*			g_pHitArray;				// Array of shield hits
	ID3DX11EffectScalarVariable*			g_pFarPlaneDist;			// Far plane distance
	ID3DX11EffectShaderResourceVariable*    g_pDepthBuffer2D;			// Texture for the glow color

	ID3DX11EffectShaderResourceVariable*	g_pTerrainHeightTexture2D;	// Heightmap texture
	ID3DX11EffectScalarVariable*			g_pTerrainResolution;		// Resolution of the terrain

	GameEffect() { ZeroMemory(this, sizeof(*this)); }

	HRESULT create(ID3D11Device* device, ConfigParser parser)
	{
		HRESULT hr;
		WCHAR path[MAX_PATH];

		// Find and load the rendering effect
		std::string shaderFolder = parser.GetShaderFolder();
		std::wstring shaderPath = std::wstring(shaderFolder.begin(), shaderFolder.end()) + L"3DRenderer.fxo";
		V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, shaderPath.c_str()));
		std::ifstream is(path, std::ios_base::binary);
		is.seekg(0, std::ios_base::end);
		std::streampos pos = is.tellg();
		is.seekg(0, std::ios_base::beg);
		std::vector<char> effectBuffer((unsigned int)pos);
		is.read(&effectBuffer[0], pos);
		is.close();
		V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, device, &g_pEffect));    
		assert(g_pEffect->IsValid());

		// Obtain the effect technique
		SAFE_GET_TECHNIQUE(g_pEffect, "Render", g_pTechnique);

		// Obtain the effect passes
		SAFE_GET_PASS(g_pTechnique, "P0_Terrain", g_pTerrainPass0);
		SAFE_GET_PASS(g_pTechnique, "P1_Mesh", g_pMeshPass1);
		SAFE_GET_PASS(g_pTechnique, "P2_Shield", g_pShieldPass2);

		// Obtain vars for general math
		SAFE_GET_MATRIX(g_pEffect, "g_World", g_pWorldMatrix);
		SAFE_GET_MATRIX(g_pEffect, "g_WorldNormals", g_pWorldNormalMatrix);
		SAFE_GET_MATRIX(g_pEffect, "g_WorldViewProjection", g_pWorldViewProjMatrix);
		SAFE_GET_VECTOR(g_pEffect, "g_LightDir", g_pLightDirVector);
		SAFE_GET_VECTOR(g_pEffect, "g_CameraPos", g_pCameraPosWorld);
		// Obtain vars for textures
		SAFE_GET_RESOURCE(g_pEffect, "g_Diffuse", g_pDiffuseTexture2D);
		SAFE_GET_RESOURCE(g_pEffect, "g_Specular", g_pSpecularTexture2D);
		SAFE_GET_RESOURCE(g_pEffect, "g_Glow", g_pGlowTexture2D);
		SAFE_GET_RESOURCE(g_pEffect, "g_Normal", g_pNormalTexture2D);
		// Obtain vars for the shield effect
		SAFE_GET_VECTOR(g_pEffect, "g_Hits", g_pHitArray);
		SAFE_GET_SCALAR(g_pEffect, "g_FarPlaneDist", g_pFarPlaneDist);
		SAFE_GET_RESOURCE(g_pEffect, "g_Depth", g_pDepthBuffer2D);
		// Obtain vars for the terrain
		SAFE_GET_RESOURCE(g_pEffect, "g_HeightMap", g_pTerrainHeightTexture2D);
		SAFE_GET_SCALAR(g_pEffect, "g_TerrainRes", g_pTerrainResolution);

		return S_OK;
	}

	void destroy()
	{
		SAFE_RELEASE(g_pEffect);
	}
};

extern GameEffect g_3DRenderEffect;