
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txNormal;
Texture2D txDepth;
TextureCube SkyboxTexture; 


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




SamplerState SkyboxSampler
{ 
   Filter = MIN_MAG_MIP_LINEAR;   
   //MaxAnisotropy=16;
   //ComparisonFunc = ALWAYS;
   AddressU = Wrap; 
   AddressV = Wrap; 
};

BlendState NoBlending
{
    BlendEnable[0] = FALSE;
};

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;   
};

DepthStencilState DisableDepthWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
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

		
RasterizerState SolidState {
	FillMode = SOLID;
    CullMode  =BACK; 
};
		
RasterizerState SolidStateFront {
	FillMode = SOLID;
    CullMode  =FRONT; 
};

RasterizerState SolidStateNoCull {
	FillMode = SOLID;
    CullMode  =NONE; 
};



RasterizerState WireState {
	FillMode = WIREFRAME;
    CullMode  = NONE; 
};

cbuffer cbConstants
{
    bool bProject4D;

};

cbuffer cbChangeOnResize
{
    matrix Projection;
    float2 RenderTargetSize;
};

cbuffer cbChangesEveryFrame
{
    matrix World;
	float3 CameraPosition;
	matrix View;

	float Time;

	//4D view matrix equiv.

	float4 Va;
	float4 Vb;
	float4 Vc;
	float4 Vd;
	float4 From4;

	float ViewangleTanInv;

	float3 vFlareSource3D;


	float4 vFlareSpecularColor;
	float4 vFaceSpecularColor;
	float3 vFaceEnviColor;
};

struct VS_INPUT
{
    float4 Pos : POSITION0;
    float4 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 PosOrig : TEXCOORD0;
    float3 Normal : TEXCOORD1;
};


float4 Project4DTo3D(float4 source)
{
	float4 t = source - From4;
	//float depth4 = dot(t,Vd);

	float rtmp = ViewangleTanInv/dot(t,Vd);

	return float4(dot(t,Va),dot(t,Vb),dot(t,Vc),1/rtmp)*rtmp;
}


const float shpereRad = 7e-2;
const float tubeRad = 5e-2;

/*
Creates half of an icosphere facing viewer
*/
void CreateHalfIcosphere(float3 pos,matrix wvp,inout TriangleStream<PS_INPUT> output)
{
	const float t = (1.0 + sqrt(5.0)) / 2.0;

	const float3 up = float3(0,1,0);

	//Construct matrix to make hemisphere always face the camera

	float3 zaxis = normalize(CameraPosition - pos);
	float3 xaxis = normalize(cross(up,zaxis));
	float3 yaxis = cross(zaxis,xaxis);

	matrix rot = 0;

	rot._44 = 1;
	rot._11_12_13 = xaxis;
	rot._21_22_23 = yaxis;
	rot._31_32_33 = zaxis;

	//Construct hemisphere

	PS_INPUT v[12];

	v[0].Pos.xyz = float4(-1,t,0,1);
	v[1].Pos.xyz = float4(1,t,0,1);
	v[2].Pos.xyz = float4(-1,-t,0,1);
	v[3].Pos.xyz = float4(1,t,0,1);

	v[4].Pos.xyz = float4(0,-1,t,1);
	v[5].Pos.xyz = float4(0,1,t,1);
	v[6].Pos.xyz = float4(0,-1,-t,1);
	v[7].Pos.xyz = float4(0,1,t,1);

	v[8].Pos.xyz =  float4(t,0,-1,1);
	v[9].Pos.xyz =  float4(t,0,1,1);
	v[10].Pos.xyz = float4(-t,0,-1,1);
	v[11].Pos.xyz = float4(-t,0,1,1);

	[unroll]
	for(int c=0;c!=12;++c)
	{
		v[c].Pos.w = 1;
		v[c].Pos = mul(v[c].Pos*shpereRad,rot);	//rotate icosa to face camera & scale it to shpereRad
		v[c].Normal = normalize(v[c].Pos);
		v[c].PosOrig = float4(v[c].Pos + pos,1);
		v[c].Pos = mul(v[c].PosOrig,wvp);
	}

	output.Append(v[0]);
	output.Append(v[11]);
	output.Append(v[5]);

	output.RestartStrip();

	output.Append(v[0]);
	output.Append(v[5]);
	output.Append(v[1]);

	output.RestartStrip();

	output.Append(v[0]);
	output.Append(v[1]);
	output.Append(v[7]);

	output.RestartStrip();

	output.Append(v[0]);
	output.Append(v[7]);
	output.Append(v[10]);

	output.RestartStrip();
	
	output.Append(v[0]);
	output.Append(v[10]);
	output.Append(v[11]);

	output.RestartStrip();
		
	output.Append(v[1]);
	output.Append(v[5]);
	output.Append(v[9]);

	output.RestartStrip();

	output.Append(v[5]);
	output.Append(v[11]);
	output.Append(v[4]);

	output.RestartStrip();
	
	output.Append(v[11]);
	output.Append(v[10]);
	output.Append(v[2]);

	output.RestartStrip();
		
	output.Append(v[2]);
	output.Append(v[4]);
	output.Append(v[11]);

	output.RestartStrip();

	output.Append(v[4]);
	output.Append(v[9]);
	output.Append(v[5]);

	output.RestartStrip();
}

[maxvertexcount(60)]
void GSSpheres(line PS_INPUT points[2], inout TriangleStream<PS_INPUT> output)
{
	matrix wvp = mul(mul(World,View),Projection);

	CreateHalfIcosphere(points[0].Pos,wvp,output);
	CreateHalfIcosphere(points[1].Pos,wvp,output);
}



[maxvertexcount(24)]
void GS(line PS_INPUT points[2], inout TriangleStream<PS_INPUT> output)
{
	float4 pos[4] = {points[0].Pos,points[0].Pos,points[1].Pos,points[1].Pos};
	float4 dir = normalize(pos[2]-pos[1]);

	const float4 up = float4(0,1,0,1);
	const float4 left = float4(-1,0,0,1);
	const float4 fw = float4(0,0,1,1);

	float3 u = left;

	if(length(cross(dir,u))<1e-2)
		u = up;

	float3 i,j,k;

	j=dir;
	i = normalize(cross(u,j));
	k = normalize(cross(i,j));

	i*=tubeRad;
	k*=tubeRad;

	PS_INPUT vtx[8];

	matrix wvp = mul(mul(World,View),Projection);

	vtx[0].Pos = float4(pos[1] + i +k,1);
	vtx[1].Pos = float4(pos[1] + i -k,1);
	vtx[2].Pos = float4(pos[1] - i -k,1);
	vtx[3].Pos = float4(pos[1] - i +k,1);

	vtx[4].Pos = float4(pos[2] + i + k,1);
	vtx[5].Pos = float4(pos[2] + i - k,1);
	vtx[6].Pos = float4(pos[2] - i - k,1);
	vtx[7].Pos = float4(pos[2] - i + k,1);

	[unroll]
	for(int c=0;c!=8;c++)
	{
		vtx[c].PosOrig = vtx[c].Pos;
		vtx[c].Normal = normalize(vtx[c].PosOrig - pos[c<4?1:2]);
		vtx[c].Pos = mul(vtx[c].Pos,wvp);
	}
		 
    output.Append(vtx[0]);
    output.Append(vtx[1]);
	output.Append(vtx[3]);
	output.Append(vtx[2]);

    output.RestartStrip();

	output.Append(vtx[5]);
    output.Append(vtx[4]);
	output.Append(vtx[6]);
	output.Append(vtx[7]);

    output.RestartStrip();
	    
	output.Append(vtx[4]);
    output.Append(vtx[5]);
	output.Append(vtx[0]);
	output.Append(vtx[1]);

    output.RestartStrip();
	    
	output.Append(vtx[3]);
    output.Append(vtx[2]);
	output.Append(vtx[7]);
	output.Append(vtx[6]);

    output.RestartStrip();
		    
	output.Append(vtx[0]);
    output.Append(vtx[3]);
	output.Append(vtx[4]);
	output.Append(vtx[7]);

    output.RestartStrip();
	    
	output.Append(vtx[2]);
    output.Append(vtx[1]);
	output.Append(vtx[6]);
	output.Append(vtx[5]);

    output.RestartStrip();

}

[maxvertexcount(3)]
void GSFaces(triangle PS_INPUT points[3], inout TriangleStream<PS_INPUT> output)
{
	matrix wvp = mul(mul(World,View),Projection);

	float3 director = normalize(points[0].Pos);		//since all our geometry should be convex

	float3 norm = normalize(cross(points[0].Pos - points[1].Pos,points[2].Pos - points[0].Pos));

	if(dot(norm,director) < 0)
		norm = -norm;

	[unroll]
	for(int c=0;c!=3;++c)
	{
		points[c].PosOrig = points[c].Pos;
		points[c].Pos = mul(points[c].Pos,wvp);
		points[c].Normal = norm;
	}

	output.Append(points[0]);
    output.Append(points[1]);
	output.Append(points[2]);

    output.RestartStrip();
}



//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

	if(bProject4D)
	{
		input.Pos = Project4DTo3D(input.Pos);
		input.Normal = Project4DTo3D(input.Normal);
	}

 	output.PosOrig = input.Pos;
    output.Pos = input.Pos;
	output.Normal = input.Normal;

    return output;
}


float3 TriBump(float3 pos, float3 norm)
{
	const float2 normtex_Scale = float2(2,2);

	float2 coord1 = pos.yz * normtex_Scale;
	float2 coord2 = pos.zx * normtex_Scale;
	float2 coord3 = pos.xy * normtex_Scale;

	float3 nor1 = txNormal.Sample(samLinear, coord1).rgb * 2.0 - 1.0;
	float3 nor2 = txNormal.Sample(samLinear, coord2).rgb  * 2.0 - 1.0;
	float3 nor3 = txNormal.Sample(samLinear, coord3).rgb  * 2.0 - 1.0;

	//Bring the 3 normal vectors into object/world space
	float3 Normal1 = float3(0, nor1.x, nor1.y);
	float3 Normal2 = float3(nor2.y, 0, nor2.x);
	float3 Normal3 = float3(nor3.x, nor3.y, 0);

	norm+=nor1 * norm.x + nor2* norm.y + nor3*norm.z;
	norm+=sin(pos.x) * norm.x + sin(pos.y)* norm.y + sin(pos.z)*norm.z;


	return normalize(norm);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------


float4 PS( PS_INPUT input) : SV_Target
{
	float3 ViewDirection = CameraPosition - input.PosOrig;

	float3 incident = -normalize(ViewDirection);
	float3 Reflection  = reflect(incident, input.Normal);

	float3 refLt = reflect(-normalize(vFlareSource3D - input.PosOrig),input.Normal);

	float specbase = saturate(dot(refLt,ViewDirection));

	const float envAmp= 2;

	float4 specular = pow(specbase,1)*vFlareSpecularColor;
		
	float4 env = SkyboxTexture.Sample(SkyboxSampler,  normalize(Reflection))*envAmp;
	
    return saturate(specular+env);
}

float4 PSWhite( PS_INPUT input) : SV_Target
{
	   return float4(1,1,1,0.6);
}




//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSFaces( PS_INPUT input) : SV_Target
{
	float3 ViewDirection = CameraPosition - input.PosOrig;

	ViewDirection = normalize(ViewDirection);
		
	input.Normal = TriBump(input.PosOrig, input.Normal);

	float3 Reflection  = reflect(-ViewDirection, input.Normal);

	float4 envi = SkyboxTexture.Sample(SkyboxSampler,  normalize(Reflection))*float4(vFaceEnviColor.rgb,pow(saturate( 1- abs(dot(ViewDirection,input.Normal))),8));

	float3 refLt = reflect(-normalize(vFlareSource3D - input.PosOrig),input.Normal);
	float4 specular = pow(saturate(dot(refLt,ViewDirection)),3)*vFaceSpecularColor;	

    return saturate(envi + specular);
}


//--------------------------------------------------------------------------------------

technique10 RenderFaces
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(CompileShader(gs_4_0, GSFaces()));
        SetPixelShader( CompileShader( ps_4_0, PSFaces() ) );
        
		SetBlendState( SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(SolidStateNoCull);

		SetDepthStencilState( DisableDepthWrite, 0 );
    }
}

technique10 RenderCubes
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(CompileShader(gs_4_0, GS()));
        SetPixelShader( CompileShader( ps_4_0, PS() ) );

		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );

		SetRasterizerState(SolidStateNoCull);
		SetDepthStencilState( EnableDepth, 0 );
//		SetBlendState( SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
//		SetPixelShader( CompileShader( ps_4_0, PSWhite() ) );
//		SetRasterizerState(WireState);
    }

	pass P1
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader(CompileShader(gs_4_0, GSSpheres()));
        SetPixelShader( CompileShader( ps_4_0, PS() ) );

		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );

		SetRasterizerState(SolidState);
		SetDepthStencilState( EnableDepth, 0 );
    }


}

