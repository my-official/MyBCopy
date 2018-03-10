#include "MyBCopy_pch.h"
#include "BackupBase.h"
#include "Utils.h"
#include "BackupJobExecuter.h"
#include "LocalDiskIOManager.h"
#include "InternetIOManager.h"
#include "UtilsMyBCopy.h"
#include "UtilsWindows.h"


BackupBase::BackupBase(const wstring& archiveName) : m_Name(archiveName)
{

}





template <typename Type>
void OverrideOption(Type& lhs, const Type& rhs)
{
	if (rhs)
	{
		lhs = rhs;
	}
}

template <typename Type>
void OverrideArrayOption(Type& lhs, const Type& rhs)
{
	if (!rhs.empty())
	{
		lhs = rhs;
	}
}


Settings Settings::Override(const Settings& lhs, const Settings& rhs)
{
	Settings result(lhs);		
	OverrideArrayOption(result.m_TemporaryDir, rhs.m_TemporaryDir);
	OverrideArrayOption(result.m_Storages, rhs.m_Storages);		
	OverrideOption(result.m_SchemeParams, rhs.m_SchemeParams);
	OverrideArrayOption(result.m_Toolchain, rhs.m_Toolchain);
	OverrideOption(result.m_Verification, rhs.m_Verification);
	return result;
}



std::list < std::shared_ptr<Storage> >::const_iterator Settings::FindStorageByName(const wstring& storageName) const
{
	auto referenceStorage_it = find_if(m_Storages.begin(), m_Storages.end(), [&](const shared_ptr<Storage>& a)->bool
	{
		return a->m_Name == storageName;
	});

	return referenceStorage_it;
}

std::shared_ptr<Storage> Settings::GetStorageByName(const wstring& storageName) const
{
	auto referenceStorage_it = FindStorageByName(storageName);

	if (referenceStorage_it != m_Storages.end())
		return *referenceStorage_it;
	else
		return nullptr;
}
