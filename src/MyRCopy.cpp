// RCopy.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "Reserver.h"



int main(int argc, char* argv[])
{
	int result = 0;
	try
	{
		locale::global(locale(""));		

		if (argc > 1)
		{
			if (argv[1] == string("-schtask"))
			{
				WriteToConsole(L"Time for RCOPY");				
				MessageBox(0,L"Time for RCOPY",L"RCOPY",0);
			}
		}

		Reserver r;
		r.Process();		
	}
	catch (BaseException& e)
	{
		WriteToConsole(wstring(L"BaseException: ") + e.ErrorMessage());
		result = -1;
	}
	catch (range_error& e)
	{
		WriteToConsole(string("range_error: ") + e.what());
		result = -1;
	}
	catch (system_error& e)
	{		
		WriteToConsole(string("system_error: ") + e.what());
		WriteToConsole(string("Error: ") + e.what());
		WriteToConsole(string("Code: ") + to_string(e.code().value()));
		WriteToConsole(string("Category: ") + e.code().category().name());
		WriteToConsole(string("Message: ") + e.code().message());

		result = -1;
	}	
	catch (std::exception& e)
	{
		WriteToConsole(string("std::exception: ") + e.what());
		result = -1;
	}
	catch (...)
	{
		WriteToConsole(L"unknown exception");
		result = -1;
	}
	
	if (result == 0)
	{
		WriteToConsole(L"Success finished!!! Press any key");		
	}
	else
	{
		WriteToConsole(L"Finished with error!!! Press any key");		
	}

	wchar_t symbol;
	wcin >> symbol;
	return result;
}


