#pragma once

#include "DXUT.h"
#include "DirectXTex.h"
#include "d3dx11effect.h"
#include <DDSTextureLoader.h>

#include "ConfigParser.h"
#include "GameEffect.h"
#include "SimpleImage.h"
#include "Util.cpp"

#include "debug.h"

class Terrain
{
public:	
	// Constructor / Destructor
	Terrain(void);
	~Terrain(void);

	// Returns height at a position
	float GetHeightAtXY(float pi_dX, float pi_dY);

	HRESULT create(ID3D11Device* device, ConfigParser parser);
	void destroy();

	void render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass);


private:

	// Configuration
	int iResolution;
	ConfigParser configParser;

	// Heightfield
	vector<float> dHeightfield;

	// Terrain rendering resources
	ID3D11Buffer*                           indexBuffer;		// The terrain's triangulation
	ID3D11Buffer*							heightBuffer;		// Heightmap buffer
	ID3D11ShaderResourceView*				heightBufferSRV;	// Heightmap desc
	ID3D11Texture2D*                        normalTexture;		// Normalmap texure
	ID3D11ShaderResourceView*				normalTextureSRV;	// Normalmap desc
	ID3D11Texture2D*                        diffuseTexture;		// The terrain's material color for diffuse lighting
	ID3D11ShaderResourceView*               diffuseTextureSRV;	// Describes the structure of the diffuse texture to the shader stages
};

