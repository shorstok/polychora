#pragma once

//#include <d3d10.h>
#include <d3dx10.h>

#include <vector>


class CViewpoint
{
public:
	CViewpoint(void) : 
		Over4Original(D3DXVECTOR4(0,0,1,0)),
		Over4(Over4Original),
		Up4Original(D3DXVECTOR4(0,1,0,0)),
		Up4(Up4Original),
		From4Original(D3DXVECTOR4(2.5f,0,0,0)),
		From4(From4Original),
		To4(D3DXVECTOR4(0,0,0,0)),

		From3(D3DXVECTOR3(9,0,3)),
		To3(D3DXVECTOR3(0,0,0)),
		Up3(D3DXVECTOR3(0,1,0)),

		Viewangle(D3DX_PI/4),

		Hardware4DProjection(true)
	{
		// Initialize the world matrix
		D3DXMatrixIdentity( &World3d );
	}

	struct RotationToken4D
	{
		RotationToken4D(double _angle,int _p1,int _p2) : angle(_angle),plane1(_p1),plane2(_p2){}
		double angle;
		int plane1;
		int plane2;
	};
	
	void SetProjections(ID3D10Effect * effect, int width, int height );
	D3DXVECTOR4 ProjectVertexTo3D( D3DXVECTOR4 vertex );

	void Rotate4DBasis( double angle, int plane_coord_index1,int plane_coord_index2 );
	void Rotate4DBasisAbsolute( std::vector<RotationToken4D> rotations );
		
public:

	const D3DXVECTOR4 From4Original;
	const D3DXVECTOR4 Over4Original;
	const D3DXVECTOR4 Up4Original;

	//4d viewpoints for projecting 4D onto 3D
	D3DXVECTOR4 From4;
	D3DXVECTOR4 To4;
	//Four-dimensional equivalent of 'up', one more vector. 
	//e.g. in 2D you can specify 'from' & 'to' vectors to fully describe view point (camera), in 3D you must add one more vector - 'up' direction, in 4D there are 2 such vectors
	D3DXVECTOR4 Over4;	
	D3DXVECTOR4 Up4;

	//3d viewpoints for classic 3d->2d projection & rasterization
	D3DXVECTOR3 From3;
	D3DXVECTOR3 To3;
	D3DXVECTOR3 Up3;

	D3DXMATRIX World3d;
	D3DXMATRIX View3d;
	D3DXMATRIX Projection3d;

	//4D equivalent of 'view' matrix
	D3DXVECTOR4 Va,Vb,Vc,Vd;


	//Common

	bool Hardware4DProjection;
	float Viewangle;
};

