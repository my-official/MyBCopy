#include "stdafx.h"
#include "Reserver.h"


//const string DestListFile = "DestList.txt";

//const string VeraCryptContainerFilePath = "J:\\";

static TCHAR buffer[255*255 * 255];


void Reserver::Schtask()
{
	//if (system(nullptr) == 0)
	//	throw EXCEPTION(ErrnoException());

	//string rarCmd = "schtasks /Create /SC DAILY /TN myBackup /TR /F  ";

	//cout << rarCmd << endl;
	
	//if (system(rarCmd.c_str()) != 0)
	//	throw EXCEPTION(ErrnoException());
}

void Reserver::RunCMD(const string& cmd)
{
	cout << cmd << endl << endl;

	auto returnCode = system(cmd.c_str());
	
	if (returnCode != 0)
	{
		cout << "Return Code is " << returnCode << endl;
		throw EXCEPTION(ErrnoException());
	}
		
}

template <typename Func>
void ForEachNonEmptySubword(const string& filetypes, char delimiter, Func func)
{
	auto length = filetypes.length();
	if (length == 0)
		return;

	auto pos = filetypes.find(delimiter);
	
	if (pos != string::npos)
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
			
		} while (pos != string::npos);

		func(&filetypes[start], length - start);
	}
	else
	{
		func(&filetypes[0], length);
	}
}

void Reserver::ParsePath(const string& line, const string& srcFile, size_t currLine)
{
	auto pos = line.rfind('\\');
	
	if (pos == string::npos)
		throw EXCEPTION(ParseException(srcFile,currLine));

	if (pos == 0)
		throw EXCEPTION(ParseException(srcFile, currLine));

	string path = line.substr(0, pos);
	if (path.empty())
		throw EXCEPTION(ParseException(srcFile, currLine));

	bool exclusion = path[0] == '-';

	if (exclusion)
	{
		if (path.size() == 1)
			throw EXCEPTION(ParseException(srcFile, currLine));

		path = path.substr(1);
	}	

	if (pos + 1 < line.size())
	{
		string filetypes = line.substr(pos + 1);

		ForEachNonEmptySubword(filetypes, ';', [&](const char* filetype, string::size_type filetypeSize)
		{
			if (filetype[0] == '-')
			{
				if (filetypeSize == 1)
					throw EXCEPTION(ParseException(srcFile, currLine));

				if (exclusion)
					throw EXCEPTION(ParseException(srcFile, currLine));

				filetype = filetype + 1;
				m_Backups.back().ExclFiles.push_back(path + string("\\") + string(filetype, filetypeSize - 1));
			}
			else
			{
				if (exclusion)
					m_Backups.back().ExclFiles.push_back(path + string("\\") + string(filetype, filetypeSize));
				else
					m_Backups.back().SrcFiles.push_back(path + string("\\") + string(filetype, filetypeSize));
			}
		});	
	}
	else
	{
		if (exclusion)
			m_Backups.back().ExclFiles.push_back(path);
		else
			m_Backups.back().SrcFiles.push_back(path);		
	}
}


void Reserver::LoadSourcesFromFile(const string& srcFile)
{
	ifstream srcFileStream;
	

	std::experimental::filesystem::path srcFileFullPath(srcFile);

	if (srcFileFullPath.is_absolute())
		srcFileStream.open(srcFile);
	else
		srcFileStream.open(m_ExePath + "\\" + srcFile);	

	if (!srcFileStream)
		return;

	size_t currLine = 0;	
	string line;

	while (!srcFileStream.eof())
	{		
		getline(srcFileStream, line);
		++currLine;

		smatch matches;
				
		if ( !regex_search(line, matches, regex( "\\S" ))  )
			continue;

		if (matches.size() != 1)
			throw EXCEPTION( ParseException(srcFile, currLine) );

		auto& match = matches[0];

		switch (match.str()[0])
		{
			case ';': break;
			case '#':
			{
				cmatch includedFileMatches;
				if (!regex_match(line.c_str() + matches.position() + match.length(), includedFileMatches, regex(R"regex(include\s(\w.*)(?=\r?$))regex" )))
					throw EXCEPTION(ParseException(srcFile,currLine));
				
				if (includedFileMatches.size() != 2)
					throw EXCEPTION(ParseException(srcFile,currLine));

				LoadSourcesFromFile(includedFileMatches[1].str());
			
			}
			break;
			case '<':
			{
				cmatch sectionNameMatches;
				if (!regex_match(line.c_str() + matches.position() + match.length(), sectionNameMatches, regex(R"((.+)>)")))
					throw EXCEPTION(ParseException(srcFile,currLine));

				if (sectionNameMatches.size() != 2)
					throw EXCEPTION(ParseException(srcFile,currLine));
				
				m_Backups.emplace_back();
				Backup& backup = m_Backups.back();
				backup.ArchiveName = sectionNameMatches[1];
			}
			break;

			default:
			{
				if (m_Backups.empty())
					throw EXCEPTION(ParseException(srcFile,currLine));

				ParsePath(line.c_str() + matches.position(), srcFile, currLine);
			}
			break;
		}		
	}
}

void Reserver::PrepareSourceFileListForRar(Backup& backup)
{
	namespace fs = std::experimental::filesystem;		

	fs::create_directories(GetTemporaryDirPath(backup));

	ofstream outfile( GetTemporaryRarSourceListFile(backup));

	if (!outfile)
		throw EXCEPTION( ErrnoException() );

	for (auto& srcfile : backup.SrcFiles)
	{
		outfile << "\"" << srcfile << "\"" << endl;
	}

	if (!backup.ExclFiles.empty())
	{
		outfile.close();

		outfile.open( GetTemporaryRarExclusionListFile(backup));

		if (!outfile)
			throw EXCEPTION(ErrnoException());

		for (auto& srcfile : backup.ExclFiles)
		{
			outfile << "\"" << srcfile << "\"" << endl;
		}
	}
}

void Reserver::CleanupRar(Backup& backup)
{
	namespace fs = std::experimental::filesystem;

	fs::remove(GetTemporaryRarSourceListFile(backup));
	fs::remove(GetTemporaryRarExclusionListFile(backup));
	fs::remove(GetTemporaryRarFile(backup));
}

void Reserver::AddPath(const string& newpath)
{
	char* path = nullptr;

	if (_dupenv_s(&path, nullptr, "PATH") != 0)
		throw EXCEPTION( ErrnoException());
	
	if (!path)
		throw EXCEPTION(ErrnoException());
	
				
	
	if (_putenv_s("PATH", (string(path) + ";" + newpath).c_str()) != 0)
		throw EXCEPTION(ErrnoException());

	free(path);
}

std::string Reserver::GetTemporaryDirPath(const Backup& backup) const
{
	return m_TemporaryDir + "\\" + backup.ArchiveName;
}

std::string Reserver::GetTemporaryRarFile(const Backup& backup) const
{	
	return GetTemporaryDirPath(backup) + "\\" + m_Now + ".rar";
}

std::string Reserver::GetTemporaryRarSourceListFile(const Backup& backup) const
{
	return GetTemporaryDirPath(backup) + "\\sources.txt";
}

std::string Reserver::GetTemporaryRarExclusionListFile(const Backup& backup) const
{
	return GetTemporaryDirPath(backup) + "\\exclusion.txt";
}

std::string Reserver::GetTemporary7zEncryptedFile(const Backup& backup) const
{
	return GetTemporaryDirPath(backup) + "\\" + m_Now + ".7z";
}

std::string Reserver::GetCloudDestPath(const Backup& backup) const
{
	return m_RCopyDirPath + "\\" + backup.ArchiveName;	
}

std::string Reserver::GetCloudDestFile(const Backup& backup) const
{
	return GetCloudDestPath(backup) + "\\" + m_Now + ".7z";
}

void Reserver::MakeRarArchive(const Backup& backup)
{
	string passwd = m_SettingsIni.top()("Rar")["Password"];
	string SourcesListFile = m_SettingsIni.top()("General")["SourcesListFile"];
	
	string cmd = "rar a -hp\"" + passwd + "\" -idq -m5 -r -s -t -y ";
		
	if (!backup.ExclFiles.empty())
	{
		cmd += "-x@\"" + GetTemporaryRarExclusionListFile(backup) + "\"";
	}

	cmd += " -- \"" + GetTemporaryRarFile(backup) + "\" @\"" + GetTemporaryRarSourceListFile(backup) + "\"";

	RunCMD(cmd);	
}

void Reserver::Make7zArchive(const Backup& backup)
{
	string passwd = m_SettingsIni.top()("7z")["Password"];
	
	RunCMD("7z a -p\"" + passwd + "\" -mhe=on -mx=0 -bso0 -bsp0 -y \"" + GetTemporary7zEncryptedFile(backup) + "\" \"" + GetTemporaryRarFile(backup) + "\"");
	RunCMD("7z t -p\"" + passwd + "\" -bso0 -bsp0 -y \"" + GetTemporary7zEncryptedFile(backup) + "\" *");			
}



std::string Reserver::Now()
{
	time_t rawtime;
	struct tm  timeinfo;
	char buffer[80];

	time(&rawtime);

	if (localtime_s(&timeinfo, &rawtime) != 0)
		throw EXCEPTION(ErrnoException());

	strftime(buffer, 80, "%F_%H-%M", &timeinfo);
	//puts(buffer);

	return buffer;
}




void Reserver::UploadToCloud(const string& IniFileSectionName)
{
	if (m_SettingsIni.top().sections.count(IniFileSectionName) == 0)
	{
		cout << "No section " << IniFileSectionName << endl;
		return;
	}


	cout << endl << "Uploading To " << IniFileSectionName << endl;
	

	MountWebDavShare(IniFileSectionName);
		
	
	for (auto& backup : m_Backups)
	{
		string destDir = GetCloudDestPath(backup);

		if (!std::experimental::filesystem::exists(destDir))
		{
			if (!std::experimental::filesystem::create_directories(destDir))
				throw EXCEPTION(SystemException());
			
		}

		string cmd = "copy "; // /V - верифицировать

		if (m_SettingsIni.top()("General").values.count("Verification"))
		{
			string v = m_SettingsIni.top()("General")["Verification"];
			if ( v == "on" || v == "1" || v == "yes" || v == "true")
				cmd += "/V ";
		}
		cmd += "/Y \"" + GetTemporary7zEncryptedFile(backup) + "\" \"" + destDir + "\" ";			 


		RunCMD(cmd);

		RemoveOldArchives(backup);
	}		

	UnmountWebDavShare();

	cout << "Uploaded to " << IniFileSectionName << endl << endl;
}



void Reserver::MountWebDavShare(const string& IniFileSectionName)
{
	string Url = m_SettingsIni.top()(IniFileSectionName)["Url"];
	string UserName = m_SettingsIni.top()(IniFileSectionName)["UserName"];
	string Password = m_SettingsIni.top()(IniFileSectionName)["Password"];

	string Disk = m_SettingsIni.top()("General")["CloudShareDisk"];		

	RunCMD("net use " + Disk + " \"" + Url + "\" /USER:\"" + UserName + "\" \"" + Password + "\" /PERSISTENT:NO");	
}

void Reserver::RemoveOldArchives(const Backup& backup)
{
	namespace fs = std::experimental::filesystem;

	set<wstring> files;

	for (auto& p : fs::directory_iterator(GetCloudDestPath(backup) ) )
	{
		if (p.path().extension() == L".7z" && fs::is_regular_file(p))
		{
			files.insert(p.path().c_str());
		}
	}

	if (files.size() > m_MaxNumOfOldArchives)
	{
		cout << "Removing old files" << endl;
		for (unsigned int c = 0, size = files.size() - m_MaxNumOfOldArchives; c < size; c++)
		{
			auto it = files.begin();
			advance(it, c);
			string ansiFileName = wstring_convert< codecvt<wchar_t, char, mbstate_t> >().to_bytes(it->c_str());
			cout << "Removing " << ansiFileName << endl;			
			fs::remove(*it);
		}
	}
}

void Reserver::UnmountWebDavShare()
{
	string Disk = m_SettingsIni.top()("General")["CloudShareDisk"];		
	RunCMD("net use " + Disk + " /DELETE");	
}


void Reserver::Process()
{
	if (system(nullptr) == 0)
		throw EXCEPTION(ErrnoException());
	
	AddPath(m_SettingsIni.top()("Rar")["DirPath"]);
	AddPath(m_SettingsIni.top()("7z")["DirPath"]);


	m_TemporaryDir = m_SettingsIni.top()("General")["TemporaryDir"];
	
	cout << "TemporaryDir is " << m_TemporaryDir << endl;

	if (!std::experimental::filesystem::exists(m_TemporaryDir))
	{
		cout << "TemporaryDir not found" << endl;
		throw SystemException();
	}
	

	m_MaxNumOfOldArchives =  stoi( m_SettingsIni.top()("General")["MaxNumOfOldArchives"] );

	if (m_MaxNumOfOldArchives < 5 )
		m_MaxNumOfOldArchives = 5;

	cout << "MaxNumOfOldArchives " << m_MaxNumOfOldArchives << endl;
	//Schtask();

	m_Now = Now();
	
	LoadSourcesFromFile(m_SettingsIni.top()("General")["SourcesListFile"] );

	if (m_Backups.empty())
	{
		cout << "No files for backup" << endl;
		return;
	}		

	for (auto& backup : m_Backups)
	{	
		PrepareSourceFileListForRar(backup);				
		MakeRarArchive(backup);						
		Make7zArchive(backup);
		CleanupRar(backup);	
	}

	string Disk = m_SettingsIni.top()("General")["CloudShareDisk"];

	if (std::experimental::filesystem::exists(Disk))
	{
		UnmountWebDavShare();
	}

	m_RCopyDirPath = Disk + "\\" + m_SettingsIni.top()("General")["CloudRCopyDirName"];

	UploadToCloud("YandexDisk");
	UploadToCloud("OneDrive");
	UploadToCloud("DropBox");		
}


string Reserver::GetThisProgramExeDirPath()
{
	ZeroMemory(buffer, sizeof(buffer));

	if (GetModuleFileName(nullptr, buffer, sizeof(buffer)) == 0)
		throw EXCEPTION( SystemException() );

	namespace fs = std::experimental::filesystem;
	wstring exeFilePathW = fs::path(buffer).parent_path();
	return wstring_convert< codecvt<wchar_t, char, mbstate_t> >().to_bytes(exeFilePathW.c_str());
}

Reserver::Reserver() : m_ExePath(GetThisProgramExeDirPath()), m_SettingsIni( (m_ExePath + "\\Settings.ini").c_str() )
{
}


Reserver::~Reserver()
{
}

ParseException::ParseException(const string& file, size_t line) : m_File(file) , m_Line(line)
{

}

void ParseException::FormattedMessageLine()
{
	TextLine("Parse error at " + m_File + ":" + to_string(m_Line));
}



//void Reserver::CreateVeraCryptContainer(string veraCryptContainerFile)
//{
//	//const string newPath = R"(C:\Program Files\VeraCrypt)";
//
//	//string cmd = "cd //D " + VeraCryptContainerFilePath + R"(" && "VeraCrypt Format.exe" /create ")" + veraCryptContainerFileName + R"(" /size 25M /password "a b ы ) " /encryption Twofish /filesystem FAT /force /silent)";
//

//}