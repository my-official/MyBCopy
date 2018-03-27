#include "MyBCopy_pch.h"
#include "Tool.h"
#include "Exceptions.h"
#include "Utils.h"
#include "UtilsMyBCopy.h"

void SaveInclusionListFile(const list<wstring>& filesForArchive, const wstring& outfile)
{
	ofstream outstream(outfile);

	if (!outstream)
		throw EXCEPTION(ErrnoException());

	for (auto& file : filesForArchive)
	{
		//outstream << L"\"" << file << L"\"" << endl;
		outstream << wstr2utf8(file) << endl;
	}
	//outstream.close();
}


Tool::Tool(const wstring& executablePath, const wstring& password)
	: m_ExecutablePath(executablePath)
	, m_Password(password)
{

}



void Tool::DecryptAndDecompress(const wstring& inFile, const wstring& outDir)
{
	Decrypt(inFile, outDir);
}

void ArchiverRar::CompressAndEncrypt(const list<wstring>& filesForArchive, const wstring& tmpWorkingDir, const wstring& outFile)
{	
	wstring inclusionListFile(tmpWorkingDir + L"\\" + L"inclusion.txt");

	SaveInclusionListFile(filesForArchive, inclusionListFile);
	ExecuteProgram_except(m_ExecutablePath, L"a -hp\"" + m_Password + L"\" -m5 -ep3 -r -s -t -y -- \"" + outFile + L"\" @\"" + inclusionListFile + L"\"");

	assert(fs::exists(inclusionListFile));
	fs::remove(inclusionListFile);
}

void ArchiverRar::Encrypt(const wstring& inFile, const wstring& outFile)
{	
	ExecuteProgram_except(m_ExecutablePath, L"a -hp\"" + m_Password + L"\" -m0 -ep -t -y -- \"" + outFile + L"\" \"" + inFile + L"\"");
}

void ArchiverRar::Decrypt(const wstring& inFile, const wstring& outDir)
{
	int result = ExecuteProgram(m_ExecutablePath, L"x -p\"" + m_Password + L"\" -o- -y -- \"" + inFile + L"\" \"" + outDir + L"\\\"");
	if (result != 0 && result != 10)
	{
		throw EXCEPTION(BaseException());
	}
}


std::wstring ArchiverRar::GetSystemDefaultExecutablePath() const
{
	static const std::wstring path = GetSystemRarExeFilePath();
	return path;
}

std::wstring ArchiverRar::GetFileTypeExt() const
{
	return L".rar";
}

EBackupError ArchiverRar::ErrorCode_NoExecutable() const
{
	return EBackupError::Settings_NoSystemRar;
}

void Archiver7z::CompressAndEncrypt(const list<wstring>& filesForArchive, const wstring& tmpWorkingDir, const wstring& outFile)
{	
	wstring inclusionListFile(tmpWorkingDir + L"\\" + L"inclusion.txt");
	
	SaveInclusionListFile(filesForArchive, inclusionListFile);

	ExecuteProgram_except(m_ExecutablePath, L"a -p\"" + m_Password + L"\" -mhe=on -mx=9 -bso0 -bsp0 -ssw -spf -y \"" + outFile + L"\" @\"" + inclusionListFile + L"\"");
	ExecuteProgram_except(m_ExecutablePath, L"t -p\"" + m_Password + L"\" -bso0 -bsp0 -y \"" + outFile + L"\" *");

	assert(fs::exists(inclusionListFile));
	fs::remove(inclusionListFile);	
}

void Archiver7z::Encrypt(const wstring& inFile, const wstring& outFile)
{
	ExecuteProgram_except(m_ExecutablePath, L"a -p\"" + m_Password + L"\" -mhe=on -mx=0 -bso0 -bsp0 -ssw -y \"" + outFile + L"\" \"" + inFile + L"\"");
	ExecuteProgram_except(m_ExecutablePath, L"t -p\"" + m_Password + L"\" -bso0 -bsp0 -y \"" + outFile + L"\" *");
}

void Archiver7z::Decrypt(const wstring& inFile, const wstring& outDir)
{	
	ExecuteProgram_except(m_ExecutablePath, L"x -p\"" + m_Password + L"\" -aos -o\"" + outDir + L"\" -y \"" + inFile + L"\"");
}

std::wstring Archiver7z::GetSystemDefaultExecutablePath() const
{
	static const std::wstring path = GetSystem7zExeFilePath();
	return path;	
}

std::wstring Archiver7z::GetFileTypeExt() const
{
	return L".7z";
}

EBackupError Archiver7z::ErrorCode_NoExecutable() const
{
	return EBackupError::Settings_NoSystem7z;
}

