#include "Polychora.h"

std::map<std::wstring, std::function<CPolychora *(void)> >  create_generators()
{
	std::map<std::wstring, std::function<CPolychora *(void)> >  rt;

	rt[L"Tesseract"] = std::bind(&CPolychora::Tesseract);
	rt[L"Hexadecachoron"] = std::bind(&CPolychora::SixteenCell);
	rt[L"Pentatope"] = std::bind(&CPolychora::Pentatope);
	rt[L"Icositetrachoron"] = std::bind(&CPolychora::Icositetrachoron_24cell);

	return rt;
}


std::map<std::wstring, std::function<CPolychora *(void)> > CPolychora::PolychoraGenerators = create_generators();

CPolychora::CPolychora(void) : 
	vFlareSpecularColor(D3DXVECTOR4(0.9,0.4,0,1)), 
	vFaceEnviColor(D3DXVECTOR3(0.1,0.2,0.4)),
	vFaceSpecularColor(D3DXVECTOR4(0.2,0.1,0,0.3)),
	vFaceSpecularAmp(1),
	vFlareSpecularAmp(1),
	vFaceEnviAmp(1),
	linesVtxBuf(nullptr),
	facesIdxBuf(nullptr),
	linesIdxBuf(nullptr),
	vertexLayout(nullptr)
{
	
}


CPolychora::~CPolychora(void)
{
}


CPolychora * CPolychora::Plane()
{
	CPolychora * ret = new CPolychora();

	ret->vertices.push_back(D3DXVECTOR4(0,-1,-1,0));
	ret->vertices.push_back(D3DXVECTOR4(0,1,-1,0));
	ret->vertices.push_back(D3DXVECTOR4(0,1,1,0));
	ret->vertices.push_back(D3DXVECTOR4(0,-1,1,0));

	ret->AddEdge(0, 1 );
	ret->AddEdge(1, 2 );
	ret->AddEdge(2, 3 );
	ret->AddEdge(3, 0 );

	return ret;
}

CPolychora * CPolychora::Line()
{
	CPolychora * ret = new CPolychora();

	ret->vertices.push_back(D3DXVECTOR4(-1,-1,-1,-1));
	ret->vertices.push_back(D3DXVECTOR4(1,1,1,1));

	ret->AddEdge(0, 1 );

	return ret;
}


//http://en.wikipedia.org/wiki/24-cell
CPolychora * CPolychora::Icositetrachoron_24cell()
{
	CPolychora * ret = new CPolychora();

	for (int i = 0; i < 4; i++)
	{
		D3DXVECTOR4 v = D3DXVECTOR4(0,0,0,0);

		((float*)v)[i]=1;
		ret->vertices.push_back(v);
		((float*)v)[i]=-1;
		ret->vertices.push_back(v);
	}

	for (int ix=0;ix!=2;++ix)
	{
		for (int iy = 0; iy < 2; iy++)
		{
			for(int iz=0;iz!=2;++iz)
			{
				for (int iw = 0; iw < 2; iw++)
				{
					D3DXVECTOR4 v = D3DXVECTOR4(ix==0?1:-1,iy==0?1:-1,iz==0?1:-1,iw==0?1:-1)/2;
					ret->vertices.push_back(v);
				}
			}
		}
	}

	ret->CreateEdgesWithLength(1);

	//to match others visual appearance
	for(auto & v : ret->vertices)
		v*=2;

	ret->Name = L"Icositetrachoron";

	return ret;
}





//http://en.wikipedia.org/wiki/5-cell
CPolychora * CPolychora::Pentatope()
{
	CPolychora * ret = new CPolychora();

	float isq10 = 1/sqrt(10);
	float isq6 = 1/sqrt(6);
	float isq3 = 1/sqrt(3);


	ret->vertices.push_back(D3DXVECTOR4(isq10,isq6,isq3,1));
	ret->vertices.push_back(D3DXVECTOR4(isq10,isq6,isq3,-1));
	ret->vertices.push_back(D3DXVECTOR4(isq10,isq6,-2*isq3,0));
	ret->vertices.push_back(D3DXVECTOR4(isq10,-sqrt(3.0/2.0),0,0));
	ret->vertices.push_back(D3DXVECTOR4(-2.0*sqrt(2.0/5),0,0,0));

	ret->CreateEdgesWithLength(2);

	//to match others visual appearance
	for(auto & v : ret->vertices)
		v*=1.6;

	ret->Name = L"Pentatope";

	return ret;
}


//4d-equiv of octahedron
CPolychora * CPolychora::SixteenCell()
{
	CPolychora * ret = new CPolychora();

	float sqrt2 = sqrt(2);

	for (int i = 0; i < 4; i++)
	{
		D3DXVECTOR4 v = D3DXVECTOR4(0,0,0,0);

		((float*)v)[i]=1;
		ret->vertices.push_back(v);
		((float*)v)[i]=-1;
		ret->vertices.push_back(v);
	}

	ret->CreateEdgesWithLength(sqrt2);

	//to match others visual appearance
	for(auto & v : ret->vertices)
		v*=1.6;

	ret->Name = L"Hexadecachoron";

	return ret;
}

CPolychora * CPolychora::Tesseract()
{
	CPolychora * ret = new CPolychora();

	//VertexList  16:
	ret->vertices.push_back(D3DXVECTOR4(-1,-1,-1,-1));
	ret->vertices.push_back(D3DXVECTOR4(1,-1,-1,-1));
	ret->vertices.push_back(D3DXVECTOR4(1, 1,-1,-1));
	ret->vertices.push_back(D3DXVECTOR4(-1, 1,-1,-1));
	ret->vertices.push_back(D3DXVECTOR4(-1,-1, 1,-1));
	ret->vertices.push_back(D3DXVECTOR4(1,-1, 1,-1));
	ret->vertices.push_back(D3DXVECTOR4(1, 1, 1,-1));
	ret->vertices.push_back(D3DXVECTOR4(-1, 1, 1,-1));
	ret->vertices.push_back(D3DXVECTOR4(-1,-1,-1, 1));
	ret->vertices.push_back(D3DXVECTOR4(1,-1,-1, 1));
	ret->vertices.push_back(D3DXVECTOR4(1, 1,-1, 1));
	ret->vertices.push_back(D3DXVECTOR4(-1, 1,-1, 1));
	ret->vertices.push_back(D3DXVECTOR4(-1,-1, 1, 1));
	ret->vertices.push_back(D3DXVECTOR4(1,-1, 1, 1));
	ret->vertices.push_back(D3DXVECTOR4(1, 1, 1, 1));
	ret->vertices.push_back(D3DXVECTOR4(-1, 1, 1, 1));


	ret->AddEdge(0, 1);
	ret->AddEdge(1, 2 );
	ret->AddEdge(2, 3 );
	ret->AddEdge(3, 0 );
	ret->AddEdge(4, 5 );
	ret->AddEdge(5, 6 );
	ret->AddEdge(6, 7 );
	ret->AddEdge(7, 4 );
	ret->AddEdge(0, 4 );
	ret->AddEdge(1, 5 );
	ret->AddEdge(2, 6 );
	ret->AddEdge(3, 7 );
	ret->AddEdge(8, 9 );
	ret->AddEdge(9,10 );
	ret->AddEdge(10,11 );
	ret->AddEdge(11, 8 );
	ret->AddEdge(12,13 );
	ret->AddEdge(13,14 );
	ret->AddEdge(14,15 );
	ret->AddEdge(15,12 );
	ret->AddEdge(8,12 );
	ret->AddEdge(9,13 );
	ret->AddEdge(10,14 );
	ret->AddEdge(11,15 );
	ret->AddEdge(0, 8 );
	ret->AddEdge(1, 9 );
	ret->AddEdge(2,10 );
	ret->AddEdge(3,11 );
	ret->AddEdge(4,12 );
	ret->AddEdge(5,13 );
	ret->AddEdge(6,14 );
	ret->AddEdge(7,15 );

	ret->Name = L"Tesseract (octachoron)";

	return ret;
}

void CPolychora::AddEdge( const int & idx1, const int & idx2 )
{
	edges.push_back(CEdge(idx1,idx2));

	if(tails.find(idx1) == tails.end())
	{
		const int v[] = {idx2};
		tails[idx1] = std::vector<int>(v,v + 1);
	}
	else
		tails[idx1].push_back(idx2);

	if(heads.find(idx2) == heads.end())
	{
		const int v[] = {idx1};
		heads[idx2] = std::vector<int>(v,v + 1);
	}
	else
		heads[idx2].push_back(idx1);
}




void CPolychora::CreateVertexBuffers(ID3D10Effect * fx, ID3D10Device * device)
{
	std::vector<Vertex4D> vtxData;
	std::vector<DWORD> FaceIdxData;
	std::vector<DWORD> idxData;

	CreateInputLayout(fx, device);

	for (auto & v : vertices)
	{
		auto vnormal = D3DXVECTOR4(v);

		D3DXVec4Normalize(&vnormal,&vnormal);

		vtxData.push_back(Vertex4D(v,vnormal));
	}

	for (auto & e : edges)
	{
		idxData.push_back(e.from);
		idxData.push_back(e.to);
	}
	
	std::set<int> tri;

	for (auto & e : edges)
	{
		for (auto & e2 : edges)
		{
			if(e.to	 == e2.to && e.from == e2.from)
				continue;

			if(e.to	 != e2.to && e.to == e2.from && e.from !=e2.from && e.from !=e2.to)
				continue;

		
			if(e.from == e2.to || e.from == e2.from ||e.to == e2.to || e.to == e2.from )
			{
				tri.clear();

				tri.insert(e.from);
				tri.insert(e.to);
				tri.insert(e2.from);
				tri.insert(e2.to);

				for (auto & i : tri)
					FaceIdxData.push_back(i);

			}
		}

	}

	/************************************************************************/
	/* Create vertex & index buffers & load them with data                  */
	/************************************************************************/

	///vtx
	D3D10_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc,sizeof(bufferDesc));
	bufferDesc.Usage = D3D10_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof( Vertex4D ) * vtxData.size();
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData,sizeof(InitData));
	InitData.pSysMem = &vtxData[0];
	auto hr = device->CreateBuffer( &bufferDesc, &InitData, &linesVtxBuf );
	if( FAILED( hr ) )
		return;

	//idx
	bufferDesc.Usage = D3D10_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof( DWORD ) * idxData.size();
	bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	InitData.pSysMem = &idxData[0];
	hr = device->CreateBuffer( &bufferDesc, &InitData, &linesIdxBuf );
	
	if( FAILED( hr ) )
		return;

	if(!FaceIdxData.empty())
	{
		ZeroMemory(&bufferDesc,sizeof(bufferDesc));
		ZeroMemory(&InitData,sizeof(InitData));

		//idx
		bufferDesc.Usage = D3D10_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof( DWORD ) * FaceIdxData.size();
		bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		InitData.pSysMem = &FaceIdxData[0];
		hr = device->CreateBuffer( &bufferDesc, &InitData, &facesIdxBuf );

		if( FAILED( hr ) )
			return;
	}
	else
	{
		facesIdxBuf =  nullptr;
	}


}

void CPolychora::Render( ID3D10Effect * fx, ID3D10Device * device ,CViewpoint * viewpoint)
{
	ID3D10EffectTechnique * lineTech = fx->GetTechniqueByName("RenderCubes");
	ID3D10EffectTechnique * faceTech = fx->GetTechniqueByName("RenderFaces");

	if(!viewpoint->Hardware4DProjection)
		SoftwareProjectLines(lineTech,device,viewpoint);

	fx->GetVariableByName("vFlareSpecularColor")->AsVector()->SetFloatVector(vFlareSpecularColor*vFlareSpecularAmp);
	fx->GetVariableByName("vFaceSpecularColor")->AsVector()->SetFloatVector(vFaceSpecularColor*vFaceSpecularAmp);
	fx->GetVariableByName("vFaceEnviColor")->AsVector()->SetFloatVector(vFaceEnviColor*vFaceEnviAmp);

	// Set the input layout
	device->IASetInputLayout( vertexLayout );

	D3D10_BUFFER_DESC desc;
	// Set index buffer
	device->IASetIndexBuffer( linesIdxBuf, DXGI_FORMAT_R32_UINT, 0 );

	// Set vertex buffer
	UINT stride = sizeof( Vertex4D );
	UINT offset = 0;
	device->IASetVertexBuffers( 0, 1, &linesVtxBuf, &stride, &offset );

	// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

	D3D10_TECHNIQUE_DESC techDesc;
	
	linesIdxBuf->GetDesc(&desc);

	// Render
	lineTech->GetDesc( &techDesc );

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		lineTech->GetPassByIndex( p )->Apply( 0 );
		device->DrawIndexed(desc.ByteWidth / sizeof(DWORD), 0, 0 );
	}

	if(facesIdxBuf!=nullptr)
	{
		// Set primitive topology
		device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// Set index buffer
		device->IASetIndexBuffer( facesIdxBuf, DXGI_FORMAT_R32_UINT, 0 );

		facesIdxBuf->GetDesc(&desc);
		// Render
		faceTech->GetDesc( &techDesc );
		for( UINT p = 0; p < techDesc.Passes; ++p )
		{
			faceTech->GetPassByIndex( p )->Apply( 0 );
			device->DrawIndexed(desc.ByteWidth / sizeof(DWORD), 0, 0 );
		}
	}

}

void CPolychora::Release()
{
	if( linesVtxBuf ) linesVtxBuf->Release();
	if( facesIdxBuf ) facesIdxBuf->Release();
	if( linesIdxBuf ) linesIdxBuf->Release();
	if( vertexLayout ) vertexLayout->Release();
}

void CPolychora::CreateInputLayout( ID3D10Effect * fx, ID3D10Device * device )
{
	// Define the input layout 
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4*4, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	fx->GetTechniqueByIndex(0)->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	auto hr = device->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout );
	if( FAILED( hr ) )
		return;
}

void CPolychora::SoftwareProjectLines( ID3D10EffectTechnique * tech, ID3D10Device * device ,CViewpoint * viewpoint)
{
	Vertex4D * vtx;

	if(SUCCEEDED(linesVtxBuf->Map(D3D10_MAP_WRITE_DISCARD,0,(void**)&vtx)))
	{
		for (int c=0;c!=vertices.size();++c)
		{
			vtx[c].Pos = viewpoint->ProjectVertexTo3D(vertices[c]);
			vtx[c].Norm =viewpoint->ProjectVertexTo3D(vertices[c]);
			vtx[c].Norm = vtx[c].Norm / D3DXVec4Length(&vtx[c].Norm);
		}

		linesVtxBuf->Unmap();
	}
}

void CPolychora::CreateEdgesWithLength( float len )
{
	for (int i=0;i!=vertices.size();++i)
	{
		for (int j=i+1;j<vertices.size();++j)
		{
			D3DXVECTOR4 d = vertices[i] - vertices[j];

			if(fabs(D3DXVec4Length(&d) - len)<1e-3)
				AddEdge(i, j);
		}
	}
}



