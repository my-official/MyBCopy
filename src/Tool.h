#pragma once
#include "Reserver.h"


class MyBCopy_API Tool
{
public:
	Tool(const wstring& executablePath, const wstring& password);
	virtual ~Tool() = default;
		
	virtual void CompressAndEncrypt(const list<wstring>& filesForArchive, const wstring& tmpWorkingDir, const wstring& outFile) = 0;	
	virtual void DecryptAndDecompress(const wstring& inFile, const wstring& outDir);
	virtual void Encrypt(const wstring& inFile, const wstring& outFile) = 0;
	virtual void Decrypt(const wstring& inFile, const wstring& outDir) = 0;

	virtual wstring GetSystemDefaultExecutablePath() const = 0;
	virtual wstring GetFileTypeExt() const = 0;
	virtual EBackupError ErrorCode_NoExecutable() const = 0;

	wstring m_ExecutablePath;
	wstring m_Password;
protected:
	
};



class MyBCopy_API ArchiverRar : public Tool
{
public:	
	using Tool::Tool;
	virtual void CompressAndEncrypt(const list<wstring>& filesForArchive, const wstring& tmpWorkingDir, const wstring& outFile) override;
	virtual void Encrypt(const wstring& inFile, const wstring& outFile) override;
	virtual void Decrypt(const wstring& inFile, const wstring& outDir) override;
	virtual wstring GetSystemDefaultExecutablePath() const override;
	virtual wstring GetFileTypeExt() const override;
	virtual EBackupError ErrorCode_NoExecutable() const override;
};

class MyBCopy_API Archiver7z : public Tool
{
public:
	using Tool::Tool;
	virtual void CompressAndEncrypt(const list<wstring>& filesForArchive, const wstring& tmpWorkingDir, const wstring& outFile) override;
	virtual void Encrypt(const wstring& inFile, const wstring& outFile) override;
	virtual void Decrypt(const wstring& inFile, const wstring& outDir) override;
	virtual wstring GetSystemDefaultExecutablePath() const override;
	virtual wstring GetFileTypeExt() const override;
	virtual EBackupError ErrorCode_NoExecutable() const override;
};