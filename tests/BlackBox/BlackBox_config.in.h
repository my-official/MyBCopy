#pragma once


//#define TestDirPath LR"(@BlackBoxTest_DirPath@)"
//#define OVERRIDE_RAR_DIR LR"(@BlackBoxTest_OVERRIDE_RAR_DIR@)"
//#define OVERRIDE_7Z_DIR LR"(@BlackBoxTest_OVERRIDE_7Z_DIR@)"

namespace Config
{
	const wstring TESTFILES_DIR = LR"(@BlackBoxTest_TESTFILES_DIR@)";
	const wstring OVERRIDE_RAR_EXE = LR"(@BlackBoxTest_OVERRIDE_RAR_EXE@)";
	const wstring OVERRIDE_7Z_EXE = LR"(@BlackBoxTest_OVERRIDE_7Z_EXE@)";
	const wstring TEMPORARY_DIR = LR"(@BlackBoxTest_TEMPORARY_DIR@)";	
}