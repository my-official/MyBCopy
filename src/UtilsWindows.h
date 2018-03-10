#pragma once




std::wstring GetValidPathFromReg32(HKEY hKey, const wstring& subKey, const wstring& valueName);
std::wstring GetValidPathFromReg64(HKEY hKey, const wstring& subKey, const wstring& valueName);
std::wstring GetValidPathFromRegCurrentBitness(HKEY hKey, const wstring& subKey, const wstring& valueName);
std::wstring GetValidPathFromRegInverseBitness(HKEY hKey, const wstring& subKey, const wstring& valueName);
std::wstring GetValidPathFromRegAnyBitness(HKEY hKey, const wstring& subKey, const wstring& valueName);


