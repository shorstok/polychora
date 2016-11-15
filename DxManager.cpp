#include "DxManager.h"

#include "fx4d_compiled_hdr"

#include <thread>
#include <string>

#include <algorithm>

#include "PolychoraSettings.h"


void LoadEffectsTexturesAsnc(CDxManager * mgr);

CDxManager::CDxManager(HWND hWnd,CScreenSaverModeHandler * ssMode) : 
	windowHandle(hWnd),
	driverType(D3D10_DRIVER_TYPE_NULL),
	device(nullptr),
	swapChain(nullptr),
	renderTargetView(nullptr),
	depthStencil(nullptr),
	depthStencilView(nullptr),
	effectRender4D(nullptr),
	noiseTexture(nullptr),
	depthShaderView(nullptr),
	envMapSRV(nullptr),
	isLoaded(false),
	EvolutionFunction(nullptr),
	plain_acc_time(0),
	UseVSync(true)

{

	ComplexEvolutionFunctions.push_back(std::bind(&CDxManager::Evolve_ChaoticPeriodicPersistent,this, std::placeholders::_1, std::placeholders::_2));
	ComplexEvolutionFunctions.push_back(std::bind(&CDxManager::Evolve_Weierstrass,this, std::placeholders::_1, std::placeholders::_2));

	for (int i = 0; i!=4;++i)
	{
		for (int j=i+1;j<4;++j)
			_rotationPlanesShuffled.push_back(std::pair<int,int>(i,j));
	}

	CPolychoraSettings settings;

	envRotationSeed = D3DX_PI * 2 * rand() / RAND_MAX;
	envRotationDir = rand()%2 == 1 ? 1: -1;

	if(!settings.UseComplexRotations)
		EvolutionFunction = std::bind(&CDxManager::Evolve_Plain,this, std::placeholders::_1, std::placeholders::_2);
	else
		EvolutionFunction = std::bind(&CDxManager::Evolve_ChaoticPeriodicPersistent,this, std::placeholders::_1, std::placeholders::_2);

	ssfxp.UseFXAA = settings.UseFXAA;
	ssfxp.ShowText = settings.ShowText && ssMode->CurrentMode() != CScreenSaverModeHandler::Preview;


	if(CPolychora::PolychoraGenerators.find(settings.Polychoron) == CPolychora::PolychoraGenerators.end())
		polychora = CPolychora::Tesseract();
	else
		polychora = CPolychora::PolychoraGenerators[settings.Polychoron]();

	ssfxp.footerText = polychora->Name;

	std::random_shuffle(_rotationPlanesShuffled.begin(),_rotationPlanesShuffled.end());
}


CDxManager::~CDxManager(void)
{
	delete polychora;
}

HRESULT CDxManager::InitDevice()
{
	try
	{
		isLoaded = false;

		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect( windowHandle, &rc );
		WindowWidth = rc.right - rc.left;
		WindowHeight = rc.bottom - rc.top;

		UINT createDeviceFlags = 0;
	#ifdef _DEBUG
		createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
	#endif

		D3D10_DRIVER_TYPE driverTypes[] =
		{
			D3D10_DRIVER_TYPE_HARDWARE,
		};

		UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory( &sd, sizeof( sd ) );
		sd.BufferCount = 1;
		sd.BufferDesc.Width = WindowWidth;
		sd.BufferDesc.Height = WindowHeight;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		//sd.Flags|= D3D10_BIND_SHADER_RESOURCE;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT|DXGI_USAGE_SHADER_INPUT;
		sd.OutputWindow = windowHandle;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
		{
			driverType = driverTypes[driverTypeIndex];
			hr = D3D10CreateDeviceAndSwapChain( NULL, driverType, NULL, createDeviceFlags,
				D3D10_SDK_VERSION, &sd, &swapChain, &device );
			if( SUCCEEDED( hr ) )
				break;
		}
		if( FAILED( hr ) )
		{
			wchar_t buf[1024];

			std::wstring errDsc=L"?";
			
			switch (hr)
			{
			case D3D10_ERROR_FILE_NOT_FOUND:
				errDsc = L"D3D10_ERROR_FILE_NOT_FOUND";
				break;
			case D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
				errDsc = L"D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
				break;
			case D3DERR_INVALIDCALL:
				errDsc = L"D3DERR_INVALIDCALL";
				break;
			case D3DERR_WASSTILLDRAWING:
				errDsc = L"D3DERR_WASSTILLDRAWING";
				break;
			case E_FAIL:
				errDsc = L"E_FAIL";
				break;
			case E_INVALIDARG:
				errDsc = L"E_INVALIDARG";
				break;
			case E_OUTOFMEMORY:
				errDsc = L"E_OUTOFMEMORY";
				break;
			case E_NOTIMPL:
				errDsc = L"E_NOTIMPL";
			case S_FALSE:
				errDsc = L"S_FALSE";
			case S_OK:
				errDsc = L"S_OK";
				break;
			default:
				break;
			}

			wsprintf(buf, L"Cannot create DX10 device, Code: %ls, %x",errDsc.c_str(),hr);

			OutputDebugString(buf);
			throw L"Cannot create DX10 device";
		}

		//Create loader fx
		loaderEffect =new LoaderEffect(device,WindowWidth,WindowHeight);


		// Create a render target view
		ID3D10Texture2D* pBuffer;
		hr = swapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBuffer );
		if( FAILED( hr ) )
			throw L"Cannot create swap chain";

		hr = device->CreateRenderTargetView( pBuffer, NULL, &renderTargetView );
		if( FAILED( hr ) )
			throw L"Cannot create render target view";

		// Setup the viewport
		D3D10_VIEWPORT vp;
		vp.Width = WindowWidth;
		vp.Height = WindowHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		ssfxp.Init(device,pBuffer,renderTargetView,vp);

		pBuffer->Release();

		D3D10_TEXTURE2D_DESC depthBufferDesc;

		// Initialize the description of the depth buffer.
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
		depthBufferDesc.Width = WindowWidth;
		depthBufferDesc.Height = WindowHeight;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D10_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL| D3D10_BIND_SHADER_RESOURCE;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		// Create the texture for the depth buffer using the filled out description.
		if(FAILED(device->CreateTexture2D(&depthBufferDesc, NULL, &depthStencil)))
			throw L"Cannot create depth buffer";

		D3D10_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

		// Initailze the depth stencil view.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		// Create the depth stencil view.
		if(FAILED(device->CreateDepthStencilView(depthStencil, &depthStencilViewDesc, &depthStencilView)))
			throw L"Cannot create CreateDepthStencilView";

		D3D10_SHADER_RESOURCE_VIEW_DESC SRDesc;

		ZeroMemory(&SRDesc, sizeof(SRDesc));

		SRDesc.Format = DXGI_FORMAT_R32_FLOAT;
		SRDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		SRDesc.Texture2D.MostDetailedMip = 0;
		SRDesc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(
			depthStencil,
			&SRDesc,
			&depthShaderView);

		device->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );
		device->RSSetViewports( 1, &vp );

		loader = new std::thread(LoadEffectsTexturesAsnc,this);
		return S_OK;
	}
	catch (LPWSTR xc)
	{
		MessageBox(NULL,xc,L"Polychora DirectX error",MB_OK);
		exit(0);
	}
}

void CDxManager::CleanupDevice()
{
	if( device ) device->ClearState();

	if( noiseTexture ) noiseTexture->Release();
	if( effectRender4D ) effectRender4D->Release();
	if( renderTargetView ) renderTargetView->Release();
	if( swapChain ) swapChain->Release();
	if( envMapSRV ) envMapSRV->Release();
	if( depthStencil ) depthStencil->Release();
	if( depthStencilView ) depthStencilView->Release();
	if( depthShaderView ) depthShaderView->Release();

	polychora->Release();
	ssfxp.Release();

	delete loaderEffect;

	if( device ) 
		device->Release();
}

void CDxManager::Render()
{
	static float timeSinceStart_sec = 0.0f;
	static float timeLastFrame_sec = 0.0f;
	static unsigned int framesSinceStart = 0;
	static DWORD dwTimeRenderStart = 0;

	if( driverType == D3D10_DRIVER_TYPE_REFERENCE )
		timeSinceStart_sec += ( float )D3DX_PI * 0.0125f;
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if( dwTimeStart == 0 )
			dwTimeStart = dwTimeCur;
		timeSinceStart_sec =( dwTimeCur - dwTimeStart ) / 1000.0f;
	}

	if(!isLoaded)
	{
		RenderLoadingStub(timeSinceStart_sec);
		return;
	}

	framesSinceStart++;

	if( dwTimeRenderStart == 0 )
		dwTimeRenderStart = GetTickCount();

	if((GetTickCount() - dwTimeRenderStart) > 1000)
	{
		auto secSinceStart = (GetTickCount() - dwTimeRenderStart)/1e3f;

		auto fps = framesSinceStart / secSinceStart;

		if(fps < 55)
		{
			UseVSync = false;
			ssfxp.headerText = L"VSYNC off";
		}
		else
			ssfxp.headerText = L"VSYNC on";

	}

	float delta_sec = max(timeSinceStart_sec - timeLastFrame_sec,9e-4);

	effectRender4D->GetVariableByName("Time" )->AsScalar()->SetFloat(timeSinceStart_sec);

	if(EvolutionFunction!=nullptr)
		EvolutionFunction(timeSinceStart_sec,delta_sec);

	rotationPhase3d = envRotationSeed + envRotationDir*timeSinceStart_sec/300;

	viewpoint.From3.x = cos(rotationPhase3d);
	viewpoint.From3.z = sin(rotationPhase3d);
	viewpoint.From3.y = sin(rotationPhase3d+2);

	D3DXVec3Normalize(&viewpoint.From3,&viewpoint.From3);

	viewpoint.From3*=10;
	rotationPhase3d*=5;

	effectRender4D->GetVariableByName( "vFlareSource3D" )->AsVector()->SetFloatVector(D3DXVECTOR3(12*cos(rotationPhase3d),12*cos(rotationPhase3d),12*sin(rotationPhase3d)));

	polychora->vFaceSpecularAmp = fabs(sin(timeSinceStart_sec*3/100));
	polychora->vFlareSpecularAmp = fabs(sin(timeSinceStart_sec*3/100))*0.9f+0.1f;

	viewpoint.SetProjections(effectRender4D,WindowWidth,WindowHeight);

	// Clear the back buffer
	const float ClearColor[4] = { 0.0f, 0, 0, 1.0f }; // rgba
	device->ClearRenderTargetView(renderTargetView, ClearColor);
	device->ClearDepthStencilView(depthStencilView,D3D10_CLEAR_DEPTH, 	1.0, 0);

	//Render scene to main buffer
	device->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

	//Sky
	ssfxp.ProcessEnv(device,envMapSRV,viewpoint);

	//Polychora
	polychora->Render(effectRender4D,device, &viewpoint);

	//Add some effects
	ssfxp.Process(device,envMapSRV,viewpoint);

	RenderText();
	
	// Present our back buffer to our front buffer
	swapChain->Present( UseVSync? 1 : 0, 0 );

	timeLastFrame_sec = timeSinceStart_sec;
}


void CDxManager::RenderText()
{

}


void CDxManager::RenderLoadingStub( float time )
{
	loaderEffect->Render(device,renderTargetView,time);
	swapChain->Present( 0, 0 );
}

void CDxManager::Evolve_Weierstrass( float timeAbs,float timeDelta )
{
	float timeDiv100_sec = timeAbs / 100;

	std::vector<float> rotateBasis;

	const int nbasis = 6;

	const float period_sec = 20;

	const float w = 2*D3DX_PI / period_sec;

	const float phi = 2*D3DX_PI/nbasis;

	const float normalizer = sqrt(2/(float)nbasis);

	float rotspeed = 0;

	const float weierstrass_period_sec = 70;
	
	
	for (int k =0;k!=6;++k)
	{
		rotspeed = 0;

		//Weierstrass fractal function
		for (int c=0;c!=15;++c)
			rotspeed+=pow(0.6f,c) * cos(pow(5,c) * (timeAbs+(D3DX_PI/3 * k)) *D3DX_PI * 2 / weierstrass_period_sec);

		rotateBasis.push_back(rotspeed * normalizer);
		//rotateBasis.push_back(sin(timeAbs*w - c*phi)*normalizer * rotspeed);
	}

	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[0],0,1);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[1],0,2);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[2],0,3);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[3],1,2);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[4],1,3);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[5],2,3);
}

void CDxManager::Evolve_ChaoticPeriodicPersistent( float timeAbs,float timeDelta )
{
	float timeDiv100_sec = timeAbs / 100;

	float rotateBasis[] = 
	{
		pow(sin(timeDiv100_sec),9),
		pow(cos(timeDiv100_sec),9),
		pow(sin(timeDiv100_sec*0.982+133),9),
		pow(cos(timeDiv100_sec*0.982+133),9),
		pow(sin(timeDiv100_sec*1.152+893),9),
		pow(cos(timeDiv100_sec*1.152+893),9),
	};

	float s =0;

	for (auto v : rotateBasis)
		s+=fabs(v) / (sizeof(rotateBasis)/sizeof(rotateBasis[0]));

	s*=6;

	for (auto & v : rotateBasis)
		v/=s;

	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[0],0,1);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[1],0,2);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[2],0,3);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[3],1,2);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[4],1,3);
	viewpoint.Rotate4DBasis(timeDelta*rotateBasis[5],2,3);

	if(timeAbs> 3)
		ssfxp.headerText =L"";
}

void CDxManager::Evolve_Plain( float timeAbs,float timeDelta )
{
	wchar_t buf[1024];
	wchar_t planes[] = {'x','y','z','w'};
	const float RotSpeed = 1/2.0f;
	const float easeTime_sec = 1;
	const float switchPeriod_sec = /*one 360° turn*/ 2*D3DX_PI;
	
	int cur_plane_pair = ((int)floor(plain_acc_time / switchPeriod_sec)) %_rotationPlanesShuffled.size();
	int next_plane_pair = (cur_plane_pair + 1) %  _rotationPlanesShuffled.size();

	float period_time = fmod(plain_acc_time,switchPeriod_sec);

	float k = 1 - (switchPeriod_sec - period_time) / easeTime_sec;

	timeDelta *=RotSpeed;

	plain_acc_time+=timeDelta;

	if(k <= 0)
		viewpoint.Rotate4DBasis(timeDelta,_rotationPlanesShuffled[cur_plane_pair].first,_rotationPlanesShuffled[cur_plane_pair].second);
	else
	{
		float b = k * k * (3 - 2 * k); //Bezier easing fn

		viewpoint.Rotate4DBasis(timeDelta*b,_rotationPlanesShuffled[next_plane_pair].first,_rotationPlanesShuffled[next_plane_pair].second);
		viewpoint.Rotate4DBasis(timeDelta*(1-b),_rotationPlanesShuffled[cur_plane_pair].first,_rotationPlanesShuffled[cur_plane_pair].second);
	}

	swprintf(buf,L"Rotating over %c%c planes",planes[_rotationPlanesShuffled[cur_plane_pair].first],planes[_rotationPlanesShuffled[cur_plane_pair].second]);
	
	if(timeAbs> 3)
		ssfxp.headerText = buf;
}

EvoFunctor CDxManager::PickNextEvolutorFrom( const std::vector<EvoFunctor> & evoFnPool )
{	
	return evoFnPool[_pickerSeeds[&evoFnPool]++ % evoFnPool.size()];
}



void LoadEffectsTexturesAsnc(CDxManager * mgr)
{
	// Create the effect
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	ID3D10Effect * fx;

	auto hr = D3DX10CreateEffectFromMemory(fx4d_compiled,sizeof(fx4d_compiled),nullptr,nullptr,nullptr,"fx_4_0",dwShaderFlags,0,mgr->device,nullptr,nullptr,&fx,nullptr,nullptr);
	if(FAILED( hr ))
	{
		MessageBox( NULL,
			L"The FX file cannot be located.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		return;
	}


	// Load environment texture
	if(FAILED(D3DX10CreateShaderResourceViewFromResource( mgr->device,0, MAKEINTRESOURCE(IDR_NEBULA_TEX), NULL, NULL, &mgr->envMapSRV, NULL )))
		return;   

	fx->GetVariableByName( "SkyboxTexture" )->AsShaderResource()->SetResource(mgr->envMapSRV);

	// Load noise texture
	if(FAILED(D3DX10CreateShaderResourceViewFromResource( mgr->device,0, MAKEINTRESOURCE(IDR_NOISETEX), NULL, NULL, &mgr->noiseTexture, NULL )))
		return;   

	fx->GetVariableByName( "txNormal" )->AsShaderResource()->SetResource( mgr->noiseTexture );

	mgr->polychora->CreateVertexBuffers(fx,mgr->device);

	mgr->effectRender4D = fx;

	mgr->isLoaded = true;
}
