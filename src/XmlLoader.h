#pragma once
#include "Reserver.h"



class MyBCopyFile;

class MyBCopyFile::XmlLoader
{
public:
	XmlLoader(const wstring& srcFile);
	MyBCopyFile Load();
private:
	wstring m_SrcFile;
	MyBCopyFile m_Result;

	void ParseSettings(const pugi::xml_node& xmlRoot, Settings& settings);
	void ParseStorages(const pugi::xpath_node_set& xpStorages, list< shared_ptr<Storage> >& storages);
	shared_ptr<Storage> ParseAndMakeStorage(const pugi::xml_node& xmlStorage);
	shared_ptr<RegularScheme> ParseAndMakeScheme(const pugi::xml_node& xmlScheme);
	shared_ptr<Tool> ParseAndMakeTool(const pugi::xml_node& xmlTool);

	void ParseBackup(const pugi::xml_node& xmlBackupNode);
	void ParseSrcPaths(const pugi::xpath_node_set& xpNodes, list<SrcPathDesc>& paths);
	void ValidateBackupName(const ParsedSources& backup);

	void ParseFilters(const pugi::xpath_node_set& xpFilters, bool inclusion, Filters::EntryType entryType, list< shared_ptr<Filters::FilterBase> >& filters);
	shared_ptr<Filters::FilterBase> ParseAndMakeFilter(const pugi::xml_node& xmlFilter);

	/*template <typename Container, typename Func>
	void ParseList(const pugi::xpath_node_set& xpStorages, Container& container, Func func);*/


	template <typename OutT>
	bool TryParseStringAttr(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField);
	template <typename OutT>
	bool TryParseBoolAttr(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField);
	template <typename OutT>
	bool TryParseUIntAttr(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField);
	unsigned int ParseUIntAttr(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName);

	bool TryParseAttr(const pugi::xml_node& xmlRoot, const wchar_t* attrName, const wchar_t*& text);




	template <typename OutT>
	bool TryParseStringParam(const pugi::xml_node& xmlNode, const wchar_t* paramName, OutT& settingsField);
	template <typename OutT>
	bool TryParseBoolParam(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField);
	template <typename OutT>
	bool TryParseUIntParam(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName, OutT& settingsField);
	unsigned int ParseUIntParam(const pugi::xml_node& xmlBackupDir, const wchar_t* attrName);

	bool TryParseParam(const pugi::xml_node& xmlNode, const wchar_t* paramName, const wchar_t*& text);
	void ParseToolchain(const pugi::xpath_node_set& xpStorages, list<shared_ptr<Tool>>& toolchain);





	//class EasyDate
	//{
	//	public:
	//		EasyDate(const wstring& nowTimestamp);
	//		operator fs::file_time_type() const;

	//		void SubtractYears(int years);
	//		void SubtractMonths(int months);
	//		void SubtractWeeks(int weeks);
	//		void SubtractDays(int days);
	//		void CorrectDown();
	//	private:
	//		static int CountDaysInMonth(int month);
	//		struct tm timeDate;
	//};



};





class MyBCopy_API ParseException : public CachedMessageException
{
public:
	ParseException(const wstring& file, size_t line = size_t(-1));

	wstring m_File;
	size_t m_Line;
protected:
	virtual void FormattedMessageLine() override;
};
