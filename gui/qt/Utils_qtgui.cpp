#include "qtgui_pch.h"
#include "Utils_qtgui.h"



std::list<ParsedSources>::iterator findBackupByName(MyBCopyFile* mbcFile, const wstring& backupName)
{
	return find_if(mbcFile->m_Backups.begin(), mbcFile->m_Backups.end(), [&backupName](const ParsedSources& backup)->bool
	{
		return backup.m_Name == backupName;
	});
}



QString EBackupStage2String(EBackupStage stage)
{
	switch (stage)
	{
	//case EBackupStage::Queuing: return QObject::tr("Queuing");
	case EBackupStage::Downloading: return QObject::tr("Downloading latest version");
	case EBackupStage::Extracting: return QObject::tr("Extracting latest version");
	case EBackupStage::Archiving: return QObject::tr("Archiving");
	case EBackupStage::Uploading: return QObject::tr("Uploading to storage(s)");
	default:
	{
		assert(0);
		throw EXCEPTION(BaseException());
	}
	}
}

QString EStorageStage2String(EStorageStage stage)
{
	switch (stage)
	{		
	case EStorageStage::Uploading: return QObject::tr("Uploading");
	case EStorageStage::RemovingOld: return QObject::tr("Removing old versions");
	case EStorageStage::PostUploadCheck: return QObject::tr("Checking the result");
	case EStorageStage::StorageFinish: return QObject::tr("Complete");
	default:
	{
		assert(0);
		throw EXCEPTION(BaseException());
	}
	}
}

QString EBackupError2String(EBackupError error)
{
	switch (error)
	{
	case EBackupError::NoError: return QObject::tr("Complete");
	case EBackupError::WithErrors: return QObject::tr("Complete with error(s)");	

	case EBackupError::NoFilesForBackup: return QObject::tr("No files for backup");
	case EBackupError::PostUploadCheck: return QObject::tr("Uploading failed");
	case EBackupError::TimeStampExists: return QObject::tr("Backup with same timestamp already exists");
	case EBackupError::Download: return QObject::tr("Downloading failed");

	case EBackupError::Extraction: return QObject::tr("Extraction of the old archive failed");
	case EBackupError::CreationArchive: return QObject::tr("Creation of the new archive failed");
	case EBackupError::NoChangesForIncrementalBackup: return QObject::tr("No changes for the new incremental backup");

	case EBackupError::Settings_SrcPaths: return QObject::tr("Bad settings. No sources for backup specified");
	case EBackupError::Settings_TemporaryDir: return QObject::tr("Bad settings. No temporary directory not available");
	case EBackupError::Settings_Storages: return QObject::tr("Bad settings. No storage specified");
	case EBackupError::Settings_Toolchain: return QObject::tr("Bad settings. No tool specified");
	case EBackupError::Settings_ToolNoPassword: return QObject::tr("Bad settings. Password not specified");
	case EBackupError::Settings_NoSystemRar: return QObject::tr("Bad settings. System rar executable not found");
	case EBackupError::Settings_NoSystem7z: return QObject::tr("Bad settings. System 7z executable not found");
	case EBackupError::Settings_NoReferenceStorageName: return QObject::tr("Bad settings. No reference storage for incremental backup");

	case EBackupError::Unknown: return QObject::tr("Unknown internal error");
	default:
	{
		assert(0);
		throw EXCEPTION(BaseException());
	}
	}
}


QString EntryTypeToString(Filters::EntryType entryType)
{
	switch (entryType)
	{
	case Filters::EntryType::Entry: return QObject::tr("entries");
	case Filters::EntryType::Dir: return QObject::tr("directories");
	case Filters::EntryType::File: return QObject::tr("files");
	default:
	{
		assert(0);
		throw EXCEPTION(BaseException());
	}
	break;
	}
}


QString AppendSpaceIfNotEmpty(const QString& s)
{
	if (!s.isEmpty())
		return s + " ";
	else
		return s;

}

void SpaceIfNotEmpty(QString& s)
{
	if (!s.isEmpty())
		s += " ";
}




QString GetFilterSpecificText(Filters::FilterBase* filter)
{
	QString result;	

	if (filter->IsInclusion())
		result += QObject::tr("Include");
	else
		result += QObject::tr("Exclude");

	result += " ";

	QString entryType = EntryTypeToString(filter->GetEntryType());

	result += entryType + " ";

	return result + FilterImplementatorSelector::Instance().GetFilterImplementator(filter)->GetSpecificText(filter);
}

Filters::EntryType GetComboBox_EntryType(QComboBox* comboBox)
{
	switch (comboBox->currentIndex())
	{
	case 0: return Filters::EntryType::File;
	case 1: return Filters::EntryType::Dir;
	case 2: return Filters::EntryType::Entry;
	default:
		assert(0);
		throw EXCEPTION(BaseException());
		break;
	}
}

bool GetComboBox_Inclusion(QComboBox* comboBox)
{
	return comboBox->currentIndex() == 0;
}

void SetComboBox_EntryType(QComboBox* comboBox, Filters::EntryType value)
{
	switch (value)
	{
	case Filters::EntryType::File: comboBox->setCurrentIndex(0); break;
	case Filters::EntryType::Dir: comboBox->setCurrentIndex(1); break;
	case Filters::EntryType::Entry: comboBox->setCurrentIndex(2); break;
	default:
		assert(0);
		throw EXCEPTION(BaseException());
		break;
	}
}

void SetComboBox_Inclusion(QComboBox* comboBox, bool value)
{
	return comboBox->setCurrentIndex(value ? 0 : 1);
}

std::type_index GetStorageTypeIndex(Storage* storage)
{
	{
		auto hlStorage = dynamic_cast<WebDavStorage*>(storage);
		if (hlStorage)
		{
			return typeid(WebDavStorage);
		}
	}

	{
		auto hlStorage = dynamic_cast<LocalDiskStorage*>(storage);
		if (hlStorage)
		{
			return typeid(LocalDiskStorage);
		}
	}

	assert(0);
	throw logic_error("not impl");
}

bool LessByPath(const SrcPathDesc& lhs, const SrcPathDesc& rhs)
{
	return lhs.m_Path < rhs.m_Path;
}


void RemoveSelectedRowsFromModel(QAbstractItemView* view)
{
	auto selectionModel = view->selectionModel();
	assert(selectionModel);
	QModelIndexList selectedRows = selectionModel->selectedRows();
	list<QPersistentModelIndex> persistentSelectedSrcs(selectedRows.begin(), selectedRows.end());

	auto model = view->model();
	assert(model);

	for (const QPersistentModelIndex& idx : persistentSelectedSrcs)
	{
		if (!idx.isValid())
			continue;

		model->removeRows(idx.row(), 1, idx.parent());
	}
}




void StorageImplementator::RegisterStorageImplementator(type_index implementableStorageTypeIndex, StorageImplementator* performer)
{
	assert(StorageImplementatorSelector::Instance().m_TypeIndex2Implementator.count(implementableStorageTypeIndex) == 0);
	StorageImplementatorSelector::Instance().m_TypeIndex2Implementator.emplace(implementableStorageTypeIndex, performer);
}

StorageImplementator* StorageImplementatorSelector::GetStorageImplementator(Storage* storage)
{
	return GetStorageImplementator(GetStorageTypeIndex(storage));
}

StorageImplementator* StorageImplementatorSelector::GetStorageImplementator(type_index storageTypeIndex)
{
	auto it = m_TypeIndex2Implementator.find(storageTypeIndex);
	assert(it != m_TypeIndex2Implementator.end());
	return it->second;
}

std::type_index StorageImplementatorSelector::GetStorageTypeIndex(const Storage* storage)
{
	{
		auto hlStorage = dynamic_cast<const WebDavStorage*>(storage);
		if (hlStorage)
		{
			return typeid(WebDavStorage);
		}
	}

	{
		auto hlStorage = dynamic_cast<const LocalDiskStorage*>(storage);
		if (hlStorage)
		{
			return typeid(LocalDiskStorage);
		}
	}

	assert(0);
	throw logic_error("not impl");
}

QString WebDavImplementator::GetSpecificText(Storage* storage) const
{
	return QObject::tr("Username:") + " " + QString::fromStdWString(hl_cast(storage)->m_UserName);
}

QString WebDavImplementator::GetAddress(Storage* storage) const
{
	return QString::fromStdWString(hl_cast(storage)->m_Url);
}

QString WebDavImplementator::GetTypeName(Storage* storage) const
{
	return QObject::tr("WebDav");
}

void WebDavImplementator::SerializeToXML(pugi::xml_node& xmlStorage, Storage* storage) const
{
	xmlStorage.append_attribute(L"type").set_value(L"WebDav");

	auto hlStorage = hl_cast(storage);
	xmlStorage.append_attribute(L"url").set_value(hlStorage->m_Url.c_str());
	xmlStorage.append_attribute(L"username").set_value(hlStorage->m_UserName.c_str());
	xmlStorage.append_attribute(L"password").set_value(hlStorage->m_Password.c_str());
	xmlStorage.append_attribute(L"backup_dirname").set_value(hlStorage->m_BackupDirName.c_str());	
}

const WebDavImplementator& g_WebDavImplementator = WebDavImplementator::Instance();



QString LocalDiskImplementator::GetSpecificText(Storage* storage) const
{
	return "";
}

QString LocalDiskImplementator::GetAddress(Storage* storage) const
{
	return QString::fromStdWString(hl_cast(storage)->m_Path);
}

QString LocalDiskImplementator::GetTypeName(Storage* storage) const
{
	return QObject::tr("Local Disk");
}


void LocalDiskImplementator::SerializeToXML(pugi::xml_node& xmlStorage, Storage* storage) const
{
	xmlStorage.append_attribute(L"type").set_value(L"LocalDisk");
	xmlStorage.append_attribute(L"path").set_value(hl_cast(storage)->m_Path.c_str());
}

const LocalDiskImplementator& g_LocalDiskImplementator = LocalDiskImplementator::Instance();




FilterImplementator* FilterImplementatorSelector::GetFilterImplementator(Filters::FilterBase* filter)
{
	return GetFilterImplementator(GetFilterTypeIndex(filter));
}

FilterImplementator* FilterImplementatorSelector::GetFilterImplementator(type_index filterTypeIndex)
{
	auto it = m_TypeIndex2Implementator.find(filterTypeIndex);
	assert(it != m_TypeIndex2Implementator.end());
	return it->second;
}

std::type_index FilterImplementatorSelector::GetFilterTypeIndex(const Filters::FilterBase* filter)
{
	using namespace Filters;	
	{
		auto hlFilter = dynamic_cast<const WildcardsFilter*>(filter);
		if (hlFilter)
		{
			return typeid(WildcardsFilter);
		}
	}

	{
		auto hlFilter = dynamic_cast<const RegexpsFilter*>(filter);
		if (hlFilter)
		{
			return typeid(RegexpsFilter);
		}
	}

	assert(0);
	throw logic_error("not impl");
}


void FilterImplementator::RegisterFilterImplementator(type_index implementableFilterTypeIndex, FilterImplementator* performer)
{
	assert(FilterImplementatorSelector::Instance().m_TypeIndex2Implementator.count(implementableFilterTypeIndex) == 0);
	FilterImplementatorSelector::Instance().m_TypeIndex2Implementator.emplace(implementableFilterTypeIndex, performer);
}



ToolImplementator* ToolImplementatorSelector::GetToolImplementator(Tool* tool)
{
	return GetToolImplementator(GetToolTypeIndex(tool));
}

ToolImplementator* ToolImplementatorSelector::GetToolImplementator(type_index toolTypeIndex)
{
	auto it = m_TypeIndex2Implementator.find(toolTypeIndex);
	assert(it != m_TypeIndex2Implementator.end());
	return it->second;

}

ToolImplementator* ToolImplementatorSelector::GetToolImplementatorByTypename(const QString& typeName)
{
	auto it = find_if(m_TypeIndex2Implementator.begin(), m_TypeIndex2Implementator.end(), [&typeName](auto& p)->bool
	{
		return p.second->GetTypeName() == typeName;
	});

	assert(it != m_TypeIndex2Implementator.end());
	return it->second;
}

std::type_index ToolImplementatorSelector::GetToolTypeIndex(const Tool* tool)
{
	{
		auto hlStorage = dynamic_cast<const ArchiverRar*>(tool);
		if (hlStorage)
		{
			return typeid(ArchiverRar);
		}
	}

	{
		auto hlStorage = dynamic_cast<const Archiver7z*>(tool);
		if (hlStorage)
		{
			return typeid(Archiver7z);
		}
	}

	assert(0);
	throw logic_error("not impl");
}

const std::map<std::type_index, ToolImplementator*>& ToolImplementatorSelector::GetMap() const
{
	return m_TypeIndex2Implementator;
}



void ToolImplementator::RegisterToolImplementator(type_index implementableToolTypeIndex, ToolImplementator* performer)
{
	assert(ToolImplementatorSelector::Instance().m_TypeIndex2Implementator.count(implementableToolTypeIndex) == 0);
	ToolImplementatorSelector::Instance().m_TypeIndex2Implementator.emplace(implementableToolTypeIndex, performer);
}


QString ArchiverRarImplementator::GetDefaultExecutablePath() const
{
	return QString::fromStdWString(GetSystemRarExeFilePath());
}

QString ArchiverRarImplementator::GetFileDialogFilter() const
{
	return QObject::tr("Rar executable (rar.exe)");
}

QString ArchiverRarImplementator::GetTypeName() const
{
	return "rar";
}

void ArchiverRarImplementator::SerializeToXML(pugi::xml_node& xmlTool, Tool* tool) const
{
	xmlTool.append_attribute(L"type").set_value(L"rar");
}

std::shared_ptr<Tool> ArchiverRarImplementator::MakeTool(const QString& exePath, const QString& passwd)
{
	return make_shared<ArchiverRar>(exePath.toStdWString(), passwd.toStdWString());
}

const ArchiverRarImplementator& g_ArchiverRarImplementator = ArchiverRarImplementator::Instance();


QString Archiver7zImplementator::GetDefaultExecutablePath() const
{
	return QString::fromStdWString(GetSystem7zExeFilePath());
}

QString Archiver7zImplementator::GetFileDialogFilter() const
{
	return QObject::tr("7z executable (7z.exe)");
}

QString Archiver7zImplementator::GetTypeName() const
{
	return "7z";
}

void Archiver7zImplementator::SerializeToXML(pugi::xml_node& xmlTool, Tool* tool) const
{
	xmlTool.append_attribute(L"type").set_value(L"7z");
}


std::shared_ptr<Tool> Archiver7zImplementator::MakeTool(const QString& exePath, const QString& passwd)
{
	return make_shared<Archiver7z>(exePath.toStdWString(), passwd.toStdWString());
}

const Archiver7zImplementator& g_Archiver7zImplementator = Archiver7zImplementator::Instance();