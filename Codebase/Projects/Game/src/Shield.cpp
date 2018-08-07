#include "Shield.h"

#define PI 3.141592654F

using namespace std;
using namespace DirectX;

ID3D11InputLayout*	Shield::m_pInputLayout;
ID3D11ShaderResourceView* Shield::g_pDepthSRV;
ID3D11DepthStencilView* Shield::g_pDepthStencilView;

#pragma region Methods

HRESULT Shield::create(ID3D11Device * pDevice, int loopcount)
{
	HRESULT hr;

	// Loop needs to be power of 2 and at least 4
	loopcount = loopcount < 2 ? 4 : loopcount * 2;

	// Vector that contains the sphere vertices
	vector<ShieldVertexIn> l_arrSphere;

	// Zenith loop
	for (int i = 0;  i < loopcount; i++) {
		// Azimuth loop (first/last level is only one ring)
		for (int j = 0; j < loopcount; j += (i == 0 || i == (loopcount - 1)) ? 2 : 1) {
			ShieldVertexIn l_vPush;
			float x, y, z, u, v;	
			// Offsets depending on azimuth and zenith
			float off_top = (i == 0 ? 0 : (i == loopcount - 1 ? 2 * PI / loopcount : (j % 2 == 0 ? 0 : 2 * PI / loopcount)));
			float off_bot = (i == 0 ? 2 * PI / loopcount : (i == loopcount - 1 ? 0 : (j % 2 == 0 ? 2 * PI / loopcount : 0)));
			// Flips tri's around depending on level
			int add_i = (i == 0 ? 1 : (i == loopcount - 1 ? 0 : (j % 2 == 0 ? 1 : 0)));
			// Special cases at very bottom, also phase them each level
			int add_j = i == loopcount - 1 ? 0 : i % 2;
			// Top vertex / Top left vertex
			x = cosf((((float)(j + add_j) / loopcount) * (2.0f * PI)) - off_top) * sinf(((float)i / loopcount) * PI);
			z = sinf((((float)(j + add_j) / loopcount) * (2.0f * PI)) - off_top) * sinf(((float)i / loopcount) * PI);
			y= cosf(((float)i / loopcount) * PI);
			u = atan2f(z, x) / PI;
			v = acosf(y) / PI;
			l_vPush.Position = XMVectorSet(x, y, z, 1);
			l_vPush.TexCoord = XMFLOAT2(u, v);
			l_arrSphere.push_back(l_vPush);
			// Bottom right vertex / Top right vertex
			x = cosf((((float)(j + add_j) / loopcount) * (2.0f * PI)) + off_top + off_bot) * sinf(((float)(i + add_i) / loopcount) * PI);
			z = sinf((((float)(j + add_j) / loopcount) * (2.0f * PI) )+ off_top + off_bot) * sinf(((float)(i + add_i) / loopcount) * PI);
			y = cosf(((float)(i + add_i) / loopcount) * PI);
			u = atan2f(z, x) / PI;
			v = acosf(y) / PI;
			l_vPush.Position = XMVectorSet(x, y, z, 1);
			l_vPush.TexCoord = XMFLOAT2(u, v);
			l_arrSphere.push_back(l_vPush);
			// Bottom left vertex / Bottom vertex
			x = cosf((((float)(j + add_j) / loopcount) * (2.0f * PI)) - off_bot) * sinf(((float)(i + 1) / loopcount) * PI);
			z = sinf((((float)(j + add_j) / loopcount) * (2.0f * PI)) - off_bot) * sinf(((float)(i + 1) / loopcount) * PI);
			y = cosf(((float)(i + 1) / loopcount) * PI);
			u = atan2f(z, x) / PI;
			v = acosf(y) / PI;
			l_vPush.Position = XMVectorSet(x, y, z, 1);
			l_vPush.TexCoord = XMFLOAT2(u, v);
			l_arrSphere.push_back(l_vPush);
		}
	}

	// Data description
	D3D11_SUBRESOURCE_DATA id = { 0 };
	id.pSysMem = &l_arrSphere[0];
	id.SysMemPitch = sizeof(ShieldVertexIn);
	id.SysMemSlicePitch = 0;
	
	// 
	D3D11_BUFFER_DESC bd = { 0 };
	bd.MiscFlags = 0;
	bd.CPUAccessFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(ShieldVertexIn) * l_arrSphere.size();

	// Create vertex buffer
	V(pDevice->CreateBuffer(&bd, &id, &m_pVertexBuffer));

	// Save the size
	m_iVertexCount = l_arrSphere.size();

	// Create texture
	V(DirectX::CreateDDSTextureFromFile(pDevice, L"resources\\wasser12.DDS", (ID3D11Resource**) &m_pShieldTex2D, &m_pShieldTexSRV));

	return S_OK;
}

void Shield::destroy()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pShieldTex2D);
	SAFE_RELEASE(m_pShieldTexSRV);
}

HRESULT Shield::createDepthBuffer(ID3D11Device * pDevice, UINT width, UINT height)
{
	HRESULT hr;
	
	// Setup depth stencil texture description
	ID3D11Texture2D* pDepthTex2D;
	D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
	depthTexDesc.Width = width;
	depthTexDesc.Height = height;
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	// Stencil view description
	D3D11_DEPTH_STENCIL_VIEW_DESC depthSVDesc;
	ZeroMemory(&depthSVDesc, sizeof(depthSVDesc));
	depthSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthSVDesc.Texture2D.MipSlice = 0;	

	// SRV description
	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
	ZeroMemory(&depthSRVDesc, sizeof(depthSRVDesc));
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MipLevels = 1;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;

	// Create the texture
	V(pDevice->CreateTexture2D(&depthTexDesc, NULL, &pDepthTex2D));

	// Create depth stencil view
	V(pDevice->CreateDepthStencilView(pDepthTex2D, &depthSVDesc, &g_pDepthStencilView));

	// Create corresponding SRV
	V(pDevice->CreateShaderResourceView(pDepthTex2D, &depthSRVDesc, &g_pDepthSRV));

	// Release the used texture
	SAFE_RELEASE(pDepthTex2D);

	return S_OK;
}

void Shield::destroyDepthBuffer()
{
	SAFE_RELEASE(g_pDepthSRV);
	SAFE_RELEASE(g_pDepthStencilView);
}

HRESULT Shield::createInputLayout(ID3D11Device * device, ID3DX11EffectPass * pass)
{
	HRESULT hr;

	// Define the layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// Obtain description
	D3DX11_PASS_DESC pd;	
	V_RETURN(pass->GetDesc(&pd));

	// Apply input layout
	V_RETURN(device->CreateInputLayout(layout, numElements, 
		pd.pIAInputSignature, pd.IAInputSignatureSize, &m_pInputLayout));

	return S_OK;
}

void Shield::destroyInputLayout()
{
	SAFE_RELEASE(m_pInputLayout)
}

HRESULT Shield::render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass,
	ID3DX11EffectShaderResourceVariable* diffuseTex)
{
	HRESULT hr;

	// Bind the terrain vertex buffer to the input assembler stage 
	ID3D11Buffer* vbs[] = { m_pVertexBuffer, };
	unsigned int strides[] = { sizeof(ShieldVertexIn), }, offsets[] = { 0, };
	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Tell the input assembler stage which primitive topology to use
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_pInputLayout);

	// Set texture
	V(diffuseTex->SetResource(m_pShieldTexSRV));

	// Apply the pass
	V(pass->Apply(0, context));

	// Draw the shield
	context->Draw(m_iVertexCount, 0);

	return S_OK;
}

#pragma endregion

#pragma region Constructors & Destructors

Shield::Shield() :
	m_pVertexBuffer(NULL),
	m_pShieldTex2D(NULL),
	m_pShieldTexSRV(NULL),
	m_iVertexCount(0)
{
}

Shield::~Shield()
{
}

#pragma endregion