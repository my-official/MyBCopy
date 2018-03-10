#pragma once
#include "Utils.h"




//enum class IncrementBackupMode
//{
//	Increment,
//	FromRegularOnly
//};



class Reserver;
class TraversedBackup;



class MyBCopy_API RegularScheme
{
public:
	virtual ~RegularScheme() = default;	
	unsigned int m_NumStoredOldRegularBackups;
	
};

class MyBCopy_API IncrementalScheme : public RegularScheme
{
public:
	unsigned int m_NumIncrementBranches;
	unsigned int m_LengthIncrementChain;
	unsigned int m_NumStoredOldIncrementBranches;

	wstring m_ReferenceStorageName;
};


class Storage;
class Tool;



class MyBCopy_API Settings
{
public:	
	wstring m_TemporaryDir;

	list < shared_ptr<Storage> > m_Storages;

	shared_ptr<RegularScheme> m_SchemeParams;

	list < shared_ptr<Tool> > m_Toolchain;

	boost::optional<bool> m_Verification;

	static Settings Override(const Settings& lhs, const Settings& rhs);	
	
	list < shared_ptr<Storage> >::const_iterator  FindStorageByName(const wstring& storageName) const;
	shared_ptr<Storage> GetStorageByName(const wstring& storageName) const;
};


class MyBCopy_API BackupBase
{
public:
	BackupBase() = default;
	BackupBase(const wstring& archiveName);
	virtual ~BackupBase() = default;

	wstring m_Name;	
	Settings m_Settings;

	virtual bool Empty() = 0;	

	
};
