#pragma once


//#ifndef MyUtils_API
//#define MyUtils_API
//#endif


MyUtils_API wstring wildcard2wregex(const wstring& wildcard);
MyUtils_API const wstring& GetThisProgramExeDirPath();


MyUtils_API int FileCmp(const wstring& file1, const wstring& file2);


MyUtils_API void WriteToStdout(const string& message);
MyUtils_API void WriteToStdout(const wstring& message);

MyUtils_API void WriteToStderr(const string& message);
MyUtils_API void WriteToStderr(const wstring& message);



MyUtils_API wstring str2wstr(const string& message);
MyUtils_API string wstr2str(const wstring& message);
MyUtils_API wstring utf8chars2wstr(const char* message);
MyUtils_API string wstr2utf8(const wstring& message);



MyUtils_API string AsciiStringToLower(string message);
MyUtils_API wstring AsciiStringToLower(wstring message);

MyUtils_API wstring AsciiStringToUpper(wstring message);
MyUtils_API void TrimQuotes(wstring& arg);
MyUtils_API wstring TrimString(const wstring& arg);
MyUtils_API bool IsDecimalDigitsOnly(const wstring& arg);




MyUtils_API void _wReadLine(istream& in, wstring& line);

MyUtils_API fs::path GetCurrentUserHomePath();



MyUtils_API wstring file_time_type2wstr(const fs::file_time_type& now);


template <typename OutType>
void SplitWStringAnyOf(const wstring& text, const wchar_t* delims, list<OutType>& words, bool noEmpty = true);


MyUtils_API void ReplaceAllSubStrings(string& line, const string& macro, const string& value);
MyUtils_API void ReplaceAllSubStrings(wstring& line, const wstring& macro, const wstring& value);


MyUtils_API void ExecuteProgram_except(const wstring& cmd, const wstring& args = L"");
MyUtils_API int ExecuteProgram(const wstring& cmd, const wstring& args = L"");
				

MyUtils_API void AddPath(const wstring& newpath);

MyUtils_API void _copy_file(const wstring& src, const wstring& dest);
MyUtils_API void _force_remove_all(const fs::path& path);

MyUtils_API fs::path _relative_path_to(const fs::path& relPath, const fs::path& finalPath);






template <typename Func>
void ForEachNonEmptySubword(const wstring& filetypes, wchar_t delimiter, Func func);


template <typename IntType>
IntType RandomInt(IntType min, IntType max);


template <class ValueType, class BinaryPredicate>
void erase_if_swallowed(list<ValueType>& container, BinaryPredicate pred);


MyUtils_API bool _wstricmp(const wstring& lhs, const wstring& rhs);




template <class DerivedT>
class Singleton
{
protected:
	Singleton() = default;
public:
	static DerivedT& Instance();
	virtual ~Singleton() = default;
};


template <class T>
class SettingsField
{
public:
	SettingsField() : m_Initialized(false)
	{
	}
	template <typename U>
	SettingsField(const U& rhs) : m_Value(static_cast<T>(rhs)), m_Initialized(true)
	{
	}

	template <typename U>
	SettingsField<T>& operator=(const U& rhs)
	{
		m_Value = static_cast<T>(rhs);
		m_Initialized = true;
		return *this;
	}

	//const T* operator->() const
	//{
	//	return &m_Value;
	//}

	T* operator->()
	{
		return &m_Value;
	}

	T& get()
	{
		return m_Value;
	}

	const T& get() const
	{
		return m_Value;
	}
			
	bool Initialized() const
	{
		return m_Initialized;
	}

	template <typename U>
	SettingsField<T>& Override(const SettingsField<U>& rhs)
	{
		if (rhs.Initialized())
		{
			get() = static_cast<T>(rhs.get());
			m_Initialized = true;
		}
		return *this;
	}
protected:
	bool	m_Initialized;
	T		m_Value;
};

template<>
template <typename U>
SettingsField<wstring>::SettingsField(const U& rhs) : m_Value(rhs), m_Initialized(rhs != wstring(L""))
{

}

template<>
template <typename U>
SettingsField<wstring>& SettingsField<wstring>::operator=(const U& rhs)
{	
	m_Value = static_cast<wstring>(rhs);
	m_Initialized = (rhs != wstring(L""));
	return *this;
}

//
//template <class Type>
//inline Type checked_cast(QObject* ptr)
//{
//	assert(dynamic_cast<Type>(ptr) != nullptr);
//	return static_cast<Type>(ptr);
//}

//template <class Type>
//inline Type checked_cast(const QObject* ptr)
//{
//	assert(dynamic_cast<const Type>(ptr) != nullptr);
//	return static_cast<const Type>(ptr);
//}


//
//template <typename T>
//class SettingsFieldBase
//{
//public:
//	SettingsFieldBase() : m_Initialized(false)
//	{
//	}
//	template <typename U>
//	SettingsFieldBase(const U& rhs) : m_Value(static_cast<T>(rhs)), m_Initialized(true)
//	{
//	}
//
//	template <typename U>
//	SettingsFieldBase<T>& operator=(const U& rhs)
//	{
//		m_Value = static_cast<T>(rhs);
//		m_Initialized = true;
//		return *this;
//	}
//
//	virtual T& Value() = 0;
//	virtual const T& Value() const = 0;
//
//	bool Initialized() const
//	{
//		return m_Initialized;
//	}
//
//	template <typename U>
//	SettingsFieldBase<T>& Override(const SettingsFieldBase<U>& rhs)
//	{
//		if (rhs.Initialized())
//		{
//			Value() = static_cast<T>(rhs.Value());
//			m_Initialized = true;
//		}
//		return *this;
//	}
//protected:
//	bool m_Initialized;
//};
//
//
//template <typename T>
//class SimpleSettingsField : public SettingsFieldBase<T>
//{
//public:
//	virtual T& Value()  override final
//	{
//		return static_cast<T&>(m_Value);
//	}
//
//	virtual const T& Value() const override final
//	{
//		return static_cast<const T&>(m_Value);
//	}
//
//	template <typename U>
//	SimpleSettingsField<T>& Override(const SettingsFieldBase<U>& rhs)
//	{
//		return static_cast<SimpleSettingsField<T>>(SettingsFieldBase<T>::Override(rhs));
//	}
//private:
//	T m_Value;
//};
//
//
//template <class T>
//class SettingsField : public T, public SettingsFieldBase<T>
//{
//public:
//	virtual T& Value()  override final
//	{
//		return static_cast<T&>(*this);
//	}
//
//	virtual const T& Value() const override final
//	{
//		return static_cast<const T&>(*this);
//	}
//
//	template <typename U>
//	SettingsField<T>& Override(const SettingsFieldBase<U>& rhs)
//	{
//		return static_cast<SettingsField<T>>(SettingsFieldBase<T>::Override(rhs));
//	}
//};
//
//


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


//template <basic_ostream<wchar_t> stdio>
//class LockableWStdIO
//{
//private:
//	LockableWStdIO(mutex& mtx);
//public:
//	static LockableWStdIO<stdio> Instance()
//	{
//		static mutex mtx;
//		return LockableWCout(mtx);
//	}
//	decltype(stdio)& Get()
//	{
//		return stdio;
//	}
//private:
//	static unique_lock<mutex> m_Lock;
//};
//
//
//
//using LockableStdOut = LockableWStdIO<wcout>;
//using LockableStdErr =  LockableWStdIO<wcerr>;



class MyUtils_API TmpDir
{
public:	
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


template <typename OutType>
void SplitWStringAnyOf(const wstring& text, const wchar_t* delims, list<OutType>& words, bool noEmpty /*= true*/)
{
	auto pos = text.find_first_of(delims);
	decltype(pos) start = 0;

	while (pos != wstring::npos)
	{
		auto sub = text.substr(start, pos - start);
		if (!sub.empty() || !noEmpty)
			words.emplace_back(sub);

		start = pos + 1;
		pos = text.find_first_of(delims, start);
	}

	auto sub = text.substr(start);
	if (!sub.empty() || !noEmpty)
		words.emplace_back(sub);
}




///{a,	b,	c,	d}		is_include_pred

//Предполагается, что  is_include_pred(a,b) и is_include_pred(c,d)

//Исходные сравнения

//			a		b		c		d
//a			*		+		+		+
//
//b					*		+		+
//
//c							*		+
//
//d									*

//Результат
//{ab,	cd}		a поглотила b, c поглотила d
template <class ValueType, class BinaryPredicate>
void erase_if_swallowed(list<ValueType>& container, BinaryPredicate pred)
{
	erase_if_swallowed(container.begin(), container.end(), pred, [&container](auto it)
	{
		return container.erase(it);
	});
	

	//if (container.size() < 2)
	//	return;

	//
	//auto preend_it = container.end();
	//--preend_it;
	//
	//for (auto lhs_it = container.begin()  ; lhs_it != preend_it; ++lhs_it)
	//{		
	//	auto rhs_it = lhs_it;
	//	advance(rhs_it, 1);
	//	for (++rhs_it; rhs_it != container.end(); )
	//	{			
	//		if (pred(*lhs_it, *rhs_it))
	//		{
	//			rhs_it = container.erase(rhs_it);
	//			preend_it = container.end();
	//			--preend_it;
	//		}
	//		else
	//		{
	//			++rhs_it;
	//		}
	//	}
	//}
}



template <class InputIterator, class BinaryPredicate, class EraseFunctor>
void erase_if_swallowed(InputIterator first, InputIterator last, BinaryPredicate pred, EraseFunctor erase)
{
	if (distance(first, last) < 2)
		return;
	
	auto preend_it = last;
	--preend_it;

	for (auto lhs_it = first; lhs_it != preend_it; ++lhs_it)
	{
		auto rhs_it = lhs_it;		
		for (++rhs_it; rhs_it != last; )
		{
			if (pred(*lhs_it, *rhs_it))
			{
				rhs_it = erase(rhs_it);
				
				if (rhs_it == last)
				{
					preend_it = last;
					--preend_it;
					if (preend_it == lhs_it)
						return;
				}
			}
			else
			{
				++rhs_it;
			}
		}
	}
}