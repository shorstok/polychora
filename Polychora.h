#pragma once

//#include <d3d10.h>
#include <d3dx10.h>

#include "Viewpoint.h"

#include <vector>
#include <unordered_map>
#include <set>

#include <string>

#include <map>
#include <functional>

#include <string>

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct Vertex4D
{
	Vertex4D(D3DXVECTOR4 vecPos,D3DXVECTOR4 vecNorm) : Pos(vecPos), Norm(vecNorm){}
	Vertex4D(D3DXVECTOR4 vecPos) : Pos(vecPos), Norm(D3DXVECTOR4()){}

	D3DXVECTOR4 Pos;
	D3DXVECTOR4 Norm;
};


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class CEdge
{
public:
	CEdge(const int & idxFrom, const int & idxTo) : from(idxFrom), to(idxTo)	{	}
	
public:

	int from;
	int to;

	CEdge & operator= (const CEdge & other)
	{
		from = other.from;
		to = other.to;

		return *this;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class CFace
{
public:
	CFace(CEdge * edgeA,CEdge * edgeB): a(edgeA), b(edgeB){}
public:
	const CEdge * a;
	const CEdge * b;

};


class CPolychora{
public:
	CPolychora(void);
	~CPolychora(void);

public:

	std::vector<D3DXVECTOR4> vertices;
	std::vector<CEdge> edges;
	std::vector<CFace> faces;

	std::unordered_map<int, std::vector<int>> tails;
	std::unordered_map<int, std::vector<int>> heads;

	void CreateVertexBuffers(ID3D10Effect * fx, ID3D10Device * device);
	void CreateInputLayout( ID3D10Effect * fx, ID3D10Device * device );

	void Render( ID3D10Effect * fx, ID3D10Device * device ,CViewpoint * viewpoint);
	void SoftwareProjectLines(ID3D10EffectTechnique * tech, ID3D10Device * device,CViewpoint * viewpoint);


	void Release();

	static CPolychora * Tesseract();
	static CPolychora * SixteenCell();
	static CPolychora * Pentatope();
	static CPolychora * Icositetrachoron_24cell();

	static CPolychora * Line();
	static CPolychora * Plane();

	float vFlareSpecularAmp;
	float vFaceSpecularAmp;
	float vFaceEnviAmp;

	static std::map<std::wstring, std::function<CPolychora *(void)> > PolychoraGenerators;

	std::wstring Name;

private:

	ID3D10InputLayout* vertexLayout;

	ID3D10Buffer * linesVtxBuf;
	ID3D10Buffer * linesIdxBuf;
	ID3D10Buffer * facesIdxBuf;

	D3DXVECTOR4 vFlareSpecularColor;
	D3DXVECTOR3 vFaceEnviColor;
	D3DXVECTOR4 vFaceSpecularColor;

	void AddEdge(const int & idx1, const int & idx2);
	void CreateEdgesWithLength(float len);

};

