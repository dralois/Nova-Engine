#pragma once

#include "T3d.h"

#include <DXUT.h>
#include <d3dx11effect.h>
#include <DDSTextureLoader.h>

#include <vector>
#include <cstdint>

//This class ecapsulates the D3D11 resources needed for a mesh
class Mesh
{
public:
	//Create mesh object and set filenames
	Mesh(	const std::string& filenameT3D,					// Filename of mesh geometry
			const std::string& filenameDDSDiffuse,			// Filename of diffuse texture
			const std::string& filenameDDSSpecular,			// Filename of specular texture
			const std::string& filenameDDSGlow,				// Filename of glow texture
			const std::string& filenameDDSNormal,			// Filename of normal texture
			const std::string& filenameDDSTransparency);	// Filename of transparency texture
		 
	Mesh(	const std::wstring& filenameT3D,				// Filename of mesh geometry
			const std::wstring& filenameDDSDiffuse,			// Filename of diffuse texture
			const std::wstring& filenameDDSSpecular,		// Filename of specular texture
			const std::wstring& filenameDDSGlow,			// Filename of glow texture
			const std::wstring& filenameDDSNormal,			// Filename of normal texture
			const std::wstring& filenameDDSTransparency);	// Filename of transparency texture

	//Currently does nothing
	~Mesh(void);

	//Creates the required D3D11 resources from the given input files
	HRESULT create(ID3D11Device* device);

	//Releases all D3D11 resources of the mesh
	void destroy();

	// Creates an input layout which is used for meshes
	static HRESULT createInputLayout(ID3D11Device* device, ID3DX11EffectPass* pass);

	// Releases the input layout
	static void destroyInputLayout();

	// Render the mesh
	HRESULT render(	ID3D11DeviceContext* context, ID3DX11EffectPass* pass,
					ID3DX11EffectShaderResourceVariable* diffuseEV,
					ID3DX11EffectShaderResourceVariable* specularEV,
					ID3DX11EffectShaderResourceVariable* glowEV,
					ID3DX11EffectShaderResourceVariable* normalEV,
					ID3DX11EffectShaderResourceVariable* transparencyEV);

private:
	// Reads the complete file given by "path" byte-wise into "data".
	static HRESULT loadFile(const char * filename, std::vector<uint8_t>& data);

	// Creates DX Texture Resources from File
	static HRESULT createTexture(	ID3D11Device* device, const std::wstring& filename, 
									ID3D11Texture2D** tex, ID3D11ShaderResourceView** srv);
	
private:
	// Filenames
	std::wstring		m_sFilenameT3d;
	std::wstring		m_sFilenameDDSDiffuse;
	std::wstring		m_sFilenameDDSSpecular;
	std::wstring		m_sFilenameDDSGlow;
	std::wstring		m_sFilenameDDSNormal;
	std::wstring		m_sFilenameDDSTransparency;

	// Mesh geometry information
	ID3D11Buffer*       m_pVertexBuffer;
	ID3D11Buffer*		m_pIndexBuffer;
	int					m_iIndexCount;

	// Mesh textures and corresponding shader resource views
	ID3D11Texture2D*            m_pDiffuseTex;
	ID3D11ShaderResourceView*   m_pDiffuseSRV;
	ID3D11Texture2D*            m_pSpecularTex;
	ID3D11ShaderResourceView*   m_pSpecularSRV;
	ID3D11Texture2D*	        m_pGlowTex;
	ID3D11ShaderResourceView*   m_pGlowSRV;
	ID3D11Texture2D*	        m_pNormalTex;
	ID3D11ShaderResourceView*   m_pNormalSRV;
	ID3D11Texture2D*	        m_pTransparencyTex;
	ID3D11ShaderResourceView*   m_pTransparencySRV;

	// Mesh input layout
	static ID3D11InputLayout*	m_pInputLayout;
};