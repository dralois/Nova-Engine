#pragma once

#include <vector>

#include <DXUT.h>
#include <DirectXMath.h>
#include <d3dx11effect.h>
#include <DDSTextureLoader.h>

#include "Effect3D.h"
#include "Util.h"

// Shield vertex
struct ShieldVertexIn {
	DirectX::XMVECTOR Position;
	DirectX::XMFLOAT2 TexCoord;
};

class Shield
{
public:
	// Constructor & Destructor
	Shield();
	~Shield();

	// Create the resources
	HRESULT create(ID3D11Device* pDevice, int loopcount, const string& resourcePath);
	// Release the resources
	void destroy();

	// Create depth buffer
	static HRESULT createDepthBuffer(ID3D11Device* pDevice, UINT width, UINT height);
	// Destroy depth buffer
	static void destroyDepthBuffer();

	// Create the (static!) input layout
	static HRESULT createInputLayout(ID3D11Device* device, ID3DX11EffectPass* pass);
	// Destroy the (static!) input layout
	static void destroyInputLayout();

	// Render the effect
	HRESULT render(	ID3D11DeviceContext* context, ID3DX11EffectPass* pass,
					ID3DX11EffectShaderResourceVariable* diffuseEV);

	// Depth buffer as texture
	static ID3D11ShaderResourceView*	g_pDepthSRV;
	// Depth buffer view
	static ID3D11DepthStencilView*		g_pDepthStencilView;

private:
	// A texture that is a tileable force shield
	ID3D11ShaderResourceView*		m_pShieldTexSRV;
	ID3D11Texture2D*				m_pShieldTex2D;

	// Vertex buffer
	ID3D11Buffer*					m_pVertexBuffer;
	int								m_iVertexCount;

	// Input layout
	static ID3D11InputLayout*		m_pInputLayout;
};