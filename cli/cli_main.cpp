#include "cli_pch.h"

class CmdParser
{
public:
	CmdParser(int argc, wchar_t* argv[]);

	bool m_Parsed;
	wchar_t m_CommandSwitch;

	wstring m_Source;
	map<wstring, wstring> m_Options;

 const vector<wstring> m_OptionValueRequired = {
		L"o",
	};

	void PrintUsage();
	void PrintVersion();	
private:

	bool isOptionValueRequired(const wstring& arg);
	bool ParseCmdOptionsAndFiles(int argc, wchar_t* argv[]);	


};

CmdParser::CmdParser(int argc, wchar_t* argv[]) : m_Parsed(false),
m_CommandSwitch(0)
{
	if (argc < 2)
		return;

	auto lenArg1 = wcslen(argv[1]);
	if (lenArg1 > 2)
		return;

	if (lenArg1 == 2)
	{
		if (wcscmp(argv[1], L"-v") == 0)
		{
			m_CommandSwitch = L'v';
		}
		else
		{
			return;
		}
	}
	else
	{
		m_CommandSwitch =  toupper( argv[1][0] );
	}

	if (!ParseCmdOptionsAndFiles(argc, argv))
		return;

	m_Parsed = true;
}

void CmdParser::PrintUsage()
{
	WriteToStdout(LR"(
		**MyBCopy [A|R|V] [options] [file.mybcopy]
		MyBCopy -v		Print version
		A - create new backup for specified files (or user-default file if unspecified)
		R - restore backup(s)
		V - verification backup(s)

		[options]
		General Options:
		-------------------------------------------------
		-q Quite mode (Supress all output)


		New Backup Options:
		-------------------------------------------------
		-v Run verification after backup creation done


		Restore Backup Options:
		-------------------------------------------------
		-o restore to directory (default is origin place)
		-prar '%value%' specify the rar password
		-p7z '%value%' specify the 7z password
		-p%ToolName% '%value%' not implemented
		-s '%StorageName%' specify the storage that will use as backup source
		-b '%FileRelPath%' specify the backup archive file
		-m	['y'|'n'|'d'] specify the overwrite mode


		Verification Backup Options:
		-------------------------------------------------
		---No options---



		Return Codes:
		0 - no errors

		)");
}

void CmdParser::PrintVersion()
{
	WriteToStdout(L"2.1");
}

bool CmdParser::isOptionValueRequired(const wstring& arg)
{
	return find(m_OptionValueRequired.begin(), m_OptionValueRequired.end(), arg) != m_OptionValueRequired.end();
}

bool CmdParser::ParseCmdOptionsAndFiles(int argc, wchar_t* argv[])
{
	wstring optionName;

	for (int c = 2; c < argc; c++)
	{
		wstring arg = argv[c];

		if (arg.empty())
			continue;//такого не должно быть

		if (arg[0] == L'-')
		{
			if (optionName.empty())
			{
				return false;
			}

			arg = arg.substr(1);

			if (isOptionValueRequired(arg))
			{
				optionName = arg;
			}
		}
		else
		{
			TrimQuotes(arg);
			if (!optionName.empty())
			{
				m_Options[optionName] = arg;
			}
			else
			{
				m_Source = move(arg);
				if (c != argc)
					return false;				
			}
		}
	}

	return true;
}


int wmain(int argc, wchar_t* argv[])
{
	int result = 0;

	try
	{
		locale::global(locale(""));		
		if (_wsystem(nullptr) == 0)
			throw EXCEPTION(ErrnoException());


		
		CmdParser cmd(argc, argv);
		if (!cmd.m_Parsed)
		{
			cmd.PrintUsage();
			return 0;
		}


		switch (cmd.m_CommandSwitch)
		{
			case L'A':
			{
				Notificators notificators;

				notificators.m_ProgressNotificator = [&](const ParsedSources& backup, EBackupStage stage)
				{
					wstring message = L"Backup '" + backup.m_Name + L"' progress: ";

					switch (stage)
					{
						//case EBackupStage::Queuing: return QObject::tr("Queuing");
					case EBackupStage::Downloading: message += L"Downloading latest version"; break;
					case EBackupStage::Extracting: message += L"Extracting latest version"; break;
					case EBackupStage::Archiving: message += L"Archiving"; break;
					case EBackupStage::Uploading: message += L"Uploading to storage(s)"; break;
					default:
					{
						assert(0);
						throw EXCEPTION(BaseException());
					}
					}

					WriteToStdout(message);
				};

				notificators.m_FinishNotificator = [&](const ParsedSources& backup, EBackupError error)
				{
					wstring message = L"Backup '" + backup.m_Name + L"': ";

					if (error != EBackupError::NoError)
						message += L"Complete";
					else
						message += L"Error. Code is " + to_wstring(int(error));	

					WriteToStdout(message);
				};

				notificators.m_ErrorStorageNotificator = [&](const ParsedSources& backup, const wstring& storageName, EBackupError error)
				{
					wstring message = L"Backup '" + backup.m_Name + L"': Storage '" + storageName + L"' error is " + to_wstring(int(error));					
					WriteToStdout(message);
				};

				notificators.m_ProgressStorageNotificator = [&](const ParsedSources& backup, const wstring& storageName, EStorageStage stage)
				{
					wstring message = L"Backup '" + backup.m_Name + L"': Storage '" + storageName + L"' stage is " + to_wstring(int(stage));
					WriteToStdout(message);
				};

				Reserver r;
				MyBCopyFile file;
				file.LoadFromXMLFile(cmd.m_Source);
				r.Backup(file, notificators);				
			}
			break;
			case L'R':
			{
			/*	if (cmd.m_Sources.empty())
					cmd.m_Sources.push_back(L"src.mybcopy");


				list<MyBCopyFile> myBCopyFiles = LoadMyBCopyFiles(cmd.m_Sources);

				RestoringParams params;
				if (cmd.m_Options.count(L"o"))
				{
					params.m_OutputDir = cmd.m_Options[L"o"];
					params.CreateDiskDirectory = true;
				}

				if (cmd.m_Options.count(L"s"))
				{
					params.m_StorageName = cmd.m_Options[L"s"];
					if (cmd.m_Options.count(L"b"))
					{
						params.m_BackupFileRel = cmd.m_Options[L"b"];
					}
				}
				

				Reserver r;
				r.Restore(myBCopyFiles, params);*/
				WriteToStdout("Not Implemented");
			}
			break;
			case L'V':
			{
				WriteToStdout("Not Implemented");
			}
			break;
			case L'v':
			{
				WriteToStdout("Not Implemented");
			}
			break;
			default:
			{
				cmd.PrintUsage();
			}
			break;
		}
	
	}
	catch (BaseException& e)
	{
		WriteToStderr(wstring(L"BaseException: ") + e.ErrorMessage());
		result = -1;
	}
	catch (range_error& e)
	{
		WriteToStderr(string("range_error: ") + e.what());
		result = -1;
	}
	catch (system_error& e)
	{
		WriteToStderr(string("system_error: ") + e.what());
		WriteToStderr(string("Error: ") + e.what());
		WriteToStderr(string("Code: ") + to_string(e.code().value()));
		WriteToStderr(string("Category: ") + e.code().category().name());
		WriteToStderr(string("Message: ") + e.code().message());

		result = -1;
	}
	catch (std::exception& e)
	{
		WriteToStderr(string("std::exception: ") + e.what());
		result = -1;
	}
	catch (...)
	{
		WriteToStderr(L"unknown exception");
		result = -1;
	}

	if (result == 0)
	{
		WriteToStdout(L"Success finished!!! Press any key");
	}
	else
	{
		WriteToStdout(L"Finished with error(s)!!! Press any key");
	}

	//wchar_t symbol;
	//wcin >> symbol;
	return result;	
}


