//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Buffer<float> g_HeightMap; // Buffer for height values
Texture2D   g_Diffuse; // Material albedo for diffuse lighting
Texture2D	g_NormalMap; // Normalmap for lighting
Texture2D   g_Specular; // Specular texture
Texture2D   g_Glow; // Glow texture


//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
    float4  g_LightDir; // Object space
	int		g_TerrainRes; // Resolution
};

cbuffer cbChangesEveryFrame
{
    matrix  g_WorldViewProjection;
	matrix	g_WorldNormals;
    matrix  g_World;
    float4  g_CameraPos;
    float  g_Time;
};

cbuffer cbUserChanges
{
};


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct PosNorTex
{
    float4 Pos : SV_POSITION;
    float4 Nor : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PosTexLi
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float   Li : LIGHT_INTENSITY;
	float3 normal: NORMAL;
};

struct PosTex 
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

struct T3dVertexVSIn
{
    float3 Pos : POSITION; // Position in object space
    float2 Tex : TEXCOORD; // Texture coordinate
    float3 Nor : NORMAL; // Normal in object space
    float3 Tan : TANGENT; // Tangent in object space
};

struct T3dVertexPSIn
{
    float4 Pos : SV_POSITION; // Position in clip space
    float2 Tex : TEXCOORD; // Texture coordinate
    float3 PosWorld : POSITION; // Position in world space
    float3 NorWorld : NORMAL; // Normal in world space
    float3 TanWorld : TANGENT; // Tangent in world space
};

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsDefault {
};

RasterizerState rsCullFront {
    CullMode = Front;
};

RasterizerState rsCullBack {
    CullMode = Back;
};

RasterizerState rsCullNone {
	CullMode = None; 
};

RasterizerState rsLineAA {
	CullMode = None; 
	AntialiasedLineEnable = true;
};


//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};


//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

PosTex TerrainVS(uint vertexID : SV_VertexID)
{
    PosTex output = (PosTex) 0;
	// Calc xz so the center is at (0/0/0) (also set w part to 1)
    output.Pos.xzw = float3(((vertexID % g_TerrainRes) / (float) g_TerrainRes) - 0.5f,
                            ((vertexID / g_TerrainRes) / (float) g_TerrainRes) - 0.5f, 1.0f);
    // Fetch y from heightbuffer
    output.Pos.y = g_HeightMap[vertexID];
	// View transformation
    output.Pos = mul(output.Pos, g_WorldViewProjection);
	// Calc uv coords
    output.Tex.xy = float2((vertexID % g_TerrainRes) / (float) g_TerrainRes, 
                           (vertexID / g_TerrainRes) / (float) g_TerrainRes);
	// return
    return output;
}

float4 TerrainPS(PosTex Input) : SV_Target0
{
    float3 n;
    // Restore normal xz value (stored in xy in the normalmap)
    n.xz = (g_NormalMap.Sample(samAnisotropic, Input.Tex).xyz * 2.0f).xy - 1.0f;
    // Restore y component (always points up, so always positive)
    n.y = sqrt(1.0f - pow(n.x, 2.0f) - pow(n.z, 2.0f));
    // Project and normalize again
    n = normalize(mul(float4(n, 0), g_WorldNormals).xyz);
    // Sample texture at pos
    float3 matDiffuse = g_Diffuse.Sample(samLinearClamp, Input.Tex).xyz;
    // Calculate intensity (in range [0-1])
    float i = saturate(dot(n, g_LightDir.xyz));
    // Apply intensity to the color and return color
    return float4(matDiffuse * i, 1.0f);
}

// Mesh vertex shader
T3dVertexPSIn MeshVS(T3dVertexVSIn Input)
{
    T3dVertexPSIn output = (T3dVertexPSIn) 0;
    // Transform into worldspace
    output.Pos = mul(float4(Input.Pos, 1), g_WorldViewProjection);
    output.Tex = Input.Tex;
    // Transform world coordinates
    output.PosWorld = mul(float4(Input.Pos, 1), g_World).xyz;
    output.NorWorld = normalize(mul(float4(Input.Nor, 0), g_WorldNormals).xyz);
    output.TanWorld = normalize(mul(float4(Input.Tan, 0), g_World).xyz);
    // Return to pixel shader
    return output;
}

// Mesh pixel shader
float4 MeshPS(T3dVertexPSIn Input) : SV_Target0
{    
    // Weights for different terms
    float cd = 0.5f;
    float cs = 0.4f;
    float ca = 0.1f;
    float cg = 0.5f;
    // Get and store texure values at texcoord
    float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    float4 matSpecular = g_Specular.Sample(samAnisotropic, Input.Tex);
    float4 matGlow = g_Glow.Sample(samAnisotropic, Input.Tex);
    // Light and ambient color are white and not dynamic
    float4 colLight = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 colLightAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);
    // Calculate normal
    float3 n = normalize(Input.NorWorld);
    // Store light direction
    float3 l = g_LightDir.xyz;
    // Calculate reflection
    float3 r = reflect(-l, n);
    // Calculate view
    float3 v = normalize(g_CameraPos.xyz - Input.PosWorld);
    // Return the phong color
    return (cd * matDiffuse * saturate(dot(n, g_LightDir.xyz)) * colLight +
            cs * matSpecular * pow(saturate(dot(r, v)), 5) * colLight +
            ca * matDiffuse * colLightAmbient + 
            cg * matGlow);
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, TerrainPS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
    // Mesh pass
    pass P1_Mesh
    {
        SetVertexShader(CompileShader(vs_4_0, MeshVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, MeshPS()));
        
        SetRasterizerState(rsCullBack);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
