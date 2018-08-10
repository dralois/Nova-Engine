#include "Sprites.h"

#define MAXSPRITES 1024

using namespace DirectX;

ID3D11InputLayout*	Sprites::m_pInputLayout;

#pragma region Procedures

int Sprites::getSpriteID(const string& spriteName)
{
	// Return index or null
	auto l_Found = m_dicSpriteIDs.find(spriteName);
	return l_Found != m_dicSpriteIDs.end() ? l_Found->second : 0;
}

HRESULT Sprites::create(ID3D11Device* pDevice, ID3DX11EffectPass* pass, ConfigParser& parser)
{	
	HRESULT hr;

	// Buffer desc
	D3D11_BUFFER_DESC bd = { 0 };
	bd.MiscFlags = 0;
	bd.CPUAccessFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(SpriteVertex) * MAXSPRITES;

	// Create vertex buffer
	V(pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer));

	// Save sprites locally
	map<string, ConfigParser::SpriteTexture> l_Sprites = parser.GetSpriteTextures();
	int l_iCount = 0;

	// Create the textures
	for (auto it = l_Sprites.begin(); it != l_Sprites.end(); it++) {
		ID3D11ShaderResourceView * nextView;
		wstring nextPath = Util::toWString(parser.GetResourceFolder() + it->second.FilePath);
		V_RETURN(DirectX::CreateDDSTextureFromFile(pDevice, nextPath.c_str(), nullptr, &nextView));
		m_pSpriteSRVs.push_back(nextView);
		m_dicSpriteIDs[it->first] = l_iCount++;
	}

	return S_OK;
}

void Sprites::destroy()
{
	// Release all resources
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	// Erase the SRVs
	for (auto it = m_pSpriteSRVs.begin(); it != m_pSpriteSRVs.end(); it++) {
		SAFE_RELEASE(*(it._Ptr));
	}
	// Clear the vector out
	m_pSpriteSRVs.clear();
	m_dicSpriteIDs.clear();
}

HRESULT Sprites::createInputLayout(ID3D11Device* device, ID3DX11EffectPass* pass)
{
	HRESULT hr;

	// Define the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS",		0, DXGI_FORMAT_R32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXINDEX",	0, DXGI_FORMAT_R32_SINT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PROGRESS",	0, DXGI_FORMAT_R32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ALPHA",		0, DXGI_FORMAT_R32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	D3DX11_PASS_DESC pd;

	// Create the input layout
	V_RETURN(pass->GetDesc(&pd));
	V_RETURN(device->CreateInputLayout(layout, numElements, pd.pIAInputSignature, 
		pd.IAInputSignatureSize, &m_pInputLayout));

	return S_OK;
}

void Sprites::destroyInputLayout()
{
	SAFE_RELEASE(m_pInputLayout);
}

void Sprites::renderSprites(ID3D11DeviceContext* context, ID3DX11EffectPass* pass,
							const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera)
{
	HRESULT hr;

	// Create update region
	D3D11_BOX box;
	box.left = 0; box.right = sprites.size() * sizeof(SpriteVertex);
	box.top = 0; box.bottom = 1;
	box.front = 0; box.back = 1;

	// Update the resource
	context->UpdateSubresource(m_pVertexBuffer, 0, &box, &sprites[0], 0, 0);

	// Bind the vertex buffer to the input assembler stage 
	ID3D11Buffer* vbs[] = { m_pVertexBuffer, };
	unsigned int strides[] = { sizeof(SpriteVertex), }, offsets[] = { 0, };
	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Tell the input assembler stage which primitive topology to use
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->IASetInputLayout(m_pInputLayout);

	// Update right and up vectors
	V(g_Effect2D.g_pCameraRight->SetFloatVector((float*)&(camera.GetWorldRight())));
	V(g_Effect2D.g_pCameraUp->SetFloatVector((float*)&(camera.GetWorldUp())));

	// Calculate view projection matrix and apply
	XMMATRIX mViewProj = camera.GetViewMatrix() * camera.GetProjMatrix();
	V(g_Effect2D.g_pViewProjectionMatrix->SetMatrix((float*)&mViewProj));

	// Set texture array
	V(g_Effect2D.g_pTextureArray2D->SetResourceArray(&m_pSpriteSRVs[0], 0, m_pSpriteSRVs.size()));

	// Apply the changes
	V(pass->Apply(0, context));

	// Draw all sprites
	context->Draw(sprites.size(), 0);
}

#pragma endregion

#pragma region Constructors & Destructors

Sprites::Sprites(): 
	m_pVertexBuffer(NULL),
	m_pSpriteSRVs(NULL)
{
}

Sprites::~Sprites()
{
}

#pragma endregion