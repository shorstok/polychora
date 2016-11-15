Texture2D srcTexture;
SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

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

#define FXAA_PC 1
#define FXAA_HLSL_4 1
#define FXAA_GREEN_AS_LUMA 1

#include "Fxaa3_11.fxh"

struct VertexShaderInput
{
    float4 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
};

struct VertexShaderOutput
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
    VertexShaderOutput output;

    output.Position = input.Position;
	output.Position.w = 1;

	output.TexCoord = input.TexCoord;

    return output;
}

cbuffer consts{
	float2 InverseViewportSize;
	float4 ConsoleSharpness;
	float4 ConsoleOpt1;
	float4 ConsoleOpt2;
	float SubPixelAliasingRemoval;
	float EdgeThreshold;
	float EdgeThresholdMin;
	float ConsoleEdgeSharpness;

	float ConsoleEdgeThreshold;
	float ConsoleEdgeThresholdMin;
}



// Must keep this as constant register instead of an immediate
float4 Console360ConstDir = float4(1.0, -1.0, 0.25, -0.25);

float4 PixelShaderFunction_FXAA(VertexShaderOutput inp) : SV_Target
{
	float4 theSample = srcTexture.Sample(samLinearClamp, inp.TexCoord);
	
	FxaaTex ft;

	ft.smpl = samLinearClamp;
	ft.tex = srcTexture;
	
	float4 value = FxaaPixelShader(
		inp.TexCoord,
		0,	// Not used in PC or Xbox 360
		ft,
		ft,			
		ft,			
		InverseViewportSize,	// FXAA Quality only
		ConsoleSharpness,		// Console only
		ConsoleOpt1,
		ConsoleOpt2,
		SubPixelAliasingRemoval,	// FXAA Quality only
		EdgeThreshold,// FXAA Quality only
		EdgeThresholdMin,
		ConsoleEdgeSharpness,
		ConsoleEdgeThreshold,	// TODO
		ConsoleEdgeThresholdMin, // TODO
		Console360ConstDir
		);

    return value;
}



technique10 FXAA
{
    pass Pass1
    {
	    SetVertexShader( CompileShader( vs_4_0, VertexShaderFunction() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PixelShaderFunction_FXAA() ) );

		SetBlendState(NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFff );
		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( DisableDepth, 0 );
    }
}
