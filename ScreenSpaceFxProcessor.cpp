#include "ScreenSpaceFxProcessor.h"

#include "bloom_fx_hdr.h"
#include "fxaa_compiled_hdr"

CScreenSpaceFxProcessor::CScreenSpaceFxProcessor(void) : fxBufWidth(0),
	fxBufHeight(0),
	fBrightnessTolerance(0.3), 
	UseFXAA(true),
	debugShowBloomOnly(false),
	debugShowBrightpassOnly(false)
{
}


CScreenSpaceFxProcessor::~CScreenSpaceFxProcessor(void)
{
}

void CScreenSpaceFxProcessor::Init( ID3D10Device * device, ID3D10Texture2D * baseRenderTargetTex,ID3D10RenderTargetView*sourceRenderTargetView , const D3D10_VIEWPORT & srcVptDesc )
{
	D3D10_TEXTURE2D_DESC renderTargetDesc;

	ZeroMemory(&renderTargetDesc, sizeof(renderTargetDesc));

	srcRenderTargetView = sourceRenderTargetView;
	srcViewportDesc = srcVptDesc;

	fxBufWidth = srcVptDesc.Width / 8;
	fxBufHeight = srcVptDesc.Height / 8;

	bloomViewportDesc.Width = fxBufWidth;
	bloomViewportDesc.Height = fxBufHeight;
	bloomViewportDesc.MinDepth = 0.0f;
	bloomViewportDesc.MaxDepth = 1.0f;
	bloomViewportDesc.TopLeftX = 0;
	bloomViewportDesc.TopLeftY = 0;

	renderTargetDesc.Width = fxBufWidth;
	renderTargetDesc.Height = fxBufHeight;
	renderTargetDesc.MipLevels = 1;
	renderTargetDesc.ArraySize = 1;
	renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetDesc.SampleDesc.Count = 1;
	renderTargetDesc.SampleDesc.Quality = 0;
	renderTargetDesc.Usage = D3D10_USAGE_DEFAULT;
	renderTargetDesc.CPUAccessFlags = 0;
	renderTargetDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	renderTargetDesc.MiscFlags = D3D10_RESOURCE_MISC_GENERATE_MIPS;

	for(int c=0;c!=2;++c)
		if(FAILED(device->CreateTexture2D(&renderTargetDesc, NULL, &offscreenSmallTex[c])))
			return;


	for(int c=0;c!=2;++c)
		if(FAILED( device->CreateRenderTargetView( offscreenSmallTex[c], NULL, &offscreenSmallRenderTargetView[c] ) ) )
			return;

	renderTargetDesc.Width = srcVptDesc.Width;
	renderTargetDesc.Height = srcVptDesc.Height;

	if(FAILED(device->CreateTexture2D(&renderTargetDesc, NULL, &offscreenTex)))
		return;
	
	if(FAILED( device->CreateRenderTargetView( offscreenTex, NULL, &offscreenRenderTargetView ) ) )
		return;

	D3D10_SHADER_RESOURCE_VIEW_DESC resourceDesc;

	ZeroMemory(&resourceDesc, sizeof(resourceDesc));

	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resourceDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	resourceDesc.Texture2D.MostDetailedMip = 0;
	resourceDesc.Texture2D.MipLevels = 1;

	if(FAILED(device->CreateShaderResourceView(baseRenderTargetTex,&resourceDesc,&srcTexView)))
		return;

	ZeroMemory(&resourceDesc, sizeof(resourceDesc));

	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resourceDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	resourceDesc.Texture2D.MostDetailedMip = 0;
	resourceDesc.Texture2D.MipLevels = 1;

	for(int c=0;c!=2;++c)
		if(FAILED(device->CreateShaderResourceView(offscreenSmallTex[c],&resourceDesc,&offscSmallTexView[c])))
			return;

	if(FAILED(device->CreateShaderResourceView(offscreenTex,&resourceDesc,&offscreenTexView)))
		return;

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	auto hr = D3DX10CreateEffectFromMemory(bloom_fx_compiled,sizeof(bloom_fx_compiled),nullptr,nullptr,nullptr,"fx_4_0",dwShaderFlags,0,device,nullptr,nullptr,&fxBloom,nullptr,nullptr);
	if( FAILED( hr ) )
	{
		MessageBox( NULL,L"Could not load bloom shader.", L"Error", MB_OK );
		return;
	}

	hr = D3DX10CreateEffectFromMemory(fxaa_compiled,sizeof(fxaa_compiled),nullptr,nullptr,nullptr,"fx_4_0",dwShaderFlags,0,device,nullptr,nullptr,&fxFxaa,nullptr,nullptr);
	if( FAILED( hr ) )
	{
		MessageBox( NULL,L"Could not load FXAA shader.", L"Error", MB_OK );
		return;
	}


	// Define the input layout 
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4*4, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	fxBloom->GetTechniqueByIndex(0)->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	hr = device->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertex3Layout );
	if( FAILED( hr ) )
		return;

	///fullscreen quad

	Vertex3D vtxData[] = 
	{
		Vertex3D(D3DXVECTOR4(-1,-1,0.5,1),D3DXVECTOR2(0,1)),
		Vertex3D(D3DXVECTOR4(1,-1,0.5,1),D3DXVECTOR2(1,1)),
		Vertex3D(D3DXVECTOR4(-1,1,0.5,1),D3DXVECTOR2(0,0)),
		Vertex3D(D3DXVECTOR4(1,1,0.5,1),D3DXVECTOR2(1,0)),
	};

	D3D10_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc,sizeof(bufferDesc));
	bufferDesc.Usage = D3D10_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof( vtxData );
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData,sizeof(InitData));
	InitData.pSysMem = vtxData;
	hr = device->CreateBuffer( &bufferDesc, &InitData, &fsQuadBuf );
	if( FAILED( hr ) )
		return;
	
	fontRenderer = std::unique_ptr<CFontRenderer>(new CFontRenderer(MAKEINTRESOURCE(IDR_FONT_TEX),device));
}

void CScreenSpaceFxProcessor::Release()
{
	fontRenderer = nullptr;

	for (int c=0;c!=2;++c)
	{
		if(offscreenSmallTex[c]!=nullptr)
			offscreenSmallTex[c]->Release();
		if(offscreenSmallRenderTargetView[c]!=nullptr)
			offscreenSmallRenderTargetView[c]->Release();
		if(offscSmallTexView[c]!=nullptr)
			offscSmallTexView[c]->Release();
	}
	
	if(srcTexView!=nullptr)
		srcTexView->Release();
	if(fxBloom!=nullptr)
		fxBloom->Release();
	if(vertex3Layout!=nullptr)
		vertex3Layout->Release();
	if(fsQuadBuf!=nullptr)
		fsQuadBuf->Release();
	if(fxFxaa!=nullptr)
		fxFxaa->Release();

	if(offscreenRenderTargetView!=nullptr)
		offscreenRenderTargetView->Release();
	if(offscreenTex!=nullptr)
		offscreenTex->Release();
	if(offscreenTexView!=nullptr)
		offscreenTexView->Release();
}

void CScreenSpaceFxProcessor::Process( ID3D10Device * device,ID3D10ShaderResourceView * envmap, const CViewpoint & viewpoint )
{
	const int bloomStre = 3;

	if(UseFXAA)
	{
		//Process FXAA
		device->OMSetRenderTargets( 1, &offscreenRenderTargetView, nullptr );
		device->RSSetViewports( 1, &srcViewportDesc );
		processFxaa(device);

		//Copy result
		device->OMSetRenderTargets( 1, &srcRenderTargetView, nullptr );
		device->RSSetViewports( 1, &srcViewportDesc );
		DrawFsQuad(device,offscreenTexView,"CopyScreenQuadNoFilter");
	}
	
	if(ShowText)
		renderText(device);

	//Rescale

	device->OMSetRenderTargets( 1, &offscreenSmallRenderTargetView[1], nullptr );
	device->RSSetViewports( 1, &bloomViewportDesc );

	fxBloom->GetVariableByName("TexSize")->AsVector()->SetFloatVector(D3DXVECTOR2(srcViewportDesc.Width,srcViewportDesc.Height));

	DrawFsQuad(device,UseFXAA?offscreenTexView:srcTexView,"CopyScreenQuadSoftened");
	
	//Filter (brightpass)

	device->OMSetRenderTargets( 1, &offscreenSmallRenderTargetView[0], nullptr );
	device->RSSetViewports( 1, &bloomViewportDesc );

	fxBloom->GetVariableByName("brighnessTol")->AsScalar()->SetFloat(fBrightnessTolerance);
	fxBloom->GetVariableByName("flareBrighnessTol")->AsScalar()->SetFloat(0.2);
	fxBloom->GetVariableByName("TexSize")->AsVector()->SetFloatVector(D3DXVECTOR2(srcViewportDesc.Width,srcViewportDesc.Height));

	DrawFsQuad(device,offscSmallTexView[1],"CopyScreenQuad");

	if(debugShowBrightpassOnly)
	{
		device->OMSetRenderTargets( 1, &srcRenderTargetView, nullptr );
		device->RSSetViewports( 1, &srcViewportDesc );
		DrawFsQuad(device,offscSmallTexView[0],"CopyScreenQuadNoFilter");
		return;
	}

	fxBloom->GetVariableByName("TexSize")->AsVector()->SetFloatVector(D3DXVECTOR2(fxBufWidth,fxBufHeight));

	for (int c=0;c!=bloomStre;++c)
	{
		fxBloom->GetVariableByName("bHorizontal")->AsScalar()->SetBool(false);
		device->OMSetRenderTargets( 1, &offscreenSmallRenderTargetView[1], nullptr );
		DrawFsQuad(device,offscSmallTexView[0],"Blur");

		fxBloom->GetVariableByName("bHorizontal")->AsScalar()->SetBool(true);
		device->OMSetRenderTargets( 1, &offscreenSmallRenderTargetView[0], nullptr );
		DrawFsQuad(device,offscSmallTexView[1],"Blur");
	}

	device->OMSetRenderTargets( 1, &srcRenderTargetView, nullptr );
	device->RSSetViewports( 1, &srcViewportDesc );
	if(!debugShowBloomOnly)
		DrawFsQuad(device,offscSmallTexView[0],"AddFullScreenQuad");
	else
		DrawFsQuad(device,offscSmallTexView[0],"CopyScreenQuadNoFilter");
}

void CScreenSpaceFxProcessor::ProcessEnv( ID3D10Device * device,ID3D10ShaderResourceView * envmap, const CViewpoint & viewpoint )
{
	//draw skybox

	D3DXMATRIX mat;

	D3DXMatrixMultiply(&mat,&viewpoint.World3d,&viewpoint.View3d);
	D3DXMatrixMultiply(&mat,&mat,&viewpoint.Projection3d);
	D3DXMatrixInverse(&mat,nullptr,&mat);

	fxBloom->GetVariableByName("gWorldViewProjInv")->AsMatrix()->SetMatrix(mat);
	fxBloom->GetVariableByName("SkyboxTexture")->AsShaderResource()->SetResource(envmap);

	DrawFsQuad(device,offscSmallTexView[0],"RenderBackground");
}


void CScreenSpaceFxProcessor::DrawFsQuad(ID3D10Device * device,ID3D10ShaderResourceView * srcTextureView, LPCSTR name )
{
	ID3D10EffectTechnique * tech = fxBloom->GetTechniqueByName(name);

	fxBloom->GetVariableByName("srcTexture")->AsShaderResource()->SetResource(srcTextureView);

	// Set vertex buffer
	UINT stride = sizeof( Vertex3D );
	UINT offset = 0;
	device->IASetInputLayout(vertex3Layout);
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	device->IASetVertexBuffers( 0, 1, &fsQuadBuf, &stride, &offset );
	tech->GetPassByIndex( 0 )->Apply( 0 );
	device->Draw(4,0);

	fxBloom->GetVariableByName("srcTexture")->AsShaderResource()->SetResource(nullptr);
	tech->GetPassByIndex( 0 )->Apply( 0 );
}

void CScreenSpaceFxProcessor::processFxaa( ID3D10Device * device )
{
	ID3D10EffectTechnique * tech = fxFxaa->GetTechniqueByName("FXAA");

	fxFxaa->GetVariableByName("srcTexture")->AsShaderResource()->SetResource(srcTexView);

	fxFxaa->GetVariableByName("InverseViewportSize")->AsVector()->SetFloatVector(D3DXVECTOR2(1.0/srcViewportDesc.Width,1.0/srcViewportDesc.Height));
	fxFxaa->GetVariableByName("EdgeThreshold")->AsScalar()->SetFloat(0.166f);
	fxFxaa->GetVariableByName("EdgeThresholdMin")->AsScalar()->SetFloat(0);
	fxFxaa->GetVariableByName("SubPixelAliasingRemoval")->AsScalar()->SetFloat(0.75);

	// Set vertex buffer
	UINT stride = sizeof( Vertex3D );
	UINT offset = 0;
	device->IASetInputLayout(vertex3Layout);
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	device->IASetVertexBuffers( 0, 1, &fsQuadBuf, &stride, &offset );
	tech->GetPassByIndex( 0 )->Apply( 0 );
	device->Draw(4,0);

	fxFxaa->GetVariableByName("srcTexture")->AsShaderResource()->SetResource(nullptr);
	tech->GetPassByIndex( 0 )->Apply( 0 );
}

void CScreenSpaceFxProcessor::renderText(ID3D10Device * device)
{
	auto color = D3DXVECTOR4(1,0.9,1,0.2);

	fontRenderer->RenderString(device,headerText,D3DXVECTOR2(0,0),color);

	auto dim = fontRenderer->MeasureString(footerText);

	fontRenderer->RenderString(device,footerText,D3DXVECTOR2(srcViewportDesc.Width - dim.x,srcViewportDesc.Height - dim.y),color);
}

