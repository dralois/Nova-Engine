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

#include "Util.h"
#include "Effect2D.h"
#include "ConfigParser.h"

#include "Debug.h"

struct SpriteVertex
{
	DirectX::XMFLOAT3 Position;     // world-space position
	float Radius;                   // world-space radius
	int TextureIndex;               // which texture to use
	float Progress;					// Animation progress
	float Alpha;					// Additional alpha
};

class Sprites
{
public:
	// Constructor and destructor (does nothing)
	Sprites();
	~Sprites();

	// Returns internal index to a sprite name
	int getSpriteID(const string& spriteName);

	// Create all required resources (textures, buffers, ...)
	HRESULT create(ID3D11Device* pDevice, ID3DX11EffectPass* pass, ConfigParser& parser);
	// Release resources again.
	void destroy();

	// Creates an input layout which is used for meshes
	static HRESULT createInputLayout(ID3D11Device* device, ID3DX11EffectPass* pass);
	// Releases the input layout
	static void destroyInputLayout();

	// Render the given sprites. They must already be sorted into back-to-front order.
	void renderSprites(	ID3D11DeviceContext* context, ID3DX11EffectPass* pass,
						const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera);

private:
	// Sprite textures and corresponding shader resource views.
	std::vector<ID3D11ShaderResourceView*>	m_pSpriteSRVs;
	std::map<string, int>					m_dicSpriteIDs;

	// Vertex buffer for sprite vertices
	ID3D11Buffer*							m_pVertexBuffer;

	// Sprite input layout
	static ID3D11InputLayout*				m_pInputLayout;
};
