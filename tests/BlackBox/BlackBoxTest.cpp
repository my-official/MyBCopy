#include "BlackBox_pch.h"
#include "BlackBoxTest.h"
#include "Utils.h"
#include "BlackBox_consts.h"


wstring g_SourceTmpPath;

void GenerateRandomFiles(const vector<wstring>& files)
{
	string data;
	data.reserve(1024*1024);
	for (auto& file : files)
	{
		GenerateRandomData(data);
		WriteNewFile(g_SourceTmpPath + L"\\" + file, data);
	}
}


void GenerateRandomData(string& result, size_t length /*= RandomInt(0, 256 * 1024)*/)
{
	result.resize(length);	
	
	for (size_t c_symbol = 0; c_symbol < length; ++c_symbol)
	{
		result[c_symbol] = RandomInt<int>(0, 255);
	}
}

void WriteNewFile(const wstring& file, const string& fileData)
{
	fs::create_directories(fs::path(file).parent_path());

	ofstream f;
	f.exceptions(ios::badbit | ios::failbit);

	f.open(file, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	f.write(fileData.data(), fileData.size());
}



void PreprocessingSrcXml(string& line)
{
	using namespace Config;

	static const map<string, wstring> macros = {
		 { "%BACKUP1_DIR%",	g_SourceTmpPath }
		,{ "%OVERRIDE_RAR_EXE%", OVERRIDE_RAR_EXE.empty() ? L"" : L"path=\"" + OVERRIDE_RAR_EXE + L"\"" }
		,{ "%OVERRIDE_7Z_EXE%", OVERRIDE_7Z_EXE.empty() ? L"" : L"path=\"" + OVERRIDE_7Z_EXE + L"\"" }
		,{ "%TEMPORARY_DIR%", TEMPORARY_DIR.empty() ? L"" : L"<temporary_dir>" + TEMPORARY_DIR + L"</temporary_dir>" }
		
		//,{ "%newmgr_DIR%", wstr2utf8( g_SourceTmpPath + L"\\" + g_NewMgrsDir) }
	//	,{ "%CTorrent_DIR%", wstr2utf8( g_SourceTmpPath + L"\\" + g_CTorrentDir) }		
		 
		
	};

	for (auto& macro : macros)
	{
		ReplaceAllSubStrings(line, macro.first, wstr2utf8( macro.second ));
	}
}






void Scenario::CreateBackup()
{
	Reserver reserver;	
	Notificators notificators;


	notificators.m_FinishNotificator = [&](const ParsedSources& backup, EBackupError error)
	{
		if (error == EBackupError::NoError)
			return;

		WriteToStderr(L"Backup " + backup.m_Name + L" error. ErrorCode is " + to_wstring(int(error)));
		throw EXCEPTION(BaseException());
	};

	notificators.m_ErrorStorageNotificator = [&](const ParsedSources& backup, const wstring& storageName, EBackupError error)
	{
		if (error == EBackupError::NoError)
			return;

		WriteToStderr(L"Storage " + storageName + L" error. Backup is " + backup.m_Name + L". ErrorCode is " + to_wstring(int(error)));
		throw EXCEPTION(BaseException());
	};

	reserver.Backup(m_MyBCopyFile, notificators);
}



void Scenario::ValidateFiles()
{
	/*forward_list<fs::path> checkFiles(fs::recursive_directory_iterator(checkTmpDir.GetPath()), fs::recursive_directory_iterator());

	if (!is_permutation(GetIteration()->m_FiltredFiles.begin(), GetIteration()->m_FiltredFiles.end(), checkFiles.begin(), checkFiles.end(), [](const fs::path& lhs, const fs::path& rhs)->bool
	{
		if (!fs::equivalent(lhs, rhs))
		{
			return false;
		}

		return FileCmp(lhs, rhs) == 0;
	}))
	{
		throw EXCEPTION(BaseException());
	}*/
}

void RegularScenario::PrepareFiles()
{
	GenerateRandomFiles(g_Backup1Files);
}


void RegularScenario::SaveSrcXML()
{
	ifstream in;
	in.exceptions(ios::badbit | ios::failbit);
	in.open(g_InputSrcXmlFile);
	in.exceptions(ios::badbit);

	ofstream out;
	out.exceptions(ios::badbit | ios::failbit);
	out.open(g_OutputSrcXmlFile, ios::trunc);

	string line;

	while (!in.eof())
	{
		getline(in, line);
		PreprocessingSrcXml(line);
		out << line << endl;
	}
}



void RegularScenario::Execute()
{
	PrepareFiles();
	SaveSrcXML();

	if (!m_MyBCopyFile.LoadFromXMLFile(g_OutputSrcXmlFile))
	{
		WriteToStderr(L"Can't load file " + g_OutputSrcXmlFile);
		throw EXCEPTION(BaseException());
	}
		

	auto scheme = make_shared<RegularScheme>();
	scheme->m_NumStoredOldRegularBackups = cm_NumIteration;
	m_MyBCopyFile.m_Settings.m_SchemeParams = scheme;
	

	for (auto& _storage : m_MyBCopyFile.m_Settings.m_Storages)
	{
		LocalDiskStorage& storage = dynamic_cast<LocalDiskStorage&>(*_storage);
		_force_remove_all(storage.m_Path);
	}
	

	for (int c = 0, size = cm_NumIteration; c < size; c++)
	{
		CreateBackup();
		Sleep(1000);
	}

	CheckRegularFiles();

	CreateBackup();

	CheckRegularFiles();
}


void RegularScenario::CheckRegularFiles()
{
	for (auto& storage : m_MyBCopyFile.m_Settings.m_Storages)
	{
		auto storageFiles = storage->GetEnumeratedBackupFiles(m_MyBCopyFile.m_Backups.front());
		if (storageFiles.Regular2Branches.size() != cm_NumIteration)
			throw EXCEPTION(BaseException());

		if (!storageFiles.LostRegular2Branches.empty())
			throw EXCEPTION(BaseException());

		for (auto& branch : storageFiles.Regular2Branches)
		{
			if (!branch.second.Branch2Increments.empty())
				throw EXCEPTION(BaseException());
		}
	}
}

void IncrementsScenarion::Execute()
{
	if (!m_MyBCopyFile.LoadFromXMLFile(g_OutputSrcXmlFile))
	{
		WriteToStderr(L"Can't load file " + g_OutputSrcXmlFile);
		throw EXCEPTION(BaseException());
	}

	auto scheme = make_shared<IncrementalScheme>();



	scheme->m_NumStoredOldRegularBackups = cm_NumStoredOldRegularBackups;

	scheme->m_NumIncrementBranches = 1;
	scheme->m_LengthIncrementChain = cm_LengthIncrementChain;

	scheme->m_ReferenceStorageName = m_MyBCopyFile.m_Settings.m_Storages.front()->m_Name;
	m_MyBCopyFile.m_Settings.m_SchemeParams = scheme;


	vector<wstring> newfiles;
	newfiles.push_back(L"realmmgr\\nodeapi\\nodeapi.cxx");
	newfiles.push_back(L"realmmgr\\nodeapi\\nodeapi.hxx");
	newfiles.push_back(L"realmmgr\\nodeapi\\scheme.bmp");
	GenerateRandomFiles(newfiles);

	for (unsigned int c_incrementBackup = 0; c_incrementBackup < scheme->m_LengthIncrementChain; c_incrementBackup++)
	{
		WriteNewFile(g_SourceTmpPath + L"\\ChangesChain1.cpp", "Chain 1 Change No. " + to_string(c_incrementBackup));
		CreateBackup();
		Sleep(1000);
	}
	
	WriteNewFile(g_SourceTmpPath + L"\\NewRegular.h", "Regular backup data");
	CreateBackup();
	Sleep(1000);

	for (unsigned int c_incrementBackup = 0; c_incrementBackup < scheme->m_LengthIncrementChain; c_incrementBackup++)
	{
		WriteNewFile(g_SourceTmpPath + L"\\ChangesChain2.cpp", "Chain 2 Change No. " + to_string(c_incrementBackup));
		CreateBackup();
		Sleep(1000);
	}

	CheckFiles();
}

void IncrementsScenarion::CheckFiles()
{
	for (auto& storage : m_MyBCopyFile.m_Settings.m_Storages)
	{
		auto storageFiles = storage->GetEnumeratedBackupFiles(m_MyBCopyFile.m_Backups.front());
		if (storageFiles.Regular2Branches.size() != cm_NumStoredOldRegularBackups)
			throw EXCEPTION(BaseException());

		if (!storageFiles.LostRegular2Branches.empty())
			throw EXCEPTION(BaseException());

		for (auto& branch : storageFiles.Regular2Branches)
		{
			if (branch.second.Branch2Increments.size() != 1)
				throw EXCEPTION(BaseException());

			auto& incrementBackupFileRels = branch.second.Branch2Increments.begin()->second.IncrementBackupFileRels;
			if (incrementBackupFileRels.size() != cm_LengthIncrementChain)
				throw EXCEPTION(BaseException());
		}
	}
}

