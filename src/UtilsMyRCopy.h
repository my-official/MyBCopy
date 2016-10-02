#pragma once

std::wstring NowTimestampAsString();
wstring ExtractTimestampFromFilename(const wstring& filename);
wstring ExtractRegularTimestampFromDeltaBackup(const wstring& filename);
wstring ExtractDeltaTimeStampFromIncrementDirName(const wstring& incrementDirName);
wstring ExtractIncrementTimeStamp(const wstring& filename);




wstring MakeRegularArchiveFileName(const wstring& nowTimestamp, const wstring& ext = L".7z");
wstring MakeDeltaArchiveFileName(const wstring& nowTimestamp, const wstring& prevRegularTimestamp, const wstring& ext = L".7z");
wstring MakeIncrementArchiveFileName(const wstring& nowTimestamp, const wstring& ext = L".7z");
wstring MakeIncrementArchivesDirName(const wstring& nowTimestamp, const wstring& prevDeltaTimestamp);


#define L_BkTimeStampMask LR"(\d\d\d\d-\d\d-\d\d_\d\d-\d\d)"
//#define L_BkTimeStampExample LR"(2000-01-01_00-00)"
//constexpr wchar_t L_BkTimeStampMask[] = LR"(\d\d\d\d-\d\d-\d\d_\d\d-\d\d)";
constexpr wchar_t L_BkTimeStampExample[] = LR"(2000-01-01_00-00)";

wstring GetFreeMountPoint();

//struct LastRegularBackupInfo;
//LastRegularBackupInfo EnumLastBackups(const wstring& path);

struct RegularBackupList;
RegularBackupList EnumBackupFiles(const wstring& path);

void RemoveOldBackupFiles(const RegularBackupList& enumeratedList, const  wstring& parantPath);

using RemoveList = list< wstring >; //путь
RemoveList MakeRemoveListForOldBackupFiles(const RegularBackupList& enumeratedList);