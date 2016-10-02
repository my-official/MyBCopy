#include "stdafx.h"
#include "Utils.h"
#include "Storage.h"



std::wstring wildcard2regex(const wstring& wildcard)
{
	std::wstring result = wildcard;

	auto pos = result.find(L'.');

	while (pos != wstring::npos)
	{
		result.replace(pos, 1, L"\\.");
		pos = result.find(L'.', pos + 2);
	}	

	pos = result.find(L'*');

	while (pos != wstring::npos)
	{
		result.replace(pos, 1, L".*");
		pos = result.find(L'*', pos + 2);
	}

	return result;
}

static std::wstring GetThisProgramExeDirPathStatic()
{
	vector<TCHAR> buffer(MAX_PATH, 0);

	if (GetModuleFileName(nullptr, buffer.data(), buffer.size()) == 0)
		throw EXCEPTION(WinException());

	return fs::path(buffer.data()).parent_path();
}

std::wstring GetThisProgramExeDirPath()
{
	static std::wstring path = GetThisProgramExeDirPathStatic();	
	return path;
}

int FileCmp(const wstring& file1, const wstring& file2)
{	
	auto file1Size = fs::file_size(file1);
	auto file2Size = fs::file_size(file2);

	if (file1Size != file2Size)
	{
		return file1Size < file2Size ? -1 : 1;
	}
		
	ifstream f1, f2;
	f1.exceptions(wifstream::failbit | wifstream::badbit);
	f2.exceptions(wifstream::failbit | wifstream::badbit);
	f1.open(file1, ios_base::binary);
	f2.open(file2, ios_base::binary);

	const int BUFFERSIZE = 4 * 1024 * 1024;
	vector<char> buffer1(BUFFERSIZE);
	vector<char> buffer2(BUFFERSIZE);

	while (file1Size)
	{
		auto count = min<decltype(file1Size)>(BUFFERSIZE, file1Size);
		f1.read( &buffer1[0], count);
		f2.read( &buffer2[0], count);

		int result = memcmp(&buffer1[0], &buffer2[0], size_t(count));
		if (result != 0)
			return result;

		file1Size -= count;
	}
	return 0;
}

std::wstring str2wstr(const string& message)
{
	auto messageLength = MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, NULL, 0) - 1;

	assert(messageLength && messageLength == message.length());

	auto prevErrorMessageLength = message.length();
	wstring wmessage(prevErrorMessageLength + messageLength + 1, L'\n');

	wchar_t* data = &wmessage[prevErrorMessageLength];

	int result = MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, data, messageLength + 1);

	assert(result);

	return move(wmessage);
}


void WriteToConsole(const wstring& message)
{
	LockableWCout lockable = LockableWCout::Instance();
	auto& lwcout = lockable.Get();
	lwcout << message << endl;
}


void WriteToConsole(const string& message)
{	
	LockableWCout lockable = LockableWCout::Instance();
	auto& lwcout = lockable.Get();
	lwcout << str2wstr(message) << endl;
}

void RunCMD(const wstring& cmd)
{
	WriteToConsole(cmd);

	auto returnCode = _wsystem(cmd.c_str());

	if (returnCode != 0)
	{
		WriteToConsole(wstring(L"Return Code is ") + to_wstring(returnCode));
		throw EXCEPTION(ErrnoException());
	}
}



void AddPath(const wstring& newpath)
{
	wchar_t* path = nullptr;

	if (_wdupenv_s(&path, nullptr, L"PATH") != 0)
		throw EXCEPTION(ErrnoException());

	if (!path)
		throw EXCEPTION(ErrnoException());


	if (_wputenv_s(L"PATH", (wstring(path) + L";" + newpath).c_str()) != 0)
		throw EXCEPTION(ErrnoException());

	free(path);
}

void CopyFileAndPrint(const wstring& src, const wstring& dest, bool print /*= true*/)
{
	auto parentPath = fs::path(dest).parent_path();

	if (!fs::exists(parentPath))
	{
		if (!fs::create_directories(parentPath))
			throw EXCEPTION(WinException());
	}

	assert(fs::exists(src));
	assert(!fs::exists(dest));

	fs::copy_file(src, dest);

	if (print)
		WriteToConsole(L"Copied " + src + L" to " + dest);
}

void RemoveFileAndPrint(const wstring& file)
{
	if (!fs::exists(file))
		return;

	if (fs::is_directory(file))
	{
		fs::remove_all(file);
	}
	else
	{
		fs::remove(file);
	}

	WriteToConsole(L"Removed " + file);
}

void _force_remove_all(const fs::path& path)
{	
	for (auto p : fs::recursive_directory_iterator(path))
	{
		fs::permissions(p, fs::perms::add_perms | fs::perms::all);
	}
	remove_all(path);
}

LockableWCout::LockableWCout(mutex& mtx) : m_Lock(mtx)
{

}

LockableWCout LockableWCout::Instance()
{
	static mutex mtx;
	return LockableWCout(mtx);
}

decltype(wcout)& LockableWCout::Get()
{
	return wcout;
}

TmpDir::TmpDir(const wstring& path) : m_Path(GenerateNonexistDirName(path))
{
	fs::create_directories(m_Path);	
}



TmpDir::~TmpDir()
{
	if (fs::exists(m_Path))
	{
		_force_remove_all(m_Path);
		/*WriteToConsole(GetPath());
		error_code ec;
		fs::remove_all(m_Path,ec);

		WriteToConsole(string("Code: ") + to_string(ec.value()));
		WriteToConsole(string("Category: ") + ec.category().name());
		WriteToConsole(string("Message: ") + ec.message());*/
	}
}


std::wstring TmpDir::GetPath() const
{
	return wstring(m_Path);
}

std::wstring TmpDir::GenerateNonexistDirName(const wstring& path)
{
	wstring tmpPath = path;
	while (fs::exists(tmpPath))
	{
		tmpPath = path + to_wstring(RandomInt<int>(0,1000000));
	}

	return tmpPath; 
}

