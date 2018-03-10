#pragma once

std::wstring NowTimestampAsString();
wstring ExtractTimestampFromFilename(const wstring& filename);
wstring ExtractRegularTimeStampFromIncrementDirName(const wstring& incrementDirName);
//wstring ExtractIncrementTimeStamp(const wstring& filename);




wstring MakeRegularArchiveFileName(const wstring& nowTimestamp, const wstring& ext = L".7z");
wstring MakeIncrementArchiveFileName(const wstring& nowTimestamp, const wstring& ext = L".7z");
wstring MakeIncrementArchivesDirName(const wstring& nowTimestamp, const wstring& regularTimestamp);


#define L_BkTimeStampMask LR"(\d\d\d\d-\d\d-\d\d_\d\d-\d\d-\d\d)"
//#define L_BkTimeStampExample LR"(2000-01-01_00-00)"
//constexpr wchar_t L_BkTimeStampMask[] = LR"(\d\d\d\d-\d\d-\d\d_\d\d-\d\d)";
constexpr wchar_t L_BkTimeStampExample[] = LR"(2000-01-01_00-00-00)";

wstring GetFreeMountPoint();

//struct LastRegularBackupInfo;
//LastRegularBackupInfo EnumLastBackups(const wstring& path);

struct RegularBackupList;
RegularBackupList EnumBackupFiles(const wstring& path);



using RemoveList = list< wstring >; //путь
RemoveList MakeRemoveListForOldRegularBackup(const RegularBackupList& enumeratedList, unsigned int numStoredOldRegularBackups);



fs::file_time_type cvtFilelastWriteTime(const string& dur);

fs::file_time_type date2file_time_type(const wstring& nowTimestamp);

class BackupBase;







MyBCopy_API wstring GetSystemRarExeFilePath();

MyBCopy_API wstring GetSystem7zExeFilePath();


