#include "Mesh.h"

ID3D11InputLayout*	Mesh::m_pInputLayout;

#pragma region Procedures

HRESULT Mesh::create(ID3D11Device* device)
{	
	HRESULT hr;

	//Some needed variables
	D3D11_SUBRESOURCE_DATA id = {0};
	D3D11_BUFFER_DESC bd = {0};
	std::vector<T3dVertex> vertexBufferData;
	std::vector<uint32_t> indexBufferData;

	//Read mesh
	V(T3d::readFromFile(m_sFilenameT3d.c_str(), vertexBufferData, indexBufferData));
	
	// Define vertex buffer
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = vertexBufferData.size() * sizeof(T3dVertex);
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	// Fill with data
	id.pSysMem = &vertexBufferData[0];
	id.SysMemPitch = sizeof(T3dVertex);
    id.SysMemSlicePitch = 0;

	// Create buffer
	V(device->CreateBuffer(&bd, &id, &m_pVertexBuffer));

	// Save index count
	m_iIndexCount = indexBufferData.size();

	// Define index buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(unsigned int) * m_iIndexCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	
	// Fill with data
	ZeroMemory(&id, sizeof(id));
	id.pSysMem = &indexBufferData[0];
	
	// Create Buffer
	V(device->CreateBuffer(&bd, &id, &m_pIndexBuffer));

	// Create textures
	V(createTexture(device, m_sFilenameDDSDiffuse, &m_pDiffuseTex, &m_pDiffuseSRV));
	V(createTexture(device, m_sFilenameDDSSpecular, &m_pSpecularTex, &m_pSpecularSRV));
	V(createTexture(device, m_sFilenameDDSGlow, &m_pGlowTex, &m_pGlowSRV));
	V(createTexture(device, m_sFilenameDDSNormal, &m_pNormalTex, &m_pNormalSRV));
	V(createTexture(device, m_sFilenameDDSTransparency, &m_pTransparencyTex, &m_pTransparencySRV));

	return S_OK;
}

void Mesh::destroy()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pDiffuseTex);
	SAFE_RELEASE(m_pDiffuseSRV);
	SAFE_RELEASE(m_pSpecularTex);
	SAFE_RELEASE(m_pSpecularSRV);
	SAFE_RELEASE(m_pGlowTex);
	SAFE_RELEASE(m_pGlowSRV);
	SAFE_RELEASE(m_pNormalTex);
	SAFE_RELEASE(m_pNormalSRV);
	SAFE_RELEASE(m_pTransparencyTex);
	SAFE_RELEASE(m_pTransparencySRV);
}

HRESULT Mesh::createInputLayout(ID3D11Device* device, ID3DX11EffectPass* pass)
{
	HRESULT hr;
	V(T3d::createT3dInputLayout(device, pass, &m_pInputLayout));
	return S_OK;
}

void Mesh::destroyInputLayout()
{
	SAFE_RELEASE(m_pInputLayout);
}

HRESULT Mesh::render(	ID3D11DeviceContext* context, ID3DX11EffectPass* pass, 
						ID3DX11EffectShaderResourceVariable* diffuseEV,
						ID3DX11EffectShaderResourceVariable* specularEV,
						ID3DX11EffectShaderResourceVariable* glowEV,
						ID3DX11EffectShaderResourceVariable* normalEV,
						ID3DX11EffectShaderResourceVariable* transparencyEV)
{
	HRESULT hr;

	// Check if textures exist
    if ((diffuseEV == nullptr) || !diffuseEV->IsValid())
    {
        throw std::exception("Diffuse EV is null or invalid");
    }
    if ((specularEV == nullptr) || !specularEV->IsValid())
    {
        throw std::exception("Specular EV is null or invalid");
    }
    if ((glowEV == nullptr) || !glowEV->IsValid())
    {
        throw std::exception("Glow EV is null or invalid");
    }
	if ((normalEV == nullptr) || !normalEV->IsValid())
	{
		throw std::exception("Normal EV is null or invalid");
	}
	if ((transparencyEV == nullptr) || !transparencyEV->IsValid())
	{
		throw std::exception("Transparency EV is null or invalid");
	}

	// Set textures
	V(diffuseEV->SetResource(m_pDiffuseSRV));
	V(specularEV->SetResource(m_pSpecularSRV));
	V(glowEV->SetResource(m_pGlowSRV));
	V(normalEV->SetResource(m_pNormalSRV));
	V(transparencyEV->SetResource(m_pTransparencySRV));

	// Bind the terrain vertex buffer to the input assembler stage 
	ID3D11Buffer* vbs[] = { m_pVertexBuffer, };
    unsigned int strides[] = {sizeof(T3dVertex), }, offsets[] = { 0, };
    context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

    // Tell the input assembler stage which primitive topology to use
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_pInputLayout);

	// Apply the pass
	V(pass->Apply(0, context));

	// Render the mesh
	context->DrawIndexed(m_iIndexCount, 0, 0);

	return S_OK;	
}

HRESULT Mesh::loadFile(const char * filename, std::vector<uint8_t>& data)
{
	FILE * filePointer = NULL;
    errno_t error = fopen_s(&filePointer, filename, "rb");
	if (error) 	{ return E_INVALIDARG; }
	fseek(filePointer, 0, SEEK_END);
	long bytesize = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);
	data.resize(bytesize);
	fread(&data[0], 1, bytesize, filePointer);
	fclose(filePointer);
	return S_OK;
}

HRESULT Mesh::createTexture(ID3D11Device* device, const std::wstring& filename,
							ID3D11Texture2D** tex, ID3D11ShaderResourceView** srv)
{
	HRESULT hr;
	
	// If filename is not '-' (no texture given)
	if (filename.c_str()[filename.length() - 1] != '-') {
		// Create the texture and SRV
		V_RETURN(DirectX::CreateDDSTextureFromFile(device, filename.c_str(), (ID3D11Resource**)tex, srv));
	}

	return S_OK;
}

#pragma endregion

#pragma region Constructors & Destructors

Mesh::Mesh(	const std::string& filenameT3d,
			const std::string& filenameDDSDiffuse,
			const std::string& filenameDDSSpecular,
			const std::string& filenameDDSGlow,
			const std::string& filenameDDSNormal,
			const std::string& filenameDDSTransparency):
	m_pVertexBuffer(NULL), m_pIndexBuffer(NULL),
	m_iIndexCount(0),
	m_pDiffuseTex(NULL), m_pDiffuseSRV(NULL),
	m_pSpecularTex(NULL), m_pSpecularSRV(NULL),
	m_pGlowTex(NULL), m_pGlowSRV(NULL),
	m_pNormalTex(NULL), m_pNormalSRV(NULL),
	m_pTransparencyTex(NULL), m_pTransparencySRV(NULL)
{
	m_sFilenameT3d = std::wstring(filenameT3d.begin(), filenameT3d.end());
	m_sFilenameDDSDiffuse = std::wstring(filenameDDSDiffuse.begin(), filenameDDSDiffuse.end());
	m_sFilenameDDSSpecular = std::wstring(filenameDDSSpecular.begin(), filenameDDSSpecular.end());
	m_sFilenameDDSGlow = std::wstring(filenameDDSGlow.begin(), filenameDDSGlow.end());
	m_sFilenameDDSNormal = std::wstring(filenameDDSNormal.begin(), filenameDDSNormal.end());
	m_sFilenameDDSTransparency = std::wstring(filenameDDSTransparency.begin(), filenameDDSTransparency.end());
}

Mesh::Mesh(	const std::wstring& filenameT3d,
			const std::wstring& filenameDDSDiffuse,
			const std::wstring& filenameDDSSpecular,
			const std::wstring& filenameDDSGlow,
			const std::wstring& filenameDDSNormal,
			const std::wstring& filenameDDSTransparency):
	m_sFilenameT3d(filenameT3d),
	m_sFilenameDDSDiffuse(filenameDDSDiffuse),
	m_sFilenameDDSSpecular(filenameDDSSpecular),
	m_sFilenameDDSGlow(filenameDDSGlow),
	m_sFilenameDDSNormal(filenameDDSNormal),
	m_sFilenameDDSTransparency(filenameDDSTransparency),
	m_pVertexBuffer(NULL), m_pIndexBuffer(NULL),
	m_iIndexCount(0),
	m_pDiffuseTex(NULL), m_pDiffuseSRV(NULL),
	m_pSpecularTex(NULL), m_pSpecularSRV(NULL),
	m_pGlowTex(NULL), m_pGlowSRV(NULL),
	m_pNormalTex(NULL), m_pNormalSRV(NULL),
	m_pTransparencyTex(NULL), m_pTransparencySRV(NULL)
{
}

Mesh::~Mesh(void)
{
}

#pragma endregion