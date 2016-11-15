#pragma once

#include <d3dx10.h>

#include <vector>



class LoaderEffect
{
public:
	LoaderEffect(ID3D10Device * device,int initWidth,int initHeight);
	void Render( ID3D10Device * device, ID3D10RenderTargetView * renderTarget, float time );
	~LoaderEffect(void);

protected:

	ID3D10Buffer * fsQuadBuf;
	ID3D10InputLayout* vertex3Layout;
	ID3D10Effect* effect;

	int nVtx;

	std::vector<IUnknown*> resources;

	bool IsValid;

};

