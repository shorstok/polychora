
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D srcTexture;
TextureCube SkyboxTexture; 

cbuffer rare
{
	float brighnessTol;
	float flareBrighnessTol;
}

cbuffer often
{
	float2 TexSize;
	bool bHorizontal;
	matrix gWorldViewProjInv;
}



SamplerState samAniClamp
{
    Filter = ANISOTROPIC;
    AddressU = Clamp;
    AddressV = Clamp;
};


SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};


SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
};


DepthStencilState DisableDepthWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
	DepthFunc = Less_Equal;
};

RasterizerState SolidStateNoCull {
	FillMode = SOLID;
    CullMode  =NONE; 
};

BlendState NoBlending
{
    BlendEnable[0] = FALSE;
};

BlendState SrcAlphaBlendingAdd
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
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

struct PS_ENV_INPUT
{
    float4 Pos : SV_POSITION;
    float4 PosL : TEXCOORD0;
};

PS_INPUT VS( VS_INPUT input )
{
	PS_INPUT output;

	output.Pos = input.Pos;
	output.Pos.w = 1;

	output.Tex = input.Tex;

	return output;
}


#define KernelSize 21


#if KernelSize == 7

float2 PixelKernel[KernelSize] =
{
	{ -3, 0 },
	{ -2, 0 },
	{ -1, 0 },
	{ 0, 0 },
	{ 1, 0 },
	{ 2, 0 },
	{ 3, 0 },
};

const float BlurWeights[KernelSize] = 
{
	0.064759,
	0.120985,
	0.176033,
	0.199471,
	0.176033,
	0.120985,
	0.064759,
};

#endif

#if KernelSize == 21

const float BlurWeights[KernelSize] = 
{
	0.009167927656011385,
	0.014053461291849008,
	0.020595286319257878,
	0.028855245532226279,
	0.038650411513543079,
	0.049494378859311142,
	0.060594058578763078,
	0.070921288047096992,
	0.079358891804948081,
	0.084895951965930902,
	0.086826196862124602,
	0.084895951965930902,
	0.079358891804948081,
	0.070921288047096992,
	0.060594058578763078,
	0.049494378859311142,
	0.038650411513543079,
	0.028855245532226279,
	0.020595286319257878,
	0.014053461291849008,
	0.009167927656011385
};

const float2 PixelKernel[KernelSize] =
{
	{ -10, 0 },
	{ -9, 0 },
	{ -8, 0 },
	{ -7, 0 },
	{ -6, 0 },
	{ -5, 0 },
	{ -4, 0 },
	{ -3, 0 },
	{ -2, 0 },
	{ -1, 0 },
	{ 0, 0 },
	{ 1, 0 },
	{ 2, 0 },
	{ 3, 0 },
	{ 4, 0 },
	{ 5, 0 },
	{ 6, 0 },
	{ 7, 0 },
	{ 8, 0 },
	{ 9, 0 },
	{ 10, 0 },
};


#endif


float4 PS_Bloom( PS_INPUT input) : SV_Target
{
	float4 colour = 0;

	if(bHorizontal)
	{
		[unroll]
		for (int p = 0; p < KernelSize; p++)
			colour += srcTexture.Sample( samLinearClamp, input.Tex + (PixelKernel[p].xy / TexSize)) * BlurWeights[p];
	}
	else
	{
		[unroll]
		for (int p = 0; p < KernelSize; p++)
			colour += srcTexture.Sample( samLinearClamp, input.Tex+ (PixelKernel[p].yx / TexSize)) * BlurWeights[p];
	}

	colour.a = 1;

	return colour;
}


float4 PS_FilteredQuad( PS_INPUT input) : SV_Target
{
	float4 tx = 0;
		
	tx=srcTexture.Sample(samLinearClamp,input.Tex);

	float l = tx.r*0.299+tx.g*0.587+tx.b*0.114;

	return l > brighnessTol ? tx : 0;
}




float4 PS_Copy( PS_INPUT input) : SV_Target
{
	return srcTexture.Sample(samLinearClamp,input.Tex);
}

//Wide box filter used to avoid bloom 'flicker' on thin lines

#define boxsize 9

const float2 box[boxsize]={
	{0,0},
	{3,0},
	{-3,0},
	{0,3},
	{0,-3},

	{3,3},
	{-3,3},
	{3,-3},
	{-3,-3},
};


float4 PS_CopySoft( PS_INPUT input) : SV_Target
{
	float4 tx = 0;

	for(int c=0;c!=boxsize;++c)
		tx+=srcTexture.Sample(samLinearClamp,input.Tex + box[c]/TexSize);

	tx/=boxsize;

	return tx;
}

PS_ENV_INPUT VS_Env( VS_INPUT input )
{
	PS_ENV_INPUT output;

	input.Pos.w = 1;

	output.Pos = input.Pos.xyww;

	output.PosL = mul(output.Pos,gWorldViewProjInv);

	return output;
}


float4 PS_Env( PS_ENV_INPUT input) : SV_Target
{
	//return float4(1,0,0,1);

	return SkyboxTexture.Sample(samLinearClamp,input.PosL);
}


technique10 CopyScreenQuadNoFilter
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS_Copy() ) );
        
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( DisableDepth, 0 );
    }
}


technique10 CopyScreenQuadSoftened
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS_CopySoft() ) );
        
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( DisableDepth, 0 );
    }
}


technique10 CopyScreenQuad
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS_FilteredQuad() ) );
        
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( DisableDepth, 0 );
    }
}


technique10 Blur
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS_Bloom() ) );
        
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( DisableDepth, 0 );
    }
}




technique10 AddFullScreenQuad
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS_Copy() ) );
        
		SetBlendState(SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( DisableDepth, 0 );
    }
}

technique10 RenderBackground
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Env() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS_Env() ) );
        
		SetBlendState(NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFff );
		SetRasterizerState(SolidStateNoCull);
		//SetDepthStencilState( DisableDepthWrite, 0 );
		SetDepthStencilState( DisableDepthWrite, 0 );
    }
}
