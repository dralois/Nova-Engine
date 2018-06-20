//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

int         g_TextureCount;
Texture2D   g_SpriteTexture[10];

cbuffer cbChangesEveryFrame
{
    matrix g_ViewProjection;
    float4 g_CameraRight;
    float4 g_CameraUp;
};

//--------------------------------------------------------------------------------------
// Input structs
//--------------------------------------------------------------------------------------

struct SpriteVertex
{
    float3 Position : POSITION;
    float Radius : RADIUS;
    int TextureIndex : TEXINDEX;
};

struct PSVertex
{
    float4 Position : SV_POSITIOn;
    float2 TexCoords : TEXCOORD;
    int TextureIndex : TEXINDEX;
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

RasterizerState rsCullNone
{
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
    stream.Append(v);
    // Top left
    v.Position = mul(float4(curr.Position - curr.Radius * g_CameraRight.xyz + curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    v.TextureIndex = curr.TextureIndex;
    v.TexCoords = float2(0, 1);
    stream.Append(v);
    // Bottom right
    v.Position = mul(float4(curr.Position + curr.Radius * g_CameraRight.xyz - curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    v.TextureIndex = curr.TextureIndex;
    v.TexCoords = float2(1, 0);
    stream.Append(v);
    // Top right
    v.Position = mul(float4(curr.Position + curr.Radius * g_CameraRight.xyz + curr.Radius * g_CameraUp.xyz, 1.0f), g_ViewProjection);
    v.TextureIndex = curr.TextureIndex;
    v.TexCoords = float2(1, 1);
    stream.Append(v);
}

// Samples from texture
float4 SpritePS(PSVertex input) : SV_Target0
{
    switch (input.TextureIndex)
    {
        case 0:
            return g_SpriteTexture[0].Sample(samLinearClamp, input.TexCoords);
        case 1:
            return g_SpriteTexture[1].Sample(samLinearClamp, input.TexCoords);
        default:
            return float4(0, 0, 0, 1);
    }
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
        SetGeometryShader(CompileShader(gs_4_0, SpriteGS()));
        SetPixelShader(CompileShader(ps_4_0, SpritePS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}