#pragma once

#include <d3d10.h>
#include <d3dx10.h>

#include <windows.h>
#include "resource.h"

#include <atlbase.h>

#include <string>

class CFontRenderer
{
public:
	CFontRenderer(LPWSTR fontAtlasResource, ID3D10Device * device);
	~CFontRenderer(void);

	D3DXVECTOR2 RenderString(ID3D10Device * device,std::wstring text, D3DXVECTOR2 pos, const D3DXVECTOR4 & color);
	D3DXVECTOR2 MeasureString(std::wstring text);

protected:

	D3DXVECTOR2 RenderGlyph( ID3D10Device * device,const D3DXVECTOR2 & pos, wchar_t glyph, D3DXVECTOR4 color);

	CComPtr<ID3D10ShaderResourceView> fontAtlas;
	CComPtr<ID3D10Effect>			  fxGlyph;

	CComPtr<ID3D10Buffer>			  fsGlyVtxBuf;
	CComPtr<ID3D10InputLayout>	      vertex3Layout;

	D3DXVECTOR2 fontAtlasSize;
};

