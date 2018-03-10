#include "MyBCopy_pch.h"
#include "Utils.h"
#include "Storage.h"
#include "Exceptions.h"


MyUtils_API wstring wildcard2wregex(const wstring& wildcard)
{
	static const wchar_t EscapeSymbols[] = { '$', '^', '[', ']', '(', ')', '{', '|', '+', '\\', '.', '<', '>' };

	std::wstring result;

	for (size_t c_symbol = 0, size_symbol = wildcard.length(); c_symbol < size_symbol; c_symbol++)
	{
		wchar_t ch = wildcard[c_symbol];
		bool escaped = false;
		for (int c_escape = 0, size_escape = sizeof(EscapeSymbols) / sizeof(wchar_t); c_escape < size_escape; c_escape++)
		{
			if (ch == EscapeSymbols[c_escape])
			{
				result.push_back(L'\\');
				result.push_back(ch);				
				escaped = true;
				break;
			}
		}

		if (!escaped)
		{
			if (ch == L'*')
			{
				result += L".*";
			}
			else if (ch == L'?')
			{
				result += L".";
			}
			else
			{
				result += ch;
			}
		}
	}
	return result;

/*


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
		pos = result.find_first_of(L'*', pos + 2);
	}

	pos = result.find(L'?');

	while (pos != wstring::npos)
	{
		result.replace(pos, 1, L".+");
		pos = result.find_first_of(L'?', pos + 2);
	}
*/
	return result;
}

static std::wstring GetThisProgramExeDirPathStatic()
{
	vector<TCHAR> buffer(MAX_PATH, 0);

	if (GetModuleFileName(nullptr, buffer.data(), buffer.size()) == 0)
		throw EXCEPTION(WinException());

	return fs::path(buffer.data()).parent_path();
}

const wstring& GetThisProgramExeDirPath()
{
	static const std::wstring path = GetThisProgramExeDirPathStatic();	
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
//	return wstring_convert< codecvt<wchar_t, char, mbstate_t> >().from_bytes(message);
	int result = MultiByteToWideChar(CP_ACP, 0, message.c_str(), message.length(), NULL, 0);

	if (result == 0)
		throw EXCEPTION(WinException());

	assert(result - 1 != message.length());

	wstring wmessage(result, 0);

	wchar_t* data = &wmessage.front();

	result = MultiByteToWideChar(CP_ACP, 0, message.c_str(), message.length(), data, result);

	if (result == 0)
		throw EXCEPTION(WinException());

	return wmessage;
}


std::string wstr2str(const wstring& message)
{
	return wstring_convert< codecvt<wchar_t, char, mbstate_t> >().to_bytes(message);
}

std::wstring utf8chars2wstr(const char* message)
{
	return wstring_convert< codecvt_utf8<wchar_t> >().from_bytes(message);
}


string wstr2utf8(const wstring& message)
{
	return wstring_convert< codecvt_utf8<wchar_t> >().to_bytes(message);
}



string AsciiStringToLower(string message)
{
	transform(message.begin(), message.end(), message.begin(), ::tolower);
	return message;
}

wstring AsciiStringToLower(wstring message)
{
	transform(message.begin(), message.end(), message.begin(), ::tolower);
	return message;
}

wstring AsciiStringToUpper(wstring message)
{
	transform(message.begin(), message.end(), message.begin(), ::toupper);
	return message;
}

void TrimQuotes(wstring& arg)
{
	if (arg.size() < 2)
		return;

	if (arg.front() != arg.back() || arg.back() != L'"')
		return;
	arg = arg.substr(1, arg.length() - 2);
}

MyUtils_API wstring TrimString(const wstring& arg)
{
	auto start = arg.find_first_of(L" \t");
	if (start == wstring::npos)
		return arg;

	if (start == arg.length() - 1)
		return wstring();

	auto len = arg.find_first_of(L" \t", start + 1);

	if (len != wstring::npos)
	{
		len -= start;
	}

	return arg.substr(start, len);
}

MyUtils_API bool IsDecimalDigitsOnly(const wstring& arg)
{
	if (arg.empty())
		return false;

	return all_of(arg.begin(), arg.end(), &isdigit);
}

std::wstring file_time_type2wstr(const fs::file_time_type& now)
{
	time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct tm now_tm;

	auto err = localtime_s(&now_tm, &now_c);
	if (err != 0)
	{
		throw EXCEPTION( ErrnoException() );
	}
	

	char buffer[20];
	auto result = strftime(buffer, sizeof(buffer), "%F %T", &now_tm);
	assert(result == 19);
	return str2wstr( string(buffer) );
}


//void _wReadLine(istream& in, wstring& line)
//{
//	const size_t BUFFER_SIZE = 255;
//	char buffer[25];
//}

void WriteToStdout(const wstring& message)
{
	static mutex mtx;
	unique_lock<mutex> lock(mtx);
	wcout << message << endl;
}


void WriteToStdout(const string& message)
{		
	WriteToStdout(str2wstr(message));
}

void WriteToStderr(const string& message)
{
	WriteToStderr(str2wstr(message));
}

void WriteToStderr(const wstring& message)
{
	static mutex mtx;
	unique_lock<mutex> lock(mtx);
	wcerr << message << endl;

	OutputDebugString(message.c_str());
	OutputDebugString(L"\n");
}


void ReplaceAllSubStrings(string& line, const string& macro, const string& value)
{
	auto pos = line.find(macro);

	while (pos != string::npos)
	{
		line.replace(pos, macro.length(), value);
		pos = line.find(macro);
	}
}

void ReplaceAllSubStrings(wstring& line, const wstring& macro, const wstring& value)
{
	auto pos = line.find(macro);

	while (pos != wstring::npos)
	{
		line.replace(pos, macro.length(), value);
		pos = line.find(macro);
	}
}


void ExecuteProgram_except(const wstring& cmd, const wstring& args /*= L""*/)
{
	if (0 != ExecuteProgram(cmd, args))
	{
		throw EXCEPTION( BaseException() );
	}
}

int ExecuteProgram(const wstring& cmd, const wstring& args /*= L""*/)
{		
	QProcess process(nullptr);
	process.setProgram(QString::fromStdWString(cmd));
	process.setNativeArguments(QString::fromStdWString(args));
	process.start();
	process.waitForFinished(-1);
	return process.exitCode();

	//STARTUPINFO si;
	//PROCESS_INFORMATION pi;

	//ZeroMemory(&si, sizeof(si));
	//si.cb = sizeof(si);
	//ZeroMemory(&pi, sizeof(pi));
	//
	//wchar_t* argsPtr = new wchar_t[args.length() + 1];
	//memcpy(argsPtr, args.data(), sizeof(wchar_t) * args.length() + 1);
	//		
	//if (CreateProcess(cmd.c_str(), argsPtr, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) == 0)
	//{
	//	delete[] argsPtr;
	//	throw EXCEPTION( WinException() );
	//}
	//	
	//	 
	//if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)
	//{
	//	delete[] argsPtr;
	//	throw EXCEPTION(WinException());
	//}

	//DWORD exitCode;
	//
	//if (GetExitCodeProcess(pi.hProcess, &exitCode) == 0)
	//{
	//	delete[] argsPtr;
	//	throw EXCEPTION(WinException());
	//}

	//assert(exitCode != STILL_ACTIVE);

	// Close process and thread handles. 
	//CloseHandle(pi.hProcess);
	//CloseHandle(pi.hThread);
	//	
	//delete[] argsPtr;
	//
	//return int(exitCode);
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

void _copy_file(const wstring& src, const wstring& dest)
{
	auto parentPath = fs::path(dest).parent_path();

	assert(fs::exists(src));
	assert(!fs::exists(dest));

	if (!fs::exists(parentPath))
	{
		if (!fs::create_directories(parentPath))
			throw EXCEPTION(WinException());
	}

	
	fs::copy_file(src, dest);
}


void _force_remove_all(const fs::path& path)
{	
	for (auto p : fs::recursive_directory_iterator(path))
	{
		fs::permissions(p, fs::perms::add_perms | fs::perms::all);
	}
	remove_all(path);
}

MyUtils_API fs::path _relative_path_to(const fs::path& relPath, const fs::path& finalPath)
{
	auto p = mismatch(finalPath.begin(), finalPath.end(), relPath.begin(), relPath.end());
	if (p.second != relPath.end())
		throw EXCEPTION( BaseException("_relative_path_to") );

	if (p.first == finalPath.end())
		return fs::path();
	
	auto it = p.first;
	fs::path result = accumulate(++it, finalPath.end(), *p.first, [](auto& lhs, auto& rhs)
	{
		return lhs / rhs;
	});
		
	return result;
}

bool _wstricmp(const wstring& lhs, const wstring& rhs)
{	
	return _wcsicmp(lhs.c_str(), rhs.c_str()) == 0;
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

TmpDir::TmpDir(const wstring& path) : m_Path( fs::canonical(GenerateNonexistDirName(path)) )
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

fs::path GetCurrentUserHomePath()
{
	wchar_t* buffer;
	errno_t  result = _wdupenv_s(&buffer, nullptr,L"USERPROFILE");
	if (result != 0)
	{
		throw EXCEPTION(ErrnoException(result));
	}
	 return buffer;
}