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
	diffuseTexture(nullptr),
	diffuseTextureSRV(nullptr),
	normalTexture(nullptr),
	normalTextureSRV(nullptr),
	heightBuffer(nullptr),
	heightBufferSRV(nullptr)
{
}

Terrain::~Terrain(void)
{
}

// Converts string to wchar_t pointer (use with caution, don't forget to delete)
wchar_t * strtowchar_t(string input) {
	wchar_t * output = new wchar_t[input.length() + 1];
	size_t out;
	mbstowcs_s(&out, output, input.length() + 1, input.c_str(), input.length());
	return output;
}

float Terrain::GetHeightAtXY(float pi_dX, float pi_dY)
{
	return dHeightfield[IDX((int) (iResolution-1) * pi_dX, (int) (iResolution-1) * pi_dY, iResolution)];
}

HRESULT Terrain::create(ID3D11Device* device, ConfigParser parser)
{
	HRESULT hr;

	// Heightfield fetch
	GEDUtils::SimpleImage heightfield(parser.GetTerrainPath().Height.insert(0,"resources\\").c_str());

	// Easy access
	iResolution = heightfield.getWidth();
	dHeightfield = vector<float> (iResolution * iResolution);
	
	// Fill heightfield
	for (int x = 0; x < iResolution; x++) {
		for (int y = 0; y < iResolution; y++) {
			dHeightfield[IDX(x, y, iResolution)] = heightfield.getPixel(x, y);
		}
	}

	// Create index buffer
	vector<int> indices;
	// Loop through quads and fill with appropriate indeces
	for (int x = 0; x < iResolution - 1; x++) {
		for (int y = 0; y < iResolution - 1; y++) {
			// First triangle
			indices.push_back((iResolution * y) + x);
			indices.push_back((iResolution * y) + x + 1);
			indices.push_back((iResolution * (y + 1)) + x);
			// Second triangle
			indices.push_back((iResolution * (y + 1)) + x);
			indices.push_back((iResolution * y) + x + 1);
			indices.push_back((iResolution * (y + 1)) + x + 1);
		}
	}

	// Data information
	D3D11_SUBRESOURCE_DATA id;
	id.pSysMem = &indices[0];

	// Buffer desc
	D3D11_BUFFER_DESC bd;
	bd.MiscFlags = 0;
	bd.CPUAccessFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.ByteWidth = sizeof(unsigned int) * indices.size();

	// Create index buffer
	V(device->CreateBuffer(&bd, &id, &indexBuffer));

	// Clear buffer desc and adjust
	ZeroMemory(&bd, sizeof(bd));
	bd.MiscFlags = 0;
	bd.CPUAccessFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.ByteWidth = sizeof(float) * dHeightfield.size();

	// Clear data information and adjust
	ZeroMemory(&id, sizeof(id));
	id.pSysMem = &dHeightfield[0];
	id.SysMemPitch = sizeof(float);
	id.SysMemSlicePitch = 0;

	// Create heightmap buffer
	V(device->CreateBuffer(&bd, &id, &heightBuffer));

	// Create description
	D3D11_BUFFER_SRV buffer;
	buffer.FirstElement = 0;
	buffer.NumElements = iResolution * iResolution;
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	desc.Buffer = buffer;

	// Create SRV for the heightmap buffer
	device->CreateShaderResourceView(heightBuffer, &desc, &heightBufferSRV);

	// Convert strings
	wchar_t * color = strtowchar_t(parser.GetTerrainPath().Color.insert(0, "resources\\"));
	wchar_t * normal = strtowchar_t(parser.GetTerrainPath().Normal.insert(0, "resources\\"));

	// Load color texture (color for terrain)
	V(DirectX::CreateDDSTextureFromFile(device, color, nullptr, &diffuseTextureSRV));
	
	// Load normal texture (normals for terrain)
	V(DirectX::CreateDDSTextureFromFile(device, normal, nullptr, &normalTextureSRV));

	// Cleanup
	delete[] color;
	delete[] normal;

	// Error handling
	if (hr != S_OK) {
		MessageBoxA(NULL, "Could not load either normal or color texture", "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}

	return hr;
}

void Terrain::destroy()
{
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(diffuseTexture);
	SAFE_RELEASE(diffuseTextureSRV);
	SAFE_RELEASE(normalTexture);
	SAFE_RELEASE(normalTextureSRV);
	SAFE_RELEASE(heightBuffer);
	SAFE_RELEASE(heightBufferSRV);
}

void Terrain::render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass)
{
	HRESULT hr;

	// Bind the terrain vertex buffer to the input assembler stage 
    ID3D11Buffer* vbs[] = { nullptr, };
    unsigned int strides[] = { 0, }, offsets[] = { 0, };
    context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Bind the index buffer to the input assembler stage
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Tell the input assembler stage which primitive topology to use
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);    

	// Bind the diffuse texture of the terrain
	V(g_gameEffect.diffuseEV->SetResource(diffuseTextureSRV));

	// Bind the normalmap
	V(g_gameEffect.normalmap->SetResource(normalTextureSRV));

	// Bind the heightmap
	V(g_gameEffect.heightmap->SetResource(heightBufferSRV));

	// Set the resolution
	V(g_gameEffect.resolution->SetInt(iResolution));

    // Apply the rendering pass in order to submit the necessary render state changes to the device
    V(pass->Apply(0, context));

	// Fetch index buffer desc
	D3D11_BUFFER_DESC bd;
	indexBuffer->GetDesc(&bd);
	
    // Draw (no. of indices must be known..)
    context->DrawIndexed(bd.ByteWidth / sizeof(unsigned int), 0, 0);	
}
