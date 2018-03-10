#pragma once


#ifndef MyException_API
#define MyException_API
#endif


#if defined(UNICODE) || defined(_UNICODE)
#define	universal_string	std::wstring
#else
#define	universal_string std::string
#endif	

	

	class MyException_API RuntimeException
	{
	public:		
		RuntimeException() = default;
		RuntimeException(const char* text);
		RuntimeException(const std::string& text);
		RuntimeException(const wchar_t* text);
		RuntimeException(const std::wstring& text);
		
		RuntimeException(const RuntimeException& rhs) = default;
		RuntimeException& operator=(const RuntimeException& rhs) = default;

		RuntimeException(RuntimeException&& rhs) = default;
		RuntimeException& operator=(RuntimeException&& rhs) = default;

		virtual ~RuntimeException() = default;


		RuntimeException& TextLine(const char* text);
		RuntimeException& TextLine(const std::string& text);

		RuntimeException& TextLine(const wchar_t* text);
		RuntimeException& TextLine(const std::wstring& text);

		virtual const TCHAR* ErrorMessage();
		void OutputToStdErr();
	protected:
		universal_string m_ErrorMessage;
	};



#ifdef _DEBUG
//#if 1

	//template <class DerivedClass>
	class MyException_API DebugException : public RuntimeException
	{
	public:
		using RuntimeException::RuntimeException;

		template <class ExceptionT>
		static ExceptionT& DebugFileAndLine(ExceptionT& e, const char* file, int line)
		{
			e.SetFileAndLine(file, line);
			return e;
		}				

		void SetFileAndLine(const char* file, int line);		
	};

	//#define public_Inheritance_For(classname) public DebugException<classname>

	typedef DebugException BaseException;

#define EXCEPTION(exceptionInstance)	DebugException::DebugFileAndLine(exceptionInstance, __FILE__,__LINE__)
#else

	//#define public_Inheritance_For(classname) public RuntimeException

	typedef RuntimeException BaseException;

#define EXCEPTION(exceptionInstance)	exceptionInstance
#endif
	
	#define EXCEPTION_FUNC EXCEPTION( BaseException(__func__))

	

	class MyException_API CachedMessageException : public BaseException
	{
	public:		
		using BaseException::BaseException;
		virtual const TCHAR* ErrorMessage() override;
	protected:
		bool m_MessageFormated = false;
		virtual void FormattedMessageLine() = 0;		
	};



	class MyException_API WinException : public CachedMessageException
	{
	public:
		WinException(DWORD errorCode = GetLastError());

		DWORD m_ErrorCode;				
	protected:
		static void WindowsFormatMessage(BaseException& e, DWORD errorCode);

		virtual void FormattedMessageLine() override;		
	};

	

	class MyException_API ErrnoException : public WinException
	{
	public:
		ErrnoException();				
		ErrnoException(errno_t err);
		int m_Errno;

	protected:		
		virtual void FormattedMessageLine() override;
	};



	class MyException_API DataException : public BaseException
	{
	public:
		using BaseException::BaseException;
	};
