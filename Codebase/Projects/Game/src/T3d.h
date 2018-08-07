#pragma once

#include <cstdint>
#include <vector>
#include <DXUT.h>
#include <d3dx11effect.h>

// Struct for mesh vertex data
struct T3dVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
};

class T3d
{
public:
	static HRESULT readFromFile(const std::string& filename, std::vector<T3dVertex>& vertexBufferData, 
                                                      std::vector<uint32_t>& indexBufferData);
													  
	static HRESULT readFromFile(const std::wstring& filename, std::vector<T3dVertex>& vertexBufferData, 
                                                      std::vector<uint32_t>& indexBufferData);

	static HRESULT createT3dInputLayout(ID3D11Device* pd3dDevice, 
		ID3DX11EffectPass* pass, ID3D11InputLayout** t3dInputLayout);
};