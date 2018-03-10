#include "MyBCopy_pch.h"
#include "XmlLoader.h"
#include "ParsedSources.h"
#include "UtilsMyBCopy.h"
#include "Storage.h"
#include "Tool.h"





MyBCopyFile::XmlLoader::XmlLoader(const wstring& srcFile) : m_SrcFile(srcFile)
{

}


MyBCopyFile MyBCopyFile::XmlLoader::Load()
{
	using namespace pugi;
	xml_document xmlDoc;

	auto parseResult = xmlDoc.load_file(m_SrcFile.c_str());

	if (!parseResult)
		throw EXCEPTION(ParseException(m_SrcFile, parseResult.offset));

	auto xmlRoot = xmlDoc.document_element();

	if (!xmlRoot)
		throw EXCEPTION(ParseException(m_SrcFile, 0));


	ParseSettings(xmlRoot, m_Result.m_Settings);


	auto xmlBackups = xmlRoot.select_nodes(LR"(//mybcopy[@version="2.1"]/backup[@name!=""])");


	for (auto& xmlNode : xmlBackups)
	{
		ParseBackup(xmlNode.node());
	}

	return m_Result;
}

void MyBCopyFile::XmlLoader::ParseBackup(const pugi::xml_node& xmlBackupNode)
{
	ParsedSources backup;
	backup.m_Name = xmlBackupNode.attribute(L"name").as_string();
	ValidateBackupName(backup);

	backup.m_Comment = xmlBackupNode.attribute(L"comment").as_string();

	ParseSettings(xmlBackupNode, backup.m_Settings);

	auto xpNodes = xmlBackupNode.select_nodes(LR"(src[@path!=""])");
	ParseSrcPaths(xpNodes, backup.m_SrcPaths);

	m_Result.m_Backups.emplace_back(move(backup));
}





void MyBCopyFile::XmlLoader::ValidateBackupName(const ParsedSources& backup)
{
	static const wregex mask(LR"delim((\d|\w).*)delim");

	if (!regex_match(backup.m_Name, mask))
		throw EXCEPTION(BaseException(L"Wrong backup name " + backup.m_Name));


	auto it = find_if(m_Result.m_Backups.begin(), m_Result.m_Backups.end(), [backup](const ParsedSources& e)->bool
	{
		return backup.m_Name == e.m_Name;
	});

	if (it != m_Result.m_Backups.end())
		throw EXCEPTION(BaseException(L"Similar backup name " + backup.m_Name));
}





std::shared_ptr<Filters::FilterBase> MyBCopyFile::XmlLoader::ParseAndMakeFilter(const pugi::xml_node& xmlFilter)
{
	const wchar_t* filterName = xmlFilter.name();

	wstring text = xmlFilter.text().as_string();
	if (text.empty())
		throw EXCEPTION(BaseException(L"wrong filter " + wstring(filterName) + L" value " + text));

	using namespace Filters;

	if (_wcsicmp(filterName, L"wildcards") == 0)
	{
		return make_shared<WildcardsFilter>(text);
	}

	if (_wcsicmp(filterName, L"regexps") == 0)
	{
		return make_shared<RegexpsFilter>(text);
	}


	throw EXCEPTION(BaseException(L"wrong filter " + wstring(filterName) + L" value " + text));
}

void MyBCopyFile::XmlLoader::ParseSrcPaths(const pugi::xpath_node_set& xpNodes, list<SrcPathDesc>& paths)
{
	for (auto& xpNode : xpNodes)
	{
		pugi::xml_node xmlNode = xpNode.node();

		SrcPathDesc srcPath;
		srcPath.m_Path = xmlNode.attribute(L"path").as_string();

		if (srcPath.m_Path.empty())
			throw EXCEPTION(BaseException(wstring(L"wrong path value ") + wstring(srcPath.m_Path)));

		uint32_t depth = SrcPathDesc::Depthless;

		auto depthText = TrimString(xmlNode.attribute(L"depth").value());

		if (!depthText.empty() && depthText != L"nolimit")
		{
			if (!IsDecimalDigitsOnly(depthText))
				throw EXCEPTION(BaseException(wstring(L"wrong depth value ") + depthText));

			unsigned long ulDepth = wcstoul(depthText.c_str(), nullptr, 10);

			if (errno != 0)
				throw EXCEPTION(BaseException(wstring(L"wrong depth value ") + depthText));


			if (static_cast<unsigned long>(ulDepth) >= UINT32_MAX)
				throw EXCEPTION(BaseException(wstring(L"wrong depth value ") + depthText));

			depth = static_cast<uint32_t>(ulDepth);
		}

		srcPath.m_Depth = depth;
		
		auto exclusionText = TrimString(xmlNode.attribute(L"exclusion").value());

		srcPath.m_Include = (exclusionText != L"1");

		if (srcPath.m_Include)
		{
			auto xpFilters = xmlNode.select_nodes(LR"(include_entry_if/*)");
			ParseFilters(xpFilters, true, Filters::EntryType::Entry, srcPath.m_Filters);

			xpFilters = xmlNode.select_nodes(LR"(exclude_entry_if/*)");
			ParseFilters(xpFilters, false, Filters::EntryType::Entry, srcPath.m_Filters);

			xpFilters = xmlNode.select_nodes(LR"(include_file_if/*)");
			ParseFilters(xpFilters, true, Filters::EntryType::File, srcPath.m_Filters);

			xpFilters = xmlNode.select_nodes(LR"(exclude_file_if/*)");
			ParseFilters(xpFilters, false, Filters::EntryType::File, srcPath.m_Filters);

			xpFilters = xmlNode.select_nodes(LR"(include_dir_if/*)");
			ParseFilters(xpFilters, true, Filters::EntryType::Dir, srcPath.m_Filters);

			xpFilters = xmlNode.select_nodes(LR"(exclude_dir_if/*)");
			ParseFilters(xpFilters, false, Filters::EntryType::Dir, srcPath.m_Filters);
		}

		auto xpNodes = xmlNode.select_nodes(LR"(src[@path!=""])");
		ParseSrcPaths(xpNodes, srcPath.m_SubSrcPathDescs);

		paths.emplace_back(move(srcPath));
	}
}


void MyBCopyFile::XmlLoader::ParseFilters(const pugi::xpath_node_set& xpFilters, bool inclusion, Filters::EntryType entryType, list< shared_ptr<Filters::FilterBase> >& filters)
{
	for (auto& xpFilters : xpFilters)
	{
		auto filter = ParseAndMakeFilter(xpFilters.node());
		filter->SetInclusion(inclusion);
		filter->SetEntryType(entryType);
		filters.emplace_back(move(filter));
	}
}

bool MyBCopyFile::XmlLoader::TryParseAttr(const pugi::xml_node& xmlRoot, const wchar_t* attrName, const wchar_t*& text)
{
	auto xmlAttr = xmlRoot.attribute(attrName);

	if (!xmlAttr)
		return false;

	text = xmlAttr.value();
	return true;
}

template <typename OutT>
bool MyBCopyFile::XmlLoader::TryParseStringAttr(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField)
{
	const wchar_t* text;

	if (TryParseAttr(xmlBackupDir, attrName, text))
	{
		settingsField = text;
		return true;
	}
	else
	{
		return false;
	}
}

template <typename OutT>
bool MyBCopyFile::XmlLoader::TryParseBoolAttr(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField)
{
	const wchar_t* text;

	if (TryParseAttr(xmlBackupDir, attrName, text))
	{
		wstring verificationValue = AsciiStringToLower(text);
		bool verification = (verificationValue == L"yes" || verificationValue == L"true" || verificationValue == L"on" || verificationValue == L"1");
		settingsField = boost::make_optional<bool>(verification);
		return true;
	}
	else
		return false;
}



template <typename OutT>
bool MyBCopyFile::XmlLoader::TryParseUIntAttr(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField)
{
	const wchar_t* text;

	if (TryParseAttr(xmlBackupDir, attrName, text))
	{
		//TODO fixme
		settingsField = static_cast<unsigned int>(stoul(text));
		return true;
	}
	else
		return false;
}

unsigned int MyBCopyFile::XmlLoader::ParseUIntAttr(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName)
{
	unsigned int result;

	if (TryParseUIntAttr(xmlBackupDir, attrName, result))
		return result;
	else
		throw EXCEPTION(ParseException(m_SrcFile, xmlBackupDir.offset_debug()));
}


bool MyBCopyFile::XmlLoader::TryParseParam(const pugi::xml_node& xmlNode, const wchar_t* paramName, const wchar_t*& text)
{
	auto xmlField = xmlNode.first_element_by_path(paramName);

	if (!xmlField)
		return false;

	text = xmlField.text().as_string();
	return true;
}

template <typename OutT>
bool MyBCopyFile::XmlLoader::TryParseStringParam(const pugi::xml_node& xmlNode, const wchar_t* paramName, OutT& settingsField)
{
	const wchar_t* text;

	if (TryParseParam(xmlNode, paramName, text))
	{
		settingsField = text;
		return true;
	}
	else
		return false;
}

template <typename OutT>
bool MyBCopyFile::XmlLoader::TryParseBoolParam(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField)
{
	const wchar_t* text;

	if (TryParseParam(xmlBackupDir, attrName, text))
	{
		wstring verificationValue = AsciiStringToLower(text);
		bool verification = (verificationValue == L"yes" || verificationValue == L"true" || verificationValue == L"on" || verificationValue == L"1");
		settingsField = boost::make_optional<bool>(verification);
		return true;
	}
	else
		return false;
}

template <typename OutT>
bool MyBCopyFile::XmlLoader::TryParseUIntParam(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField)
{
	const wchar_t* text;

	if (TryParseParam(xmlBackupDir, attrName, text))
	{
		//TODO fixme
		settingsField = static_cast<unsigned int>(stoul(text));
		return true;
	}
	else
		return false;
}

unsigned int MyBCopyFile::XmlLoader::ParseUIntParam(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName)
{
	unsigned int result;

	if (TryParseUIntParam(xmlBackupDir, attrName, result))
		return result;
	else
		throw EXCEPTION(ParseException(m_SrcFile, xmlBackupDir.offset_debug()));
}


void MyBCopyFile::XmlLoader::ParseSettings(const pugi::xml_node& xmlRoot, Settings& settings)
{	
	TryParseStringParam(xmlRoot, L"temporary_dir", settings.m_TemporaryDir);

	auto xmlStorages = xmlRoot.first_element_by_path(L"storages");

	if (xmlStorages)
	{
		auto xpStorages = xmlStorages.select_nodes(LR"(storage[@type!=""])");
		ParseStorages(xpStorages, settings.m_Storages);
	}

	auto xmlScheme = xmlRoot.first_element_by_path(L"scheme");

	if (xmlScheme)
	{
		settings.m_SchemeParams = ParseAndMakeScheme(xmlScheme);
	}


	auto xmlToolchain = xmlRoot.first_element_by_path(L"toolchain");

	if (xmlToolchain)
	{
		auto xpToolchain = xmlToolchain.select_nodes(LR"(tool[@type!=""])");
		ParseToolchain(xpToolchain, settings.m_Toolchain);
	//	ParseList(xpToolchain, settings.m_Toolchain, &ParseAndMakeTool);
		
		//
		//for (auto& xmlTool : xpToolchain)
		//	ParseAndMakeTool(xmlTool.node());
	}

	TryParseBoolParam(xmlRoot, L"verification", settings.m_Verification);
}

//
//template <typename Container, typename Func>
//void MyBCopyFile::XmlLoader::ParseList(const pugi::xpath_node_set& xpStorages, Container& container, Func func)
//{
//	for (auto& xpStorage : xpStorages)
//	{
//		auto storage = func(xpStorage.node());
//		container.emplace_back(move(storage));
//	}
//}

void MyBCopyFile::XmlLoader::ParseStorages(const pugi::xpath_node_set& xpStorages, list< shared_ptr<Storage> >& storages)
{
	for (auto& xpStorage : xpStorages)
	{
		auto storage = ParseAndMakeStorage(xpStorage.node());
		storages.emplace_back(move(storage));
	}
}


shared_ptr<Storage> MyBCopyFile::XmlLoader::ParseAndMakeStorage(const pugi::xml_node& xmlStorage)
{
	wstring type = AsciiStringToLower(xmlStorage.attribute(L"type").as_string());

	if (type == L"webdav")
	{
		auto hlResult = make_shared<WebDavStorage>();
		hlResult->m_Name = xmlStorage.attribute(L"name").as_string();
		hlResult->m_UserName = xmlStorage.attribute(L"username").as_string();
		hlResult->m_Password = xmlStorage.attribute(L"password").as_string();
		hlResult->m_Url = xmlStorage.attribute(L"url").as_string();
		hlResult->m_BackupDirName = xmlStorage.attribute(L"backup_dirname").as_string();
		
		if (hlResult->m_Url.empty())
			throw EXCEPTION(ParseException(m_SrcFile, xmlStorage.offset_debug()));

		return hlResult;		
	}

	if (type == L"localdisk")
	{
		auto hlResult = make_shared<LocalDiskStorage>();
		hlResult->m_Name = xmlStorage.attribute(L"name").as_string();
		hlResult->m_Path = xmlStorage.attribute(L"path").as_string();

		if (hlResult->m_Path.empty())
			throw EXCEPTION(ParseException(m_SrcFile, xmlStorage.offset_debug()));

		return hlResult;
	}

	throw EXCEPTION(ParseException(m_SrcFile, xmlStorage.offset_debug()));
}


std::shared_ptr<RegularScheme> MyBCopyFile::XmlLoader::ParseAndMakeScheme(const pugi::xml_node& xmlScheme)
{
	wstring type = AsciiStringToLower(xmlScheme.attribute(L"type").as_string());

	if (type == L"regular" || type == L"incremental")
	{
		unsigned int num_stored_old_regular_backups;

		if (!TryParseUIntParam(xmlScheme, L"num_stored_old_regular_backups", num_stored_old_regular_backups))
		{
			num_stored_old_regular_backups = 0;
		}


		if (type == L"regular")
		{
			auto hlResult = make_shared<RegularScheme>();
			hlResult->m_NumStoredOldRegularBackups = num_stored_old_regular_backups;

			return hlResult;
		}

		if (type == L"incremental")
		{
			auto hlResult = make_shared<IncrementalScheme>();
			hlResult->m_NumStoredOldRegularBackups = num_stored_old_regular_backups;

			hlResult->m_NumIncrementBranches = ParseUIntParam(xmlScheme, L"num_increment_branches");
			hlResult->m_LengthIncrementChain = ParseUIntParam(xmlScheme, L"length_increment_chain");

			if (!TryParseUIntParam(xmlScheme, L"num_stored_old_increment_branches", hlResult->m_NumStoredOldIncrementBranches))
			{
				hlResult->m_NumStoredOldIncrementBranches = 0;
			}

			TryParseStringParam(xmlScheme, L"reference_storage", hlResult->m_ReferenceStorageName);

			return hlResult;
		}
	}


	throw EXCEPTION(ParseException(m_SrcFile, xmlScheme.offset_debug()));
}

std::shared_ptr<Tool> MyBCopyFile::XmlLoader::ParseAndMakeTool(const pugi::xml_node& xmlTool)
{
	wstring type = AsciiStringToLower(xmlTool.attribute(L"type").as_string());

	wstring path;
	TryParseStringAttr(xmlTool, L"path", path);
	wstring passwd;
	TryParseStringAttr(xmlTool, L"password", passwd);

	if (type == L"rar")
	{
		return make_shared<ArchiverRar>(path, passwd);
	}

	if (type == L"7z")
	{		
		return make_shared<Archiver7z>(path, passwd);
	}

	throw EXCEPTION(ParseException(m_SrcFile, xmlTool.offset_debug()));
}

void MyBCopyFile::XmlLoader::ParseToolchain(const pugi::xpath_node_set& xpStorages, list<shared_ptr<Tool>>& toolchain)
{
	for (auto& xpStorage : xpStorages)
	{
		auto storage = ParseAndMakeTool(xpStorage.node());
		toolchain.emplace_back(move(storage));
	}
}

//
//Reserver::XmlParser::EasyDate::EasyDate(const wstring& nowTimestamp)
//{	
//	memset(&timeDate, 0, sizeof(timeDate));
//
//	timeDate.tm_year = std::stoi(nowTimestamp.substr(0, 4)) - 1900;
//	timeDate.tm_mon = std::stoi(nowTimestamp.substr(5, 2)) - 1;
//	timeDate.tm_mday = std::stoi(nowTimestamp.substr(8, 2));	
//	timeDate.tm_hour = std::stoi(nowTimestamp.substr(11, 2));
//	timeDate.tm_min = std::stoi(nowTimestamp.substr(14, 2));
//	timeDate.tm_isdst = -1;
//
//	time_t tt = std::mktime(&timeDate);
//}
//
//void Reserver::XmlParser::EasyDate::SubtractYears(int years)
//{
//	timeDate.tm_year -= years;
//
//}
//
//void Reserver::XmlParser::EasyDate::SubtractMonths(int months)
//{
//	int years = months / 12;
//	SubtractYears(years);
//	months = months % 12;	
//	timeDate.tm_mon -= months;
//}
//
//void Reserver::XmlParser::EasyDate::SubtractWeeks(int weeks)
//{
//	SubtractDays(weeks * 7);
//}
//
//void Reserver::XmlParser::EasyDate::SubtractDays(int days)
//{
//	while (days >= timeDate.tm_mday)
//	{
//		days -= timeDate.tm_mday;
//		timeDate.tm_mon -= 1; 
//		timeDate.tm_mday = CountDaysInMonth(timeDate.tm_mon);
//		
//		return;
//	}
//
//	
//}
//
//void Reserver::XmlParser::EasyDate::CorrectDown()
//{
//	if (timeDate.tm_mon % 2 == 1 && timeDate.tm_mon != 1 && timeDate.tm_mday == 31)
//	{		
//		if (timeDate.tm_mon != 1)
//			timeDate.tm_mday == 30;
//		else
//		{
//			if (timeDate.tm_year % 4 == 0)
//				timeDate.tm_mday == 29;
//			else
//				timeDate.tm_mday == 28;
//		}
//	}
//}
//
//int Reserver::XmlParser::EasyDate::CountDaysInMonth(int month)
//{
//	if 
//}
//
//Reserver::XmlParser::EasyDate::operator fs::file_time_type() const
//{
//
//}



ParseException::ParseException(const wstring& file, size_t line) : m_File(file), m_Line(line)
{

}

void ParseException::FormattedMessageLine()
{
	if (m_Line != size_t(-1))
		TextLine(L"Parse error at " + m_File + L":" + to_wstring(m_Line));
	else
		TextLine(L"Parse error at " + m_File);
}