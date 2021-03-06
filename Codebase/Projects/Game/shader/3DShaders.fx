//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Buffer<float>   g_HeightMap;    // Buffer for height values
Texture2D       g_Diffuse;      // Diffuse albedo color texture
Texture2D       g_Specular;     // Specular reflection texture
Texture2D       g_Glow;         // Glow emission texture
Texture2D       g_Normal;       // Normalmap for lighting
Texture2D       g_Transparency; // Transparency texture
Texture2D       g_Depth;        // Depth buffer texture

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
    float4  g_LightDir;     // Direction in world space
    float   g_FarPlaneDist; // Far plane distance as float
	int		g_TerrainRes;   // Resolution
};

cbuffer cbChangesEveryFrame
{
    matrix  g_WorldViewProjection;
	matrix	g_WorldNormals;
    matrix  g_World;
    float4  g_CameraPos;
    float3  g_Hits[10];
};

//--------------------------------------------------------------------------------------
// Input / output layout structs
//--------------------------------------------------------------------------------------

struct TerrainVertexPSIn
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

struct ShieldVertexVSIn 
{
	float4 Pos : POSITION;  // Position in world space
	float2 Tex : TEXCOORD;  // Texture coordinate
};

struct ShieldVertexPSIn
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float Depth : DEPTH;
    float3 NorWorld : NORMAL;
    float3 PosWorld : POSITION;
};

struct T3dVertexVSIn
{
    float3 Pos : POSITION;  // Position in object space
    float2 Tex : TEXCOORD;  // Texture coordinate
    float3 Nor : NORMAL;    // Normal in object space
    float3 Tan : TANGENT;   // Tangent in object space
};

struct T3dVertexPSIn
{
    float4 Pos : SV_POSITION;   // Position in clip space
    float2 Tex : TEXCOORD;      // Texture coordinate
    float3 PosWorld : POSITION; // Position in world space
    float3 NorWorld : NORMAL;   // Normal in world space
    float3 TanWorld : TANGENT;  // Tangent in world space
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

RasterizerState rsCullFront {
    CullMode = Front;
};

RasterizerState rsCullBack {
    CullMode = Back;
};

RasterizerState rsCullNone {
	CullMode = None; 
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

// Don't write to w-buffer but test against it
DepthStencilState ShieldDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
    DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

BlendState BSBlendOver
{
    BlendEnable[0] = TRUE;
    SrcBlend[0] = SRC_ALPHA;
    SrcBlendAlpha[0] = ONE;
    DestBlend[0] = INV_SRC_ALPHA;
    DestBlendAlpha[0] = INV_SRC_ALPHA;
};

// Simple additive blending
BlendState BSBlendShield
{
    BlendEnable[0] = TRUE;
    SrcBlend[0] = SRC_ALPHA;
    SrcBlendAlpha[0] = SRC_ALPHA;
    DestBlend[0] = ONE;
    DestBlendAlpha[0] = ONE;
};

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

TerrainVertexPSIn TerrainVS(uint vertexID : SV_VertexID)
{
    TerrainVertexPSIn output = (TerrainVertexPSIn) 0;
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

float4 TerrainPS(TerrainVertexPSIn Input) : SV_Target0
{
    float3 n;
    // Restore normal xz value (stored in xy in the normalmap)
    n.xz = (g_Normal.Sample(samAnisotropic, Input.Tex).xy * 2.0f).xy - 1.0f;
    // Restore y component (always points up, so always positive)
    n.y = sqrt(1.0f - pow(n.x, 2.0f) - pow(n.z, 2.0f));
    // Project and normalize again
    n = normalize(mul(float4(n, 0), g_WorldNormals).xyz);
    // Sample texture at pos
    float3 matDiffuse = g_Diffuse.Sample(samLinearClamp, Input.Tex).xyz;
    // Simple N*L lighting
    float i = saturate(dot(n, g_LightDir.xyz));
    // Return color based on calculated lighting
    return float4(matDiffuse * i, 1.0f);
}

// Mesh vertex shader
T3dVertexPSIn MeshVS(T3dVertexVSIn Input)
{
    T3dVertexPSIn output = (T3dVertexPSIn) 0;
    // Transform into viewspace
    output.Pos = mul(float4(Input.Pos, 1), g_WorldViewProjection);
    output.Tex = Input.Tex;
    // Transform position into worldspace
    output.PosWorld = mul(float4(Input.Pos, 1), g_World).xyz;
    // Transform normal and tangent into worldspace
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
    float matTrans = 1 - g_Transparency.Sample(samAnisotropic, Input.Tex);
    // Light and ambient color are white and not dynamic
    float4 colLight = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 colLightAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);  
    // Calculate normal
    float3 nor = normalize(Input.NorWorld);
    // Calculate tangent
    float3 tan = normalize(Input.TanWorld - (nor * dot(nor, Input.TanWorld)));
    // Calculate bitangent
    float3 biTan = cross(tan, nor);
    // Store in transformation matrix
    float3x3 TBN = float3x3(tan, biTan, nor);
    // Load and calculate normal
    float3 normal;
    // Restore normal xz value (stored in xy in the normalmap)
    normal.xz = (g_Normal.Sample(samAnisotropic, Input.Tex).xy * 2.0f).xy - 1.0f;
    // Restore y component (always points up, so always positive)
    normal.y = sqrt(1.0f - pow(normal.x, 2.0f) - pow(normal.z, 2.0f));
    // Transform normal and renormalize
    normal = normalize(mul(TBN, normal));
    // Store light direction
    float3 lightDir = g_LightDir.xyz;
    // Calculate reflection
    float3 reflectDir = reflect(-lightDir, normal);
    // Calculate view
    float3 viewDir = normalize(g_CameraPos.xyz - Input.PosWorld);
    // Return the phong color
    return float4((cd * matDiffuse * saturate(dot(normal, g_LightDir.xyz)) * colLight +
                    cs * matSpecular * pow(saturate(dot(reflectDir, viewDir)), 5) * colLight +
                    ca * matDiffuse * colLightAmbient +
                    cg * matGlow).rgb, matTrans);
}

// Shield vertex shader
ShieldVertexPSIn ShieldVS(ShieldVertexVSIn Input)
{
    ShieldVertexPSIn output = (ShieldVertexPSIn) 0;
    // Calculate the position in regards to shield radius
    output.Pos = mul(Input.Pos, g_WorldViewProjection);
    // Save texcoords
    output.Tex = Input.Tex;
    // Calculate depth
    output.Depth = output.Pos.z / output.Pos.w;
    // Normal is the normalized vertex position (transformed)
    output.NorWorld = normalize(mul(float4(Input.Pos.xyz, 0), g_WorldNormals).xyz);
    // Calculate world position as well
    output.PosWorld = mul(Input.Pos, g_World).xyz;
    // Pass to vertex shader
    return output;
}

// Shield pixel shader
float4 ShieldPS(ShieldVertexPSIn Input) : SV_Target0
{
    // Load depth from texture
    float currDepth = g_Depth.Load(int3(Input.Pos.xy, 0)).r;
    // Calculate difference
    float depthDiff = currDepth - Input.Depth;
    float intersect = 0;
    // If difference is positive
    if (depthDiff > 0) 
    {
        // Power of intersection terrain / shield
        intersect = 1 - smoothstep(0, (1.0f / g_FarPlaneDist), depthDiff);
    }
    // Calculate view direction
    float3 viewDir = normalize(g_CameraPos.xyz - Input.PosWorld);
    // Calculate rim power (direction * normal = 0 if on the rim)
    float rim = 1 - abs(dot(Input.NorWorld, viewDir));
    // Initialize hit var
    float hit = 0;
    // For all current hits
    for (int i = 0; i < 10; i++)
    {
        // If hit is valid
        if (length(g_Hits[i]) > 0)
        {
            // The closer to a hit the brighter (distance in world coordinates)
            hit = max(hit, pow(1 - smoothstep(0, 100, distance(g_Hits[i], Input.PosWorld)), 10));
        }
    }
    // Select max of hit and rim
    rim = max(rim, hit);
    // Select max of intersect and rim/hit
    float glow = max(intersect, rim);
    // Sample shield material
    float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    // Return smoothed blue outline
    return lerp(matDiffuse * float4(0, 0, 0, 0), matDiffuse * float4(0.5, 0.5, 1, 1), pow(glow, 2));
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
    // Terrain pass
    pass P0_Terrain
    {
        SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, TerrainPS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
    // Depth generation only
    pass P0_Depth
    {
        SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(NULL);
        
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
        SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
    // Shield pass
    pass P2_Shield
    {
        SetVertexShader(CompileShader(vs_4_0, ShieldVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, ShieldPS()));

        SetRasterizerState(rsCullNone);
        SetDepthStencilState(ShieldDepth, 0);
        SetBlendState(BSBlendShield, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
