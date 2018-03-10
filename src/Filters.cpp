#include "MyBCopy_pch.h"
#include "Filters.h"
#include "Utils.h"
#include "UtilsMyBCopy.h"






namespace Filters
{	
	//RegexpsFilter::RegexpsFilter(const list<wregex>& regexs) : m_Regexs(regexs)
	//{

	//}

	RegexpsFilter::RegexpsFilter(const wstring& regexpsString)
	{		
		list<wstring>  regexps;
		SplitWStringAnyOf(regexpsString, L";", regexps);
		for (auto& regexp : regexps)
		{
			AddRegexp(regexp);
		}
	}

	bool RegexpsFilter::operator()(const fs::path& file) const
	{
		wstring filename = file.filename();
		return MatchAny(filename);
	}

	
	const std::list<std::wregex>& RegexpsFilter::GetRegexs() const
	{
		return m_Regexs;
	}

	const std::wstring& RegexpsFilter::GetRegexsAsString() const
	{
		return m_RegexString;
	}

	void RegexpsFilter::AddRegexp(const wstring& regexs)
	{		
		if (!m_RegexString.empty())
			m_RegexString += L";";
		m_RegexString += regexs;
		m_Regexs.emplace_back(regexs);
	}

	bool RegexpsFilter::MatchAny(const wstring& name) const
	{
		for (auto& rgx : m_Regexs)
		{
			if (regex_match(name, rgx))
				return true;
		}
		return false;
	}

	WildcardsFilter::WildcardsFilter(const list<wstring>& wildcards)
		: m_Wildcards(wildcards)
	{
		for (auto& wildcard : wildcards)
		{
			AddRegexp(wildcard2wregex(wildcard));			
		}
	}

	WildcardsFilter::WildcardsFilter(const wstring& wildcards)
	{
		SplitWStringAnyOf(wildcards, L";", m_Wildcards);
		for (auto& wildcard : m_Wildcards)
		{
			AddRegexp(wildcard2wregex(wildcard));			
		}
	}

	std::wstring WildcardsFilter::GetWildcardsAsString() const
	{		
		wstring result;
		for (auto& wildcard : m_Wildcards)
		{
			result += wildcard + L";";			
		}		
		result.pop_back();
		return result;
	}


	FilterBase::FilterBase(bool inclusion /*= true*/, EntryType entryType /*= EntryType::Entry*/)
		: m_Inclusion(inclusion),
		m_EntryType(entryType)
	{

	}

	bool FilterBase::IsInclusion() const
	{
		return m_Inclusion;
	}

	void FilterBase::SetInclusion(bool val)
	{
		m_Inclusion = val;
	}

	Filters::EntryType FilterBase::GetEntryType() const
	{
		return m_EntryType;
	}

	/*
	LastWriteTime_LessThen::LastWriteTime_LessThen(const wstring& path, fs::file_time_type date)  m_Date(date)
	{

	}

	bool LastWriteTime_LessThen::operator()(const fs::path& file)
	{
	auto lastModTime = fs::last_write_time(file);
	return lastModTime < m_Date;
	}

	bool LastWriteTime_Greater::operator()(const fs::path& file)
	{
	return !LastWriteTime_LessThen::operator()(file);
	}
	*/

	

	void FilterBase::SetEntryType(Filters::EntryType val)
	{
		m_EntryType = val;
	}

}
