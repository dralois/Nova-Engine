//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

Texture2DArray g_Textures[5];  // Texture array that represents an animation or sprite

cbuffer cbChangesEveryFrame
{
    matrix g_ViewProjection;    // Only camera and projection matrix
    float4 g_CameraRight;       // Right vector
    float4 g_CameraUp;          // Up vector
};

//--------------------------------------------------------------------------------------
// Input structs
//--------------------------------------------------------------------------------------

struct SpriteVertex
{
    float3 Position :   POSITION;
    float Radius :      RADIUS;
    int TextureIndex :  TEXINDEX;
    float Progress :    PROGRESS;
    float Alpha :       ALPHA;
};

struct PSVertex
{
    float4 Position :   SV_POSITION;
    float2 TexCoords :  TEXCOORD;
    int TextureIndex :  TEXINDEX;
    float Progress :    PROGRESS;
    float Alpha :       ALPHA;
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

// Pass input through
SpriteVertex SpriteVS(SpriteVertex input)
{
    return input;
}

// Creates billboards
[maxvertexcount(4)]
void SpriteGS(point SpriteVertex input[1], inout TriangleStream<PSVertex> stream)
{
    PSVertex v = (PSVertex) 0;
    SpriteVertex curr = input[0];
    // Bottom left
    v.Position = mul(float4(curr.Position - curr.Radius * g_CameraRight.xyz - curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    v.TextureIndex = curr.TextureIndex;
    v.TexCoords = float2(0, 0);
    v.Progress = curr.Progress;
    v.Alpha = curr.Alpha;
    stream.Append(v);
    // Top left
    v.Position = mul(float4(curr.Position - curr.Radius * g_CameraRight.xyz + curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    v.TextureIndex = curr.TextureIndex;
    v.TexCoords = float2(0, 1);
    v.Progress = curr.Progress;
    v.Alpha = curr.Alpha;
    stream.Append(v);
    // Bottom right
    v.Position = mul(float4(curr.Position + curr.Radius * g_CameraRight.xyz - curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    v.TextureIndex = curr.TextureIndex;
    v.TexCoords = float2(1, 0);
    v.Progress = curr.Progress;
    v.Alpha = curr.Alpha;
    stream.Append(v);
    // Top right
    v.Position = mul(float4(curr.Position + curr.Radius * g_CameraRight.xyz + curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    v.TextureIndex = curr.TextureIndex;
    v.TexCoords = float2(1, 1);
    v.Progress = curr.Progress;
    v.Alpha = curr.Alpha;
    stream.Append(v);
}

// Samples from texture
float4 SpritePS(PSVertex input) : SV_Target0
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
        SetVertexShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(NULL);
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}