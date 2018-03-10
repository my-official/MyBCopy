#include "blackbox_pch.h"
#include "BlackBoxTest.h"






int main(int argc, char *argv[])
{
	try
	{		
		locale::global(locale(""));

		TmpDir tmpDir(fs::path(Config::TESTFILES_DIR) / fs::path(L"BlackBoxTest_Source"));
		g_SourceTmpPath = tmpDir.GetPath();
		for (auto& scenario : g_Scenarios)
		{
			scenario->Execute();
		}
		return 0;
	}
	catch (BaseException& e)
	{
		WriteToStderr(wstring(L"BaseException: ") + e.ErrorMessage());
	}
	catch (range_error& e)
	{
		WriteToStderr(string("range_error: ") + e.what());
	}
	catch (system_error& e)
	{
		WriteToStderr(string("system_error: ") + e.what());
		WriteToStderr(string("Error: ") + e.what());
		WriteToStderr(string("Code: ") + to_string(e.code().value()));
		WriteToStderr(string("Category: ") + e.code().category().name());
		WriteToStderr(string("Message: ") + e.code().message());
	}
	catch (std::exception& e)
	{
		WriteToStderr(string("std::exception: ") + e.what());
	}
	catch (...)
	{
		WriteToStderr(L"unknown exception");
	}

	return -1;
}
