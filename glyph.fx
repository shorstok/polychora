Texture2D srcTexture;

float2 srcViewportSize;
float2 fontAtlasSize;

float4 color;

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct VS_INPUT
{
    float4 Pos : POSITION0;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};


BlendState SrcAlphaBlending
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = SRC_ALPHA;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};


///

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
};

RasterizerState SolidStateNoCull {
	FillMode = SOLID;
    CullMode  =NONE; 
};

BlendState NoBlending
{
    BlendEnable[0] = FALSE;
};

///


PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output;

    output.Pos = input.Pos;
	output.Pos.xy/=srcViewportSize.xy;
    output.Pos.w = 1;

    output.Tex = input.Tex/fontAtlasSize.xy;

	output.Pos.x=output.Pos.x*2-1;
	output.Pos.y=1-output.Pos.y*2;

    return output;
}


float4 PS( PS_INPUT input) : SV_Target
{
	return saturate(srcTexture.Sample( samLinear,input.Tex) * color);
}



technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
        
		SetBlendState(SrcAlphaBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFff );
		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( DisableDepth, 0 );
    }
}
