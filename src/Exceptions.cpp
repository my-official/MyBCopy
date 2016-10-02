#include "stdafx.h"
#include "Exceptions.h"


//	void RuntimeExeption::TextLine(const universal_string& text)
//	{
//		
//#if defined(UNICODE) || defined(_UNICODE)		
//		m_ErrorMessage += text + L"\n";
//#else
//		m_ErrorMessage += text + "\n";
//#endif
//	}

	void RuntimeExeption::TextLine(const char* text)
	{
#if defined(UNICODE) || defined(_UNICODE)
		auto textLength = MultiByteToWideChar(CP_ACP, 0, text, -1, NULL, 0) - 1;

		assert(textLength && textLength == strlen(text));

		auto prevErrorMessageLength = m_ErrorMessage.length();
		m_ErrorMessage.resize(prevErrorMessageLength + textLength + 1, L'\n');

		wchar_t* data = &m_ErrorMessage[prevErrorMessageLength];

		int result = MultiByteToWideChar(CP_ACP, 0, text, -1, data, textLength + 1);

		assert(result);
#else
		m_ErrorMessage += text;
		m_ErrorMessage += "\n";
#endif
	}

	void RuntimeExeption::TextLine(const std::string& text)
	{
#if defined(UNICODE) || defined(_UNICODE)		
		auto textLength = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, NULL, 0) - 1;

		assert(textLength && textLength  == text.length());

		auto prevErrorMessageLength = m_ErrorMessage.length();
		m_ErrorMessage.resize(prevErrorMessageLength + textLength + 1, L'\n');
		
		wchar_t* data = &m_ErrorMessage[prevErrorMessageLength];
		
		int result = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, data, textLength + 1);

		assert(result);
#else
		m_ErrorMessage += text;
		m_ErrorMessage += "\n";
#endif
	}

	void RuntimeExeption::TextLine(const wchar_t* text)
	{
#if defined(UNICODE) || defined(_UNICODE)		
		m_ErrorMessage += text;
		m_ErrorMessage += L"\n";
#else
		m_ErrorMessage += wstring_convert< codecvt<wchar_t, char, mbstate_t> >().to_bytes(text); 
		m_ErrorMessage += "\n";						
#endif
	}

	void RuntimeExeption::TextLine(const std::wstring& text)
	{
#if defined(UNICODE) || defined(_UNICODE)		
		m_ErrorMessage += text;
		m_ErrorMessage += L"\n";
#else
		m_ErrorMessage += wstring_convert< codecvt<wchar_t, char, mbstate_t> >().to_bytes(text.c_str());
		m_ErrorMessage += "\n";
#endif
	}


	const TCHAR* RuntimeExeption::ErrorMessage()
	{
		return m_ErrorMessage.c_str();
	}

	void RuntimeExeption::OutputToStdErr()
	{
#if defined(UNICODE) || defined(_UNICODE)		
		wcerr << m_ErrorMessage;		
#else
		cerr << m_ErrorMessage;		
#endif
	}

#ifdef _DEBUG

	void DebugExeption::SetFileAndLine(const char* file, int line)
	{
		assert(file);

		TextLine(string(file) + ":" + to_string(line));
	}

#endif

	const TCHAR* CachedMessageExeption::ErrorMessage()
	{
		if (!m_MessageFormated)
			FormattedMessageLine();
		return BaseExeption::ErrorMessage();
	}


	SystemException::SystemException(DWORD errorCode /*= GetLastError()*/) : m_ErrorCode(errorCode)
	{
		
	}


	void SystemException::FormattedMessageLine()
	{
		WindowsFormatMessage(*this, m_ErrorCode);
	}


	void SystemException::WindowsFormatMessage(BaseExeption& e, DWORD errorCode)
	{
		LPTSTR  lpBuffer = NULL;

		DWORD  result = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpBuffer,
			0, NULL);

		if (result == 0)
		{
			if (lpBuffer)
			{
				LocalFree(lpBuffer);
			}
			return;
		}

		try
		{
			e.TextLine(lpBuffer);
		}
		catch (...)
		{
			LocalFree(lpBuffer);
			throw;
		}
	}

	ErrnoException::ErrnoException() : m_Errno(errno)
	{

	}

	void ErrnoException::FormattedMessageLine()
	{
		SystemException::FormattedMessageLine();
		TextLine( "errno == " + to_string(m_Errno) );
	}

	

	
