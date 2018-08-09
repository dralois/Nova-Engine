#pragma once

#include "DXUT.h"
#include "DirectXTex.h"
#include "d3dx11effect.h"
#include <DDSTextureLoader.h>

#include "ConfigParser.h"
#include "Effect3D.h"
#include "SimpleImage.h"
#include "Util.h"

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
	int m_iResolution;
	ConfigParser m_ConfigParser;

	// Heightfield
	vector<float> m_arrHeightfield;

	// Terrain rendering resources
	ID3D11Buffer*                           m_pIndexBuffer;			// The terrain's triangulation
	ID3D11Buffer*							m_pHeightBuffer;		// Heightmap buffer
	ID3D11ShaderResourceView*				m_pHeightBufferSRV;		// Heightmap desc
	ID3D11Texture2D*                        m_pNormalTexture;		// Normalmap texure
	ID3D11ShaderResourceView*				m_pNormalTextureSRV;	// Normalmap desc
	ID3D11Texture2D*                        m_pDiffuseTexture;		// The terrain's material color for diffuse lighting
	ID3D11ShaderResourceView*               m_pDiffuseTextureSRV;	// Describes the structure of the diffuse texture to the shader stages
};

