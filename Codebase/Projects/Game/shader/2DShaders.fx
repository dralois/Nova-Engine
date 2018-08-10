//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

Texture2DArray  g_Textures[5];  // Texture array that represents an animation or sprite
TextureCube     g_Skybox;       // Texture cube that is the surrounding skybox

cbuffer cbChangesEveryFrame
{
    matrix g_ViewProjection;    // Only (camera) view and projection matrix
    matrix g_ViewProjectionInv; // Inverted view projection matrix
    float4 g_CameraRight;       // Right camera vector
    float4 g_CameraUp;          // Up camera vector
    float4 g_CameraPos;         // Camera (world) position
};

//--------------------------------------------------------------------------------------
// Input structs
//--------------------------------------------------------------------------------------

struct SpriteVertexVSIn
{
    float3 Position :   POSITION;
    float Radius :      RADIUS;
    int TextureIndex :  TEXINDEX;
    float Progress :    PROGRESS;
    float Alpha :       ALPHA;
};

struct SpriteVertexPSIn
{
    float4 Position :   SV_POSITION;
    float2 TexCoords :  TEXCOORD;
    int TextureIndex :  TEXINDEX;
    float Progress :    PROGRESS;
    float Alpha :       ALPHA;
};

struct SkyboxVertexPSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
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

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsCullNone
{
    CullMode = None;
};

//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

BlendState BSBlendOver
{
    BlendEnable[0]      = TRUE;
    SrcBlend[0]         = SRC_ALPHA;
    SrcBlendAlpha[0]    = ONE;
    DestBlend[0]        = INV_SRC_ALPHA;
    DestBlendAlpha[0]   = INV_SRC_ALPHA;
};

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

// Pass input through (point)
SpriteVertexVSIn SpriteVS(SpriteVertexVSIn input)
{
    return input;
}

// Creates billboard
[maxvertexcount(4)]
void SpriteGS(point SpriteVertexVSIn input[1], inout TriangleStream<SpriteVertexPSIn> stream)
{
    SpriteVertexPSIn output = (SpriteVertexPSIn) 0;
    SpriteVertexVSIn curr = input[0];
    // Bottom left
    output.Position = mul(float4(curr.Position - curr.Radius * g_CameraRight.xyz - curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    output.TextureIndex = curr.TextureIndex;
    output.TexCoords = float2(0, 0);
    output.Progress = curr.Progress;
    output.Alpha = curr.Alpha;
    stream.Append(output);
    // Top left
    output.Position = mul(float4(curr.Position - curr.Radius * g_CameraRight.xyz + curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    output.TextureIndex = curr.TextureIndex;
    output.TexCoords = float2(0, 1);
    output.Progress = curr.Progress;
    output.Alpha = curr.Alpha;
    stream.Append(output);
    // Bottom right
    output.Position = mul(float4(curr.Position + curr.Radius * g_CameraRight.xyz - curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    output.TextureIndex = curr.TextureIndex;
    output.TexCoords = float2(1, 0);
    output.Progress = curr.Progress;
    output.Alpha = curr.Alpha;
    stream.Append(output);
    // Top right
    output.Position = mul(float4(curr.Position + curr.Radius * g_CameraRight.xyz + curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    output.TextureIndex = curr.TextureIndex;
    output.TexCoords = float2(1, 1);
    output.Progress = curr.Progress;
    output.Alpha = curr.Alpha;
    stream.Append(output);
}

// Samples corresponding sprite texture
float4 SpritePS(SpriteVertexPSIn input) : SV_Target0
{
    float4 dims = (float4) 0;
    float4 col = (float4) 0;
    // Switch over texture arrays (select texture based in index and progress)
    switch (input.TextureIndex)
    {
        case 0:
            g_Textures[0].GetDimensions(dims.x, dims.y, dims.z);
            col = g_Textures[0].Sample(samAnisotropic, float3(input.TexCoords.xy, input.Progress * dims.z));
            break;
        case 1:
            g_Textures[1].GetDimensions(dims.x, dims.y, dims.z);
            col = g_Textures[1].Sample(samAnisotropic, float3(input.TexCoords.xy, input.Progress * dims.z));
            break;
        case 2:
            g_Textures[2].GetDimensions(dims.x, dims.y, dims.z);
            col = g_Textures[2].Sample(samAnisotropic, float3(input.TexCoords.xy, input.Progress * dims.z));
            break;
        case 3:
            g_Textures[3].GetDimensions(dims.x, dims.y, dims.z);
            col = g_Textures[3].Sample(samAnisotropic, float3(input.TexCoords.xy, input.Progress * dims.z));
            break;
        case 4:
            g_Textures[4].GetDimensions(dims.x, dims.y, dims.z);
            col = g_Textures[4].Sample(samAnisotropic, float3(input.TexCoords.xy, input.Progress * dims.z));
            break;
        default:
            break;
    }
    // Return with applied alpha
    return float4(col.rgb, col.a * input.Alpha);
}

// Pass through to geometry shader
float4 SkyboxVS(uint id : SV_VertexID) : POSITION
{
    return float4(0, 0, 0, 1);
}

// Creates quad at far clipping plane
[maxvertexcount(4)]
void SkyboxGS(point float4 input[1] : POSITION, inout TriangleStream<SkyboxVertexPSIn> stream)
{
    // Corners and corresponding texture coordinates are in NDC
    SkyboxVertexPSIn curr = (SkyboxVertexPSIn) 0;
    curr.Position = float4(-1, -1, 1, 1);
    curr.TexCoord = float2(0, 0);
    stream.Append(curr);
    curr.Position = float4(-1, 1, 1, 1);
    curr.TexCoord = float2(0, 1);
    stream.Append(curr);
    curr.Position = float4(1, -1, 1, 1);
    curr.TexCoord = float2(1, 0);
    stream.Append(curr);
    curr.Position = float4(1, 1, 1, 1);
    curr.TexCoord = float2(1, 1);
    stream.Append(curr);
}

// Samples from cubemap
float4 SkyboxPS(SkyboxVertexPSIn input) : SV_Target0
{
    // Calculate corners of the far plane quad in world space
    float4 topLeft = mul(float4(-1, 1, 1, 1), g_ViewProjectionInv);
    float4 topRight = mul(float4(1, 1, 1, 1), g_ViewProjectionInv);
    float4 bottomLeft = mul(float4(-1, -1, 1, 1), g_ViewProjectionInv);
    // Dehomogenise
    topLeft /= topLeft.w;
    topRight /= topRight.w;
    bottomLeft /= bottomLeft.w;
    // Calculate directional vectors
    float4 down = bottomLeft - topLeft;
    float4 right = topRight - topLeft;
    // Calculate direction in cubemap with texture coordinates
    float3 dir = normalize((topLeft + input.TexCoord.x * right - input.TexCoord.y * down) - g_CameraPos).xyz;
    // Sample color and return it
    return g_Skybox.Sample(samAnisotropic, dir);
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
    pass P0_Sprite
    {
        SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
        SetGeometryShader(CompileShader(gs_4_0, SpriteGS()));
        SetPixelShader(CompileShader(ps_4_0, SpritePS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
    pass P1_Skybox
    {
        SetVertexShader(CompileShader(vs_4_0, SkyboxVS()));
        SetGeometryShader(CompileShader(gs_4_0, SkyboxGS()));
        SetPixelShader(CompileShader(ps_4_0, SkyboxPS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}