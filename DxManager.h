#pragma once

#include <d3d10.h>
#include <d3dx10.h>

#include <windows.h>
#include "resource.h"

#include "Viewpoint.h"
#include "Polychora.h"

#include <thread>

#include "ScreenSpaceFxProcessor.h"

#include "LoaderEffect.h"

#include <memory>

#include <map>

#include "ScreenSaverModeHandler.h"

typedef std::function<void(float,float)> EvoFunctor;

class CDxManager
{
public:
	CDxManager(HWND hwnd,CScreenSaverModeHandler * ssMode);
	~CDxManager(void);
	HRESULT InitDevice();
	
	void CleanupDevice();
	void Render();
	void RenderLoadingStub(float time);
	void RenderText();


	EvoFunctor PickNextEvolutorFrom(const std::vector<EvoFunctor> & evoFnPool);

private:

	std::map<const std::vector<EvoFunctor> *,int> _pickerSeeds;

public:
	D3D10_DRIVER_TYPE                   driverType;
	ID3D10Device*						device;
	IDXGISwapChain*                     swapChain;
	ID3D10RenderTargetView*             renderTargetView;
	ID3D10Texture2D*					depthStencil;
	ID3D10DepthStencilView*             depthStencilView;
	ID3D10Effect*                       effectRender4D;

	ID3D10ShaderResourceView*           noiseTexture;
	ID3D10ShaderResourceView*           depthShaderView;
	ID3D10ShaderResourceView*           envMapSRV;

	CViewpoint viewpoint;
	LoaderEffect * loaderEffect;

	CPolychora * polychora;
	CScreenSpaceFxProcessor ssfxp;

	std::thread * loader;

	HWND windowHandle;

	UINT WindowWidth;
	UINT WindowHeight;


	std::vector<EvoFunctor> ComplexEvolutionFunctions;
	std::vector<EvoFunctor> PlainEvolutionFunctions;
	std::function<void(float,float)> EvolutionFunction;

private:

	void Evolve_ChaoticPeriodicPersistent( float timeAbs,float timeDelta );
	void Evolve_Weierstrass( float timeAbs,float timeDelta );
	void Evolve_Plain( float timeAbs,float timeDelta );

	float plain_acc_time;

	std::vector<std::pair<int,int> > _rotationPlanesShuffled;

	float envRotationSeed;
	float envRotationDir;
	float rotationPhase3d;



public:


	bool UseVSync;
	bool isLoaded;
};


