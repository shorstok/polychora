#pragma once

#include "atlbase.h"

#include <memory>
#include <string>

class CPolychoraSettings
{
public:
	CPolychoraSettings(void);
	~CPolychoraSettings(void);
	void Save();
	void MaybeLoad();

public:

	bool UseFXAA;
	bool UseComplexRotations;
	bool ShowText;
	std::wstring Polychoron;

protected:

	std::unique_ptr<CRegKey> & CreateOrGetSettingsKey();
	std::wstring query(const std::wstring & valueName);
	void set(const std::wstring & valueName,const std::wstring & value);
	
	std::unique_ptr<CRegKey> regkey;

};

