#include "PolychoraSettings.h"


CPolychoraSettings::CPolychoraSettings(void) : 
	UseFXAA(true), 
	UseComplexRotations(true),
	Polychoron(L"Tesseract"),
	ShowText(true)
{
	MaybeLoad();
}


CPolychoraSettings::~CPolychoraSettings(void)
{
	if(regkey!=nullptr)
		regkey->Close();
}



std::unique_ptr<CRegKey> & CPolychoraSettings::CreateOrGetSettingsKey()
{
	if(regkey == nullptr)
	{
		auto rt = std::unique_ptr<CRegKey>(new CRegKey);

		if(rt->Create(HKEY_CURRENT_USER,L"Software\\Polychora")!=ERROR_SUCCESS)
			return std::unique_ptr<CRegKey>(nullptr);

		regkey = std::move(rt);
	}
		
	return regkey;
}

void CPolychoraSettings::Save()
{
	auto & key = CreateOrGetSettingsKey();

	if(key == nullptr)
		return;

	set(L"UseFXAA",UseFXAA?L"yep":L"no");
	set(L"UseComplexRotations",UseComplexRotations?L"yep":L"no");
	set(L"ShowText",ShowText?L"yep":L"no");
	set(L"Polychoron",Polychoron);
}

void CPolychoraSettings::MaybeLoad()
{
	auto & key = CreateOrGetSettingsKey();

	if(key == nullptr)
		return;

	if(!query(L"UseFXAA").empty())
		UseFXAA = query(L"UseFXAA") == L"yep";

	if(!query(L"ShowText").empty())
		ShowText = query(L"ShowText") == L"yep";
		
	if(!query(L"UseComplexRotations").empty())
		UseComplexRotations = query(L"UseComplexRotations") == L"yep";

	if(!query(L"Polychoron").empty())
		Polychoron = query(L"Polychoron");
}

std::wstring CPolychoraSettings::query( const std::wstring & valueName )
{
	if(nullptr==regkey)
		return L"";

	wchar_t buf[256];
	DWORD len = sizeof(buf);

	if(regkey->QueryStringValue(valueName.c_str(),buf,&len)!=ERROR_SUCCESS)
		return L"";

	return buf;
}

void CPolychoraSettings::set( const std::wstring & valueName,const std::wstring & value )
{
	if(nullptr==regkey)
		return;

	regkey->SetStringValue(valueName.c_str(),value.c_str());
}
