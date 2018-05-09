#include "Terrain.h"

#include "GameEffect.h"
#include "SimpleImage.h"
#include <DDSTextureLoader.h>
#include "DirectXTex.h"

#include "debug.h"

// You can use this macro to access your height field
#define IDX(X,Y,WIDTH) ((X) + (Y) * (WIDTH))

using namespace DirectX;

Terrain::Terrain(void):
	indexBuffer(nullptr),
	vertexBuffer(nullptr),
	diffuseTexture(nullptr),
	diffuseTextureSRV(nullptr),
	debugSRV(nullptr)
{
}


Terrain::~Terrain(void)
{
}

HRESULT Terrain::create(ID3D11Device* device, ConfigParser parser)
{
	HRESULT hr;

	// In our example, we load a debug texture
    V(DirectX::CreateDDSTextureFromFile(device, L"resources\\debug_green.dds", nullptr, &debugSRV));

	if (hr != S_OK) {
        MessageBoxA (NULL, "Could not load texture \"resources\\debug_green.dds\"", "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}

	// Load heightfield from path
	GEDUtils::SimpleImage heightfield(parser.GetTerrainPath().Height.c_str());

	// This buffer contains positions, normals and texture coordinates for one triangle
	vector<SimpleVertex> triangle = vector<SimpleVertex>(heightfield.getWidth() * heightfield.getHeight());

	// Easy access
	int height = heightfield.getHeight();
	int width = heightfield.getWidth();
	// For normal scaling
	XMMATRIX matNormalScaling = XMMatrixScaling(parser.GetTerrainWidth(),
												parser.GetTerrainHeight(),
												parser.GetTerrainDepth());
	matNormalScaling = XMMatrixTranspose(XMMatrixInverse(nullptr, matNormalScaling));
	// For each vertex
	for (UINT x = 0; x < width; x++) {
		for (UINT y = 0; y < height; y++) {
			SimpleVertex vert;
			// Determine texture coords
			vert.UV = XMFLOAT2((float) x / (width - 1.0F), (float) y / (height - 1.0F));
			// Determine position
			vert.Pos.x = ((x - (width / 2.0F)) / width) * parser.GetTerrainWidth();
			vert.Pos.y = heightfield.getPixel(x, y) * parser.GetTerrainHeight();
			vert.Pos.z = ((y - (height / 2.0F)) / height) * parser.GetTerrainDepth();
			vert.Pos.w = 1;
			// Determine normal
			float l_vTU = (heightfield.getPixel(x < width - 1 ? x + 1 : x, y) -
				heightfield.getPixel(x > 0 ? x - 1 : x, y)) / 2.0F * height;
			float l_vTV = (heightfield.getPixel(x, y < height - 1 ? y + 1 : y) -
				heightfield.getPixel(x, y > 0 ? y - 1 : y)) / 2.0F * width;
			// Create vector
			XMVECTOR vNormal = XMVectorSet(-l_vTU, -l_vTV, 1, 0);
			// Scale vector
			vNormal = XMVector4Transform(vNormal, matNormalScaling);
			// Normalize
			vNormal = XMVector3Normalize(vNormal);
			// Store normal
			XMStoreFloat4(&vert.Normal, vNormal);
			// Bring into [0-1]
			vert.Normal.x = (vert.Normal.x + 1.0F) / 2.0F;
			vert.Normal.y = (vert.Normal.y + 1.0F) / 2.0F;
			vert.Normal.z = (vert.Normal.z + 1.0F) / 2.0F;
			// Store vertex
			triangle[IDX(x, y, heightfield.getHeight())] = vert;
		}
	}

    D3D11_SUBRESOURCE_DATA id;
    id.pSysMem = &triangle[0];
    id.SysMemPitch = 10 * sizeof(float); // Stride
    id.SysMemSlicePitch = 0;

    D3D11_BUFFER_DESC bd;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(float) * triangle.size(); //The size in bytes of the triangle array
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;

    V(device->CreateBuffer(&bd, &id, &vertexBuffer)); // http://msdn.microsoft.com/en-us/library/ff476899%28v=vs.85%29.aspx

	// Create index buffer
	vector<int> indices;
	// Loop through "squares" and fill with appropriate index
	for (int i = 0; i < width - 1; i++) {
		for (int j = 0; j < height - 1; j++) {
			// First triangle
			indices.push_back((width * j) + i);
			indices.push_back((width * j) + i + 1);
			indices.push_back((width * (j + 1)) + i);
			// Second triangle
			indices.push_back((width * (j + 1)) + i);
			indices.push_back((width * j) + i + 1);
			indices.push_back((width * (j + 1)) + i + 1);
		}
	}

	// Clear buffer desc and adjust
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(unsigned int) * indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	// Clear data information and adjust
	ZeroMemory(&id, sizeof(id));
	id.pSysMem = &indices[0];

	V(device->CreateBuffer(&bd, &id, &indexBuffer));

	// Load color texture (color map)
	// TODO: Insert your code to load the color texture and create
	// the texture "diffuseTexture" as well as the shader resource view
	// "diffuseTextureSRV"

	return hr;
}


void Terrain::destroy()
{
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(debugSRV);

    // TODO: Release the terrain's shader resource view and texture
}


void Terrain::render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass)
{
	HRESULT hr;

	// Bind the terrain vertex buffer to the input assembler stage 
    ID3D11Buffer* vbs[] = { vertexBuffer, };
    unsigned int strides[] = { 10 * sizeof(float), }, offsets[] = { 0, };
    context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	// Bind the index buffer to the input assembler stage
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Tell the input assembler stage which primitive topology to use
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);    

    // TODO: Bind the SRV of the terrain diffuse texture to the effect variable
    // (instead of the SRV of the debug texture)
	V(g_gameEffect.diffuseEV->SetResource( debugSRV));

    // Apply the rendering pass in order to submit the necessary render state changes to the device
    V(pass->Apply(0, context));

    // Draw
    context->DrawIndexed(6, 0, 0);
}
