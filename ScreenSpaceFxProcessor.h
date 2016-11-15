#pragma once

//#include <d3d10.h>
#include <d3dx10.h>

#include <vector>
#include <unordered_map>
#include <set>
#include <memory>

#include "Viewpoint.h"

#include "FontRenderer.h"

struct Vertex3D
{
	Vertex3D(D3DXVECTOR4 vecPos,D3DXVECTOR2 vecNorm) : Pos(vecPos), Tex(vecNorm){}
	Vertex3D(D3DXVECTOR4 vecPos) : Pos(vecPos), Tex(D3DXVECTOR2()){}

	D3DXVECTOR4 Pos;
	D3DXVECTOR2 Tex;
};

class CScreenSpaceFxProcessor
{
public:
	CScreenSpaceFxProcessor(void);
	~CScreenSpaceFxProcessor(void);

	void Init( ID3D10Device * device, ID3D10Texture2D * baseRenderTargetTex,ID3D10RenderTargetView*sourceRenderTargetView, const D3D10_VIEWPORT & srcVptDesc );
	void Release();
	void Process(ID3D10Device * device,ID3D10ShaderResourceView * envmap, const CViewpoint & viewpoint);
	void ProcessEnv(ID3D10Device * device,ID3D10ShaderResourceView * envmap, const CViewpoint & viewpoint);

	bool UseFXAA;
	bool ShowText;
	bool debugShowBloomOnly;
	bool debugShowBrightpassOnly;

	std::wstring headerText;
	std::wstring footerText;

private:


	void DrawFsQuad(ID3D10Device * device,ID3D10ShaderResourceView * srcTexView, LPCSTR techName );
	void processFxaa(ID3D10Device * device);
	void renderText(ID3D10Device * device);

	ID3D10RenderTargetView*             srcRenderTargetView;

	ID3D10RenderTargetView*             offscreenRenderTargetView;
	ID3D10Texture2D*					offscreenTex;
	ID3D10ShaderResourceView*			offscreenTexView;

	ID3D10RenderTargetView*             offscreenSmallRenderTargetView[2];
	ID3D10Texture2D*					offscreenSmallTex[2];
	ID3D10ShaderResourceView*           offscSmallTexView[2];
	ID3D10Effect*						fxBloom;
	ID3D10Effect*						fxFxaa;
	ID3D10ShaderResourceView*           srcTexView;

	ID3D10Buffer * fsQuadBuf;
	ID3D10InputLayout* vertex3Layout;

	D3D10_VIEWPORT srcViewportDesc;
	D3D10_VIEWPORT bloomViewportDesc;

	std::unique_ptr<CFontRenderer> fontRenderer;

	float fBrightnessTolerance;

	int fxBufWidth;
	int fxBufHeight;

};

