#pragma once



wstring wildcard2regex(const wstring& wildcard);
wstring GetThisProgramExeDirPath();


int FileCmp(const wstring& file1, const wstring& file2);


void WriteToConsole(const string& message);
void WriteToConsole(const wstring& message);
wstring str2wstr(const string& message);
void RunCMD(const wstring& cmd);
void AddPath(const wstring& newpath);

void CopyFileAndPrint(const wstring& src, const wstring& dest, bool print = true);
void RemoveFileAndPrint(const wstring& file);



template <typename Func>
void ForEachNonEmptySubword(const wstring& filetypes, wchar_t delimiter, Func func);


template <typename IntType>
IntType RandomInt(IntType min, IntType max);




template <class DerivedT>
class Singleton
{
protected:
	Singleton() = default;
public:
	static DerivedT& Instance();
	virtual ~Singleton() = default;
};



class LockableWCout
{
private:
	LockableWCout(mutex& mtx);
public:
	static LockableWCout Instance();
	decltype(wcout)& Get();
private:
	unique_lock<mutex> m_Lock;
};




class TmpDir
{
public:
	TmpDir() = default;
	TmpDir(const wstring& path);
	TmpDir(TmpDir&&) noexcept = default;
	TmpDir& operator=(TmpDir&&) noexcept = default;
	TmpDir(const TmpDir&) = delete;
	TmpDir& operator=(const TmpDir&) = delete;
	virtual ~TmpDir();

	wstring GetPath() const;
protected:
	fs::path m_Path;
	static wstring GenerateNonexistDirName(const wstring& path);
};



void _force_remove_all(const fs::path& path);



//////////////////////////////////


template <class DerivedT>
DerivedT& Singleton<DerivedT>::Instance()
{
	static DerivedT instance;
	return instance;
}






template <typename Func>
void ForEachNonEmptySubword(const wstring& filetypes, wchar_t delimiter, Func func)
{
	auto length = filetypes.length();
	if (length == 0)
		return;

	auto pos = filetypes.find(delimiter);

	if (pos != wstring::npos)
	{
		decltype(pos) start = 0;
		do
		{
			if (start == pos)
			{
				start += 1;
			}
			else
			{
				func(&filetypes[start], pos - start);
				start = pos + 1;
			}

			if (start >= length)
				return;

			pos = filetypes.find(delimiter, start);

		} while (pos != wstring::npos);

		func(&filetypes[start], length - start);
	}
	else
	{
		func(&filetypes[0], length);
	}
}


template <typename IntType>
IntType RandomInt(IntType min, IntType max)
{
	thread_local static mt19937 generator((unsigned int)time(NULL));
	uniform_int_distribution<IntType> distribution(min, max);
	return distribution(generator);
}


