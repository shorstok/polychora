#include "LoaderEffect.h"

#include "ScreenSpaceFxProcessor.h"

#include "loader_fx_compiled_hdr"

LoaderEffect::LoaderEffect( ID3D10Device * device,int initWidth,int initHeight ) : IsValid(false)
{
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	if( FAILED(D3DX10CreateEffectFromMemory(loader_fx_compiled,sizeof(loader_fx_compiled),nullptr,nullptr,nullptr,"fx_4_0",dwShaderFlags,0,device,nullptr,nullptr,&effect,nullptr,nullptr)))
	{
		MessageBox( NULL,L"Could not load `loader` shader.", L"Error", MB_OK );
		return;
	}

	resources.push_back(effect);
		

	// Define the input layout 
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4*4, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	effect->GetTechniqueByIndex(0)->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	if( FAILED( device->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertex3Layout )) )
		return;

	resources.push_back(vertex3Layout);

	///vtx

	std::vector<Vertex3D> vtxData;

	const int nleaf_max = 30;

	const float leaf_sector_max = D3DX_PI * 2 / nleaf_max;
	const float leaf_sector = leaf_sector_max * 0.2;

	const float leaf_start_rad = 0.1;
	const float leaf_fin_rad = 0.15;

	const float leaf_start_norm = sin(leaf_sector) * leaf_start_rad;
	const float leaf_fin_norm = sin(leaf_sector) * leaf_fin_rad;

	float k = initHeight / (float)initWidth;

	for (int nleaf=0;nleaf!=nleaf_max;++nleaf)
	{
		D3DXVECTOR3 lstart,lfin;

		float angle = leaf_sector_max * nleaf;

		auto fs = D3DXVECTOR2(cos(angle),sin(angle)) * leaf_start_rad;
		auto fe = D3DXVECTOR2(cos(angle),sin(angle)) * leaf_fin_rad;
		auto fn = D3DXVECTOR2(fs.y - fe.y, fe.x - fs.x);

		D3DXVec2Normalize(&fn,&fn);

		D3DXVECTOR2 pts[] = {
			fe - fn*leaf_fin_norm,
			fe + fn*leaf_fin_norm,
			fs - fn*leaf_start_norm,

			fe + fn*leaf_fin_norm,
			fs + fn*leaf_start_norm,
			fs - fn*leaf_start_norm,
		};

		for (auto & p: pts)
			vtxData.push_back(Vertex3D(D3DXVECTOR4(p.x,p.y,0.5,1),D3DXVECTOR2(p.x/2 + 0.5,0.5 - p.y/2)));
	}

	nVtx = vtxData.size();

	D3D10_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc,sizeof(bufferDesc));
	bufferDesc.Usage = D3D10_USAGE_DEFAULT;
	bufferDesc.ByteWidth = vtxData.size() * sizeof(Vertex3D);
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData,sizeof(InitData));
	InitData.pSysMem = &vtxData[0];
	
	if( FAILED( device->CreateBuffer( &bufferDesc, &InitData, &fsQuadBuf ) ))
		return;

	resources.push_back(fsQuadBuf);

	effect->GetVariableByName("K")->AsScalar()->SetFloat(k);

	IsValid = true;
}


LoaderEffect::~LoaderEffect(void)
{
	for (auto & r: resources)
		r->Release();
}


void LoaderEffect::Render( ID3D10Device * device, ID3D10RenderTargetView * renderTarget, float time )
{
	float colors[] = {0,0,0,1};

	device->ClearRenderTargetView(renderTarget,colors);

	if(!IsValid)
		return;

	ID3D10EffectTechnique * tech = effect->GetTechniqueByName("Render");
	
	effect->GetVariableByName("Time")->AsScalar()->SetFloat(time);

	// Set vertex buffer
	UINT stride = sizeof( Vertex3D );
	UINT offset = 0;
	device->IASetInputLayout(vertex3Layout);
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device->IASetVertexBuffers( 0, 1, &fsQuadBuf, &stride, &offset );
	tech->GetPassByIndex( 0 )->Apply( 0 );
	device->Draw(nVtx,0);
}
