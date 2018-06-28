#include "SpriteRenderer.h"

using namespace DirectX;

// Convenience macros for safe effect variable retrieval
#define SAFE_GET_PASS(Technique, name, var)   {assert(Technique!=NULL); var = Technique->GetPassByName( name );						assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var) {assert(effect!=NULL); var = effect->GetTechniqueByName( name );						assert(var->IsValid());}
#define SAFE_GET_SCALAR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsScalar();			assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();			assert(var->IsValid());}
#define SAFE_GET_MATRIX(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();			assert(var->IsValid());}
#define SAFE_GET_SAMPLER(effect, name, var)   {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsSampler();			assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)  {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();	assert(var->IsValid());}

#pragma region Procedures

// Converts string to wchar_t pointer (use with caution, don't forget to delete)
wchar_t * strToWChar_t(std::string input) {
	wchar_t * output = new wchar_t[input.length() + 1];
	size_t out;
	mbstowcs_s(&out, output, input.length() + 1, input.c_str(), input.length());
	return output;
}

HRESULT SpriteRenderer::reloadShader(ID3D11Device * pDevice)
{
	HRESULT hr;
	WCHAR path[MAX_PATH];

	// Find and load the rendering effect
	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"shader\\SpriteRenderer.fxo"));
	std::ifstream is(path, std::ios_base::binary);
	is.seekg(0, std::ios_base::end);
	std::streampos pos = is.tellg();
	is.seekg(0, std::ios_base::beg);
	std::vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();

	// Create the effect
	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pDevice, &m_pEffect));
	assert(m_pEffect->IsValid());

	// Store the view projection matrix and vectors
	SAFE_GET_MATRIX(m_pEffect, "g_ViewProjection", m_pViewProjectionMatrix);
	SAFE_GET_RESOURCE(m_pEffect, "g_SpriteTexture", m_pSpriteTexture2D);
	SAFE_GET_VECTOR(m_pEffect, "g_CameraRight", m_pCameraRight);
	SAFE_GET_VECTOR(m_pEffect, "g_CameraUp", m_pCameraUp);

	return S_OK;
}

void SpriteRenderer::releaseShader()
{
	// Release the effect
	SAFE_RELEASE(m_pEffect);
}

HRESULT SpriteRenderer::create(ID3D11Device * pDevice)
{	
	HRESULT hr;

	// Buffer desc
	D3D11_BUFFER_DESC bd = { 0 };
	bd.MiscFlags = 0;
	bd.CPUAccessFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(SpriteVertex) * 1024;

	// Create vertex buffer
	V(pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer));

	// Define the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS",	     0, DXGI_FORMAT_R32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXINDEX",	 0, DXGI_FORMAT_R32_SINT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	D3DX11_PASS_DESC pd;
	ID3DX11EffectPass * pass0;
	ID3DX11EffectTechnique * technique;

	// Optain the pass
	SAFE_GET_TECHNIQUE(m_pEffect, "Render", technique);
	SAFE_GET_PASS(technique, "P0", pass0);

	// Create the input layout
	V_RETURN(pass0->GetDesc(&pd));
	V_RETURN(pDevice->CreateInputLayout(layout, numElements, pd.pIAInputSignature,
		pd.IAInputSignatureSize, &m_pInputLayout));

	// Create the textures
	for (auto it = m_textureFilenames.begin(); it != m_textureFilenames.end(); it++) {
		ID3D11ShaderResourceView * nextView;
		wchar_t * asWChar = strToWChar_t(it->data());
		V_RETURN(DirectX::CreateDDSTextureFromFile(pDevice, asWChar, nullptr, &nextView));
		m_pSpriteSRV.push_back(nextView);
		delete asWChar;
	}

	return S_OK;
}

void SpriteRenderer::destroy()
{
	// Release all resources
	SAFE_RELEASE(m_pSpriteTexture2D);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pEffect);
	for (auto it = m_pSpriteSRV.begin(); it != m_pSpriteSRV.end(); it++) {
		SAFE_RELEASE(*(it._Ptr));
	}
	// Clear the vector out
	m_pSpriteSRV.clear();
}

void SpriteRenderer::renderSprites(ID3D11DeviceContext * context, 
	const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera & camera)
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

	ID3DX11EffectPass * pass;
	ID3DX11EffectTechnique * technique;

	// Optain the pass
	SAFE_GET_TECHNIQUE(m_pEffect, "Render", technique);
	SAFE_GET_PASS(technique, "P0", pass);

	// Update right and up vectors
	V(m_pCameraRight->SetFloatVector((float*)&(camera.GetWorldRight())));
	V(m_pCameraUp->SetFloatVector((float*)&(camera.GetWorldUp())));

	// Calculate view projection matrix and apply
	XMMATRIX mViewProj = camera.GetViewMatrix() * camera.GetProjMatrix();
	V(m_pViewProjectionMatrix->SetMatrix((float*)&mViewProj));

	// Set texture array
	V(m_pSpriteTexture2D->SetResourceArray(&m_pSpriteSRV[0], 0, m_pSpriteSRV.size()));

	// Apply the changes
	V(pass->Apply(0, context));

	// Draw all sprites
	context->Draw(sprites.size(), 0);
}

#pragma endregion

#pragma region Constructors & Destructors

SpriteRenderer::SpriteRenderer(const std::vector<std::string>& textureFilenames) : 
	m_textureFilenames(textureFilenames),
	m_pVertexBuffer(nullptr),
	m_pInputLayout(nullptr),
	m_pEffect(nullptr),
	m_spriteCountMax(0)
{
}

SpriteRenderer::~SpriteRenderer()
{
}

#pragma endregion