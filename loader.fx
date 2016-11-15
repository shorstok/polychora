
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

float Time;

cbuffer constants
{
	float K;
}

PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output;

	float cs = cos(Time);
	float sn = sin(Time);

    output.Pos = input.Pos;
	
	output.Pos.x = (input.Pos.x * cs  - input.Pos.y * sn)*K; 
	output.Pos.y = input.Pos.x * sn + input.Pos.y * cs;

    output.Pos.w = 1;

    output.Tex = input.Tex;

    return output;
}


float4 PS( PS_INPUT input) : SV_Target
{
	return float4(255,160,231,255)/255.0 * saturate(Time);
}



technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
        
		SetBlendState(NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFff );
		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( DisableDepth, 0 );
    }
}
