#pragma once
#include "DXUT.h"
#include "d3dx11effect.h"
#include "ConfigParser.h"

class Terrain
{
public:
	
	//Struct für Vertexdaten
	struct SimpleVertex {
		DirectX::XMFLOAT4 Pos;
		DirectX::XMFLOAT4 Normal;
		DirectX::XMFLOAT2 UV;
	};

	Terrain(void);
	~Terrain(void);

	HRESULT create(ID3D11Device* device, ConfigParser parser);
	void destroy();

	void render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass);


private:
	Terrain(const Terrain&);
	Terrain(const Terrain&&);
	void operator=(const Terrain&);

	// Configuration
	ConfigParser configParser;
	int resolution;

	// Terrain rendering resources
	ID3D11Buffer*                           indexBuffer;		// The terrain's triangulation
	ID3D11Buffer*							heightBuffer;		// Heightmap buffer
	ID3D11ShaderResourceView*				heightBufferSRV;	// Heightmap desc
	ID3D11Texture2D*                        normalTexture;		// Normalmap texure
	ID3D11ShaderResourceView*				normalTextureSRV;	// Normalmap desc
	ID3D11Texture2D*                        diffuseTexture;		// The terrain's material color for diffuse lighting
	ID3D11ShaderResourceView*               diffuseTextureSRV;	// Describes the structure of the diffuse texture to the shader stages
};

