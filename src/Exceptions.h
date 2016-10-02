#pragma once



#if defined(UNICODE) || defined(_UNICODE)
#define	universal_string	std::wstring
#else
#define	universal_string std::string
#endif	

	

	class RuntimeExeption
	{
	public:		
		void TextLine(const char* text);
		void TextLine(const std::string& text);

		void TextLine(const wchar_t* text);
		void TextLine(const std::wstring& text);

		virtual const TCHAR* ErrorMessage();
		void OutputToStdErr();
	protected:
		universal_string m_ErrorMessage;
	};



#ifdef _DEBUG
//#if 1

	//template <class DerivedClass>
	class DebugExeption : public RuntimeExeption
	{
	public:
		template <class ExeptionT>
		static ExeptionT& DebugFileAndLine(ExeptionT& e, const char* file, int line)
		{
			e.SetFileAndLine(file, line);
			return e;
		}				

		void SetFileAndLine(const char* file, int line);		
	};

	//#define public_Inheritance_For(classname) public DebugExeption<classname>

	typedef DebugExeption BaseExeption;

#define EXCEPTION(exceptionInstance)	DebugExeption::DebugFileAndLine(exceptionInstance, __FILE__,__LINE__)
#else

	//#define public_Inheritance_For(classname) public RuntimeExeption

	typedef RuntimeExeption BaseExeption;

#define EXCEPTION(exceptionInstance)	exceptionInstance
#endif
	


	class CachedMessageExeption : public BaseExeption
	{
	public:		
		virtual const TCHAR* ErrorMessage() override;
	protected:
		bool m_MessageFormated = false;
		virtual void FormattedMessageLine() = 0;		
	};



	class SystemException : public CachedMessageExeption
	{
	public:
		SystemException(DWORD errorCode = GetLastError());

		DWORD m_ErrorCode;				
	protected:
		static void WindowsFormatMessage(BaseExeption& e, DWORD errorCode);

		virtual void FormattedMessageLine() override;		
	};

	

	class ErrnoException : public SystemException
	{
	public:
		ErrnoException();				
		int m_Errno;

	protected:		
		virtual void FormattedMessageLine() override;
	};
