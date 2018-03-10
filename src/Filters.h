#pragma once


namespace Filters
{
	enum class EntryType : int
	{
		Entry,
		Dir,
		File,
	};

	class MyBCopy_API FilterBase
	{
	public:		
		FilterBase(bool inclusion = true, EntryType entryType = EntryType::Entry);
		virtual ~FilterBase() = default;		
		virtual bool operator()(const fs::path& file) const = 0;

		bool IsInclusion() const;
		void SetInclusion(bool val);
		EntryType GetEntryType() const;
		void SetEntryType(Filters::EntryType val);
	protected:
		bool m_Inclusion;
		EntryType m_EntryType;
	};

	

	class MyBCopy_API RegexpsFilter : public FilterBase
	{
	public:
		RegexpsFilter(const wstring& regexpsString);
		//RegexpsFilter(const list<wregex>& regexs);
		virtual bool operator()(const fs::path& file) const override;
		const std::list<std::wregex>& GetRegexs() const;
		const wstring& GetRegexsAsString() const;
		void AddRegexp(const wstring& regexs);
	protected:
		RegexpsFilter() = default;
	private:
		wstring m_RegexString;
		list<wregex> m_Regexs;
		bool MatchAny(const wstring& name) const;
	};


	class MyBCopy_API WildcardsFilter : public RegexpsFilter
	{
	public:
		WildcardsFilter(const wstring& wildcards);
		WildcardsFilter(const list<wstring>& wildcards);
		wstring GetWildcardsAsString() const;
	protected:
		list<wstring> m_Wildcards;
	};
	
	}

