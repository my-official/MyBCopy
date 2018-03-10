#include "MyBCopy_pch.h"
#include "Exceptions.h"


RuntimeException::RuntimeException(const char* text)
{
	TextLine(text);
}

RuntimeException::RuntimeException(const std::string& text)
{
	TextLine(text);
}

RuntimeException::RuntimeException(const wchar_t* text)
{
	TextLine(text);
}

RuntimeException::RuntimeException(const std::wstring& text)
{
	TextLine(text);
}

//	RuntimeException& RuntimeException::TextLine(const universal_string& text)
//	{
//		
//#if defined(UNICODE) || defined(_UNICODE)		
//		m_ErrorMessage += text + L"\n";
//#else
//		m_ErrorMessage += text + "\n";
//#endif
//	}

	RuntimeException& RuntimeException::TextLine(const char* text)
	{
#if defined(UNICODE) || defined(_UNICODE)
		auto textLength = MultiByteToWideChar(CP_ACP, 0, text, -1, NULL, 0) - 1;

		assert(textLength && textLength == strlen(text));

		auto prevErrorMessageLength = m_ErrorMessage.length();
		m_ErrorMessage.resize(prevErrorMessageLength + textLength + 1, L'\n');

		wchar_t* data = &m_ErrorMessage[prevErrorMessageLength];

		int result = MultiByteToWideChar(CP_ACP, 0, text, -1, data, textLength + 1);

		assert(result);

		m_ErrorMessage[prevErrorMessageLength + textLength] = '\n';
#else
		m_ErrorMessage += text;
		m_ErrorMessage += "\n";
#endif
		return *this;
	}

	RuntimeException& RuntimeException::TextLine(const std::string& text)
	{
#if defined(UNICODE) || defined(_UNICODE)		
		auto textLength = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, NULL, 0) - 1;

		assert(textLength && textLength  == text.length());

		auto prevErrorMessageLength = m_ErrorMessage.length();
		m_ErrorMessage.resize(prevErrorMessageLength + textLength + 1, L'\n');
		
		wchar_t* data = &m_ErrorMessage[prevErrorMessageLength];
		
		int result = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, data, textLength + 1);

		assert(result);

		m_ErrorMessage[prevErrorMessageLength + textLength] = '\n';
#else
		m_ErrorMessage += text;
		m_ErrorMessage += "\n";
#endif
		return *this;
	}

	RuntimeException& RuntimeException::TextLine(const wchar_t* text)
	{
#if defined(UNICODE) || defined(_UNICODE)		
		m_ErrorMessage += text;
		m_ErrorMessage += L"\n";
#else
		m_ErrorMessage += wstring_convert< codecvt<wchar_t, char, mbstate_t> >().to_bytes(text); 
		m_ErrorMessage += "\n";						
#endif
		return *this;
	}

	RuntimeException& RuntimeException::TextLine(const std::wstring& text)
	{
#if defined(UNICODE) || defined(_UNICODE)		
		m_ErrorMessage += text;
		m_ErrorMessage += L"\n";
#else
		m_ErrorMessage += wstring_convert< codecvt<wchar_t, char, mbstate_t> >().to_bytes(text.c_str());
		m_ErrorMessage += "\n";
#endif
		return *this;
	}


	const TCHAR* RuntimeException::ErrorMessage()
	{
		return m_ErrorMessage.c_str();
	}

	void RuntimeException::OutputToStdErr()
	{
#if defined(UNICODE) || defined(_UNICODE)		
		wcerr << m_ErrorMessage;		
#else
		cerr << m_ErrorMessage;		
#endif
	}

#ifdef _DEBUG

	void DebugException::SetFileAndLine(const char* file, int line)
	{
		assert(file);

		TextLine(string(file) + ":" + to_string(line));
	}

#endif

	const TCHAR* CachedMessageException::ErrorMessage()
	{
		if (!m_MessageFormated)
			FormattedMessageLine();
		return BaseException::ErrorMessage();
	}


	WinException::WinException(DWORD errorCode /*= GetLastError()*/) : m_ErrorCode(errorCode)
	{
		
	}


	void WinException::FormattedMessageLine()
	{
		WindowsFormatMessage(*this, m_ErrorCode);
	}


	void WinException::WindowsFormatMessage(BaseException& e, DWORD errorCode)
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

	ErrnoException::ErrnoException(errno_t err) : m_Errno(static_cast<int>(err))
	{

	}

	void ErrnoException::FormattedMessageLine()
	{
		WinException::FormattedMessageLine();
		TextLine( "errno == " + to_string(m_Errno) );
	}

	

	
