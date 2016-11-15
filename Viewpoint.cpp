#include "Viewpoint.h"


void CViewpoint::SetProjections( ID3D10Effect * effect, int width, int height )
{
	/*
	=====
	4D equivalent of 'view' matrix
	=====
	*/

	D3DXVECTOR2 dim = D3DXVECTOR2(width,height);

	/* Calculate Vd, the 4th coordinate basis vector and line-of-sight. */

	Vd = To4 - From4;
	D3DXVec4Normalize(&Vd,&Vd);

	/* Calculate Va, the X-axis basis vector. */
	D3DXVec4Normalize(&Va,D3DXVec4Cross(&Va,&Up4,&Over4,&Vd));

	/* Calculate Vb, the perpendicularized Up vector. */
	D3DXVec4Normalize(&Vb,D3DXVec4Cross(&Vb,&Over4,&Vd,&Va));

	/* Calculate Vc, the perpendicularized Over vector*/
	D3DXVec4Normalize(&Vc,D3DXVec4Cross(&Vc,&Vd,&Va,&Vb));

	effect->GetVariableByName("Va")->AsVector()->SetFloatVector((float*)Va);
	effect->GetVariableByName("Vb")->AsVector()->SetFloatVector((float*)Vb);
	effect->GetVariableByName("Vc")->AsVector()->SetFloatVector((float*)Vc);
	effect->GetVariableByName("Vd")->AsVector()->SetFloatVector((float*)Vd);
	effect->GetVariableByName("From4")->AsVector()->SetFloatVector((float*)From4);

	effect->GetVariableByName("RenderTargetSize")->AsVector()->SetFloatVector((float*)dim);

	effect->GetVariableByName("ViewangleTanInv")->AsScalar()->SetFloat(1/tan(Viewangle/2));
	
	effect->GetVariableByName("bProject4D")->AsScalar()->SetBool(Hardware4DProjection);

	/*
	=====
	Classic 3D projection stuff
	=====
	*/

	

	// Initialize the projection matrix
	D3DXMatrixPerspectiveFovLH( &Projection3d, Viewangle, width / (float)height, 0.1f, 100.0f );

	D3DXMatrixLookAtLH( &View3d, &From3, &To3, &Up3);
	effect->GetVariableByName( "CameraPosition" )->AsVector()->SetFloatVector(( float* )From3);
	
	effect->GetVariableByName( "World" )->AsMatrix()->SetMatrix((float*)&World3d);
	effect->GetVariableByName( "View" )->AsMatrix()->SetMatrix((float*)&View3d);
	effect->GetVariableByName( "Projection" )->AsMatrix()->SetMatrix((float*)&Projection3d);
}

D3DXVECTOR4 CViewpoint::ProjectVertexTo3D( D3DXVECTOR4 vertex )
{
	D3DXVECTOR4 t = vertex - From4;

	float pc =  1 / tan (Viewangle/2);

	float rTmp = pc / D3DXVec4Dot(&t,&Vd);

	return D3DXVECTOR4(D3DXVec4Dot(&t,&Va)*rTmp,D3DXVec4Dot(&t,&Vb)*rTmp,D3DXVec4Dot(&t,&Vc)*rTmp,1);
}

void CViewpoint::Rotate4DBasis( double angle, int plane_coord_index1,int plane_coord_index2 )
{
	double t1,t2;
	double cos_a = cos(angle);
	double sin_a = sin(angle);

	/* Rotate the from-vector. */

	t1 = cos_a * (((float*)From4)[plane_coord_index1]-((float*)To4)[plane_coord_index1])  +  sin_a * (((float*)From4)[plane_coord_index2]-((float*)To4)[plane_coord_index2]);
	t2 = cos_a * (((float*)From4)[plane_coord_index2]-((float*)To4)[plane_coord_index2])  -  sin_a * (((float*)From4)[plane_coord_index1]-((float*)To4)[plane_coord_index1]);
	((float*)From4)[plane_coord_index1] = t1 + ((float*)To4)[plane_coord_index1];
	((float*)From4)[plane_coord_index2] = t2 + ((float*)To4)[plane_coord_index2];

	/* Rotate the Up Vector. */

	t1 = cos_a * ((float*)Up4)[plane_coord_index1]  +  sin_a * ((float*)Up4)[plane_coord_index2];
	t2 = cos_a * ((float*)Up4)[plane_coord_index2]  -  sin_a * ((float*)Up4)[plane_coord_index1];
	((float*)Up4)[plane_coord_index1] = t1;
	((float*)Up4)[plane_coord_index2] = t2;

	/* Rotate the Over Vector */

	t1 = cos_a * ((float*)Over4)[plane_coord_index1]  +  sin_a * ((float*)Over4)[plane_coord_index2];
	t2 = cos_a * ((float*)Over4)[plane_coord_index2]  -  sin_a * ((float*)Over4)[plane_coord_index1];
	((float*)Over4)[plane_coord_index1] = t1;
	((float*)Over4)[plane_coord_index2] = t2;
}

void CViewpoint::Rotate4DBasisAbsolute(std::vector<RotationToken4D> rotations )
{
	From4 = From4Original;
	Over4 = Over4Original;
	Up4 = Up4Original;

	for (auto & rot : rotations)
		Rotate4DBasis(rot.angle,rot.plane1,rot.plane2);
	
}
