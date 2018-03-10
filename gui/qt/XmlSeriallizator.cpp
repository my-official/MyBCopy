#include "qtgui_pch.h"
#include "XmlSeriallizator.h"
#include "Utils_qtgui.h"

using namespace pugi;

const wchar_t* gc_MyBCopyFileVersion = L"2.1";

const wchar_t* bool2str(bool val)
{
	if (val)
		return L"on";
	else
		return L"off";
}

wstring depth2str(uint32_t depth)
{
	if (depth == SrcPathDesc::Depthless)
		return L"nolimit";
	else
		return to_wstring(depth);
}


static void SaveToolchain(xml_node& xmlToolchain, const list<shared_ptr<Tool>>& toolchain)
{	
	for (auto& pTool : toolchain)
	{
		auto tool = pTool.get();
		auto xmlNode = xmlToolchain.append_child(L"tool");
		
		ToolImplementator* implementator = ToolImplementatorSelector::Instance().GetToolImplementator(tool);
		implementator->SerializeToXML(xmlNode, tool);

		//xmlNode.append_attribute(L"type").set_value(L"rar");

		if (!tool->m_ExecutablePath.empty())
		{
			xmlNode.append_attribute(L"path").set_value(tool->m_ExecutablePath.c_str());
		}

		if (!tool->m_Password.empty())
		{
			xmlNode.append_attribute(L"password").set_value(tool->m_Password.c_str());
		}
	}
}

void SaveStorages(xml_node& xmlStorages, const list < shared_ptr<Storage> >& storages)
{
	for (auto& s : storages)
	{		
		Storage* storage = s.get();

		auto xmlNode = xmlStorages.append_child(L"storage");
		
		xmlNode.append_attribute(L"name").set_value(storage->m_Name.c_str());

		StorageImplementator* storageImpl =  StorageImplementatorSelector::Instance().GetStorageImplementator(storage);		
		storageImpl->SerializeToXML(xmlNode, storage);
	}
}

void SaveScheme(xml_node& xmlScheme, const shared_ptr<RegularScheme>& pScheme)
{
	RegularScheme*  scheme = pScheme.get();

	auto xmlAttrType = xmlScheme.append_attribute(L"type");		

	IncrementalScheme* incrementalScheme = dynamic_cast<IncrementalScheme*>(scheme);
	if (!incrementalScheme)
	{
		xmlAttrType.set_value(L"regular");		
	}
	else
	{
		xmlAttrType.set_value(L"incremental");
		
		xmlScheme.append_child(L"num_increment_branches").text().set(incrementalScheme->m_NumIncrementBranches);		
		xmlScheme.append_child(L"length_increment_chain").text().set(incrementalScheme->m_LengthIncrementChain);
		
		if (!incrementalScheme->m_ReferenceStorageName.empty())
			xmlScheme.append_child(L"reference_storage").text().set(incrementalScheme->m_ReferenceStorageName.c_str());
	}
	
	xmlScheme.append_child(L"num_stored_old_regular_backups").text().set(scheme->m_NumStoredOldRegularBackups);
}


void SaveSettings(xml_node& xmlDoc, const Settings& settings)
{
	if (!settings.m_TemporaryDir.empty())
	{
		xmlDoc.append_child(L"temporary_dir").text().set(settings.m_TemporaryDir.c_str());
	}

	/*if (settings.m_Verification.is_initialized())
	{
		xmlDoc.append_child(L"verification").text().set(bool2str(*settings.m_Verification));
	}*/

	if (!settings.m_Storages.empty())
	{
		auto xmlNode = xmlDoc.append_child(L"storages");
		SaveStorages(xmlNode, settings.m_Storages);
	}

	if (settings.m_SchemeParams)
	{
		auto xmlNode = xmlDoc.append_child(L"scheme");
		SaveScheme(xmlNode, settings.m_SchemeParams);
	}

	if (!settings.m_Toolchain.empty())
	{
		auto xmlNode = xmlDoc.append_child(L"toolchain");
		SaveToolchain(xmlNode, settings.m_Toolchain);
	}	
}

void SaveFilters(xml_node& xmlSrcPath, const SrcPathDesc& srcPath)
{
	map<Filters::EntryType, map<bool, xml_node> > xmlAllFilters = {

		make_pair<Filters::EntryType, map<bool, xml_node> >(Filters::EntryType::Entry,{
			make_pair(true, xmlSrcPath.append_child(L"include_entry_if")),
			make_pair(false, xmlSrcPath.append_child(L"exclude_entry_if"))
		}),

		make_pair<Filters::EntryType, map<bool, xml_node> >(Filters::EntryType::Dir,{
			make_pair(true, xmlSrcPath.append_child(L"include_dir_if")),
			make_pair(false, xmlSrcPath.append_child(L"exclude_dir_if"))
		}),

		make_pair<Filters::EntryType, map<bool, xml_node> >(Filters::EntryType::File,{
			make_pair(true, xmlSrcPath.append_child(L"include_file_if")),
			make_pair(false, xmlSrcPath.append_child(L"exclude_file_if"))
		})
	};


	for (auto& pFilter : srcPath.m_Filters)
	{
		auto filter = pFilter.get();
		auto& xmlFilters = xmlAllFilters[filter->GetEntryType()][filter->IsInclusion()];
		FilterImplementatorSelector::Instance().GetFilterImplementator(filter)->SerializeToXML(xmlFilters, filter);
	}

	for (auto& entry2map : xmlAllFilters)
	{
		for (auto& inclusion2node : entry2map.second)
		{
			auto& xmlFilters = inclusion2node.second;
			if (xmlFilters.first_child().empty())
			{
				xmlSrcPath.remove_child(xmlFilters.name());
			}
		}
	}
}

void SaveSrcPath(xml_node& xmlBackup, const SrcPathDesc& srcPath)
{
	auto xmlSrcPath = xmlBackup.append_child(L"src");
	xmlSrcPath.append_attribute(L"path").set_value(srcPath.m_Path.c_str());

	if (!srcPath.m_Include)
		xmlSrcPath.append_attribute(L"exclusion").set_value(L"1");
	
	if (srcPath.m_Depth != SrcPathDesc::Depthless)
		xmlSrcPath.append_attribute(L"depth").set_value( srcPath.m_Depth );
	
	SaveFilters(xmlSrcPath, srcPath);	

	for (auto& subSrcPath : srcPath.m_SubSrcPathDescs)
		SaveSrcPath(xmlSrcPath, subSrcPath);	
}

void SaveBackup(xml_node& xmlDoc, const ParsedSources& backup)
{
	auto xmlBackup = xmlDoc.append_child(L"backup");
	
	xmlBackup.append_attribute(L"name").set_value(backup.m_Name.c_str());
	xmlBackup.append_attribute(L"comment").set_value(backup.m_Comment.c_str());

	SaveSettings(xmlBackup, backup.m_Settings);

	for (auto& srcPath : backup.m_SrcPaths)
		SaveSrcPath(xmlBackup, srcPath);	
}

bool SaveMyBCopyFile(const wstring& file, const MyBCopyFile& mbcFile)
{	
	xml_document xmlRoot;	

	auto xmlMyBCopy = xmlRoot.append_child(L"mybcopy");
	xmlMyBCopy.append_attribute(L"version").set_value(gc_MyBCopyFileVersion);
	
	SaveSettings(xmlMyBCopy, mbcFile.m_Settings);

	for (auto backup : mbcFile.m_Backups)
	{		
		backup.NormalizeRecursive();
		SaveBackup(xmlMyBCopy, backup);
	}

	return xmlRoot.save_file(file.c_str());	
}