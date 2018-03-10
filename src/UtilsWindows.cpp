#include "MyBCopy_pch.h"
#include "UtilsWindows.h"
#include "Utils.h"

static std::wstring GetValidPathFromReg(HKEY hKey, const wstring& subKey, const wstring& valueName, REGSAM desiredAccess)
{	
	using namespace winreg;
	
	auto pathSubKey = fs::path(subKey);

	wstring parent = pathSubKey.parent_path();
	wstring subKeyName = pathSubKey.filename();
	
		
	RegKey key;

	key.Open(HKEY_LOCAL_MACHINE, parent, KEY_READ | desiredAccess);
	auto subKeys = key.EnumSubKeys();

	auto subKey_it = find_if(subKeys.begin(), subKeys.end(), [&subKeyName](const wstring& e)->bool
	{
		return _wstricmp(e, subKeyName);
	});
	if (subKey_it == subKeys.end())
		return L"";


		
	key.Open(HKEY_LOCAL_MACHINE, subKey , KEY_READ | desiredAccess);
	
	auto values = key.EnumValues();

	auto value_it = find_if(values.begin(), values.end(), [&valueName](const std::pair<std::wstring, DWORD>& v2t)->bool
	{
		return _wstricmp(v2t.first, valueName) && v2t.second == REG_SZ;
	});

	if (value_it == values.end())
		return L"";

	return key.GetStringValue(valueName);
}


std::wstring GetValidPathFromReg32(HKEY hKey, const wstring& subKey, const wstring& valueName)
{
	return GetValidPathFromReg(hKey, subKey, valueName, KEY_WOW64_32KEY);
}



std::wstring GetValidPathFromReg64(HKEY hKey, const wstring& subKey, const wstring& valueName)
{
	return GetValidPathFromReg(hKey, subKey, valueName, KEY_WOW64_64KEY);	
}


std::wstring GetValidPathFromRegCurrentBitness(HKEY hKey, const wstring& subKey, const wstring& valueName)
{
#ifdef _WIN64
	return GetValidPathFromReg64(hKey, subKey, valueName);
#else
	return GetValidPathFromReg32(hKey, subKey, valueName);
#endif
}


std::wstring GetValidPathFromRegInverseBitness(HKEY hKey, const wstring& subKey, const wstring& valueName)
{
#ifdef _WIN64
	return GetValidPathFromReg32(hKey, subKey, valueName);	
#else
	return GetValidPathFromReg64(hKey, subKey, valueName);	
#endif
}



std::wstring GetValidPathFromRegAnyBitness(HKEY hKey, const wstring& subKey, const wstring& valueName)
{
	std::wstring result = GetValidPathFromRegCurrentBitness(hKey, subKey, valueName);
	if (result.empty())
		return GetValidPathFromRegInverseBitness(hKey, subKey, valueName);
	return result;
}
