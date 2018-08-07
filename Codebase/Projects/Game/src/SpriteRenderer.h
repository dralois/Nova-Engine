#pragma once

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <DXUT.h>
#include "SDKmisc.h"
#include <DXUTcamera.h>
#include <d3dx11effect.h>
#include <DDSTextureLoader.h>

#include "Util.cpp"
#include "ConfigParser.h"
#include "3DRenderer.h"

#include "debug.h"

struct SpriteVertex
{
	DirectX::XMFLOAT3 Position;     // world-space position
	float Radius;                   // world-space radius
	int TextureIndex;               // which texture to use
	float Progress;					// Animation progress
	float Alpha;					// Additional alpha
};

class SpriteRenderer
{
public:
	// Constructor: Create a SpriteRenderer
	// The textures are *not* be created immediately, but only when create is called!
	SpriteRenderer(const ConfigParser parser);
	// Destructor does nothing. Destroy and ReleaseShader must be called first!
	~SpriteRenderer();

	// Returns internal index to a sprite name
	int getSpriteID(const string& spriteName);

	// Load/reload the effect. Must be called once before create!
	HRESULT reloadShader(ID3D11Device* pDevice);
	// Release the effect again.
	void releaseShader();

	// Create all required D3D resources (textures, buffers, ...).
	// reloadShader must be called first!
	HRESULT create(ID3D11Device* pDevice);
	// Release D3D resources again.
	void destroy();

	// Render the given sprites. They must already be sorted into back-to-front order.
	void renderSprites(ID3D11DeviceContext* context, 
		const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera);

private:
	// Contains sprite info and other necessary to know things
	ConfigParser							m_configParser;

	// Rendering effect (shaders and related GPU state). Created/released in Reload/ReleaseShader.
	ID3DX11Effect*							m_pEffect;
	
	// Matrix for sprite correction
	ID3DX11EffectMatrixVariable*			m_pViewProjectionMatrix;
	// Right vector
	ID3DX11EffectVectorVariable*			m_pCameraRight;
	// Up vector
	ID3DX11EffectVectorVariable*			m_pCameraUp;
	// Variable for the sprite texture
	ID3DX11EffectShaderResourceVariable*    m_pTextureArray2D;

	// Sprite textures and corresponding shader resource views.
	std::vector<ID3D11ShaderResourceView*>	m_pSpriteSRV;
	std::map<string, int>					m_dicSpriteIDs;

	// Vertex buffer for sprite vertices, and corresponding input layout.
	ID3D11Buffer*							m_pVertexBuffer;
	ID3D11InputLayout*						m_pInputLayout;
};
