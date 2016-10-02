/**
 * The MIT License (MIT)
 * Copyright (c) <2015> <carriez.md@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
  *
  */

#ifndef INI_HPP
#define INI_HPP

#include <cassert>
#include <map>
#include <list>
#include <stdexcept>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

namespace INI
{
	template <typename charT>
	constexpr charT charT_cast(char c)
	{
		return charT(c);
	}

	

	template <typename stringT>
	struct Level
	{
		Level() : parent(NULL), depth(0)
		{
		}
		Level(Level* p) : parent(p), depth(0)
		{
		}

		using istreamT = std::conditional< std::is_same<stringT, std::string>::value, std::istream, std::wistream>;
		using ifstreamT = std::conditional< std::is_same<stringT, std::string>::value, std::ifstream, std::wifstream>;
		using ostreamT = std::conditional< std::is_same<stringT, std::string>::value, std::ostream, std::wostream>;
		using charT = std::conditional< std::is_same<stringT, std::string>::value, char, wchar_t>;

		using value_map_t = std::map<stringT, stringT>;
		using section_map_t = std::map<stringT, Level>;
		using values_t = std::list<typename value_map_t::const_iterator>;
		using sections_t = std::list<typename section_map_t::const_iterator>;
		value_map_t values;
		section_map_t sections;
		values_t ordered_values; // original order in the ini file
		sections_t ordered_sections;
		Level<stringT>* parent;
		size_t depth;

		const stringT& operator[](const stringT& name)
		{
			return values[name];
		}
		Level<stringT>& operator()(const stringT& name)
		{
			return sections[name];
		}
	};

	using WLevel = Level<std::wstring>;


	template <typename stringT = std::string>
	class Parser
	{
	private:
		using istreamT = typename std::conditional< std::is_same<stringT, std::string>::value, std::istream, std::wistream>::type;
		using ifstreamT = typename std::conditional< std::is_same<stringT, std::string>::value, std::ifstream, std::wifstream>::type;
		using ostreamT = typename std::conditional< std::is_same<stringT, std::string>::value, std::ostream, std::wostream>::type;
		using charT = typename std::conditional< std::is_same<stringT, std::string>::value, char, wchar_t>::type;
		using Level = typename Level<stringT>;

	public:
		Parser(const charT* fn) : f0_(fn), f_(&f0_), ln_(0)
		{
			if (!f0_)
				throw std::runtime_error(std::string("failed to open file"));

			parse(top_);
		}
		Parser(istreamT& f) : f_(&f), ln_(0)
		{
			parse(top_);
		}
		Level& top()
		{
			return top_;
		}

	private:
		inline void parse(Level& l)
		{
			while (std::getline(*f_, line_))
			{
				++ln_;
				if (line_[0] == charT_cast<charT>('#') || line_[0] == charT_cast<charT>(';')) continue;
				line_ = trim(line_);
				if (line_.empty()) continue;
				if (line_[0] == charT_cast<charT>('['))
				{
					size_t depth;
					stringT sname;
					parseSLine(sname, depth);
					Level* lp = NULL;
					Level* parent = &l;
					if (depth > l.depth + 1)
						err("section with wrong depth");
					if (l.depth == depth - 1)
						lp = &l.sections[sname];
					else
					{
						lp = l.parent;
						size_t n = l.depth - depth;
						for (size_t i = 0; i < n; ++i) lp = lp->parent;
						parent = lp;
						lp = &lp->sections[sname];
					}
					if (lp->depth != 0)
						err("duplicate section name on the same level");
					if (!lp->parent)
					{
						lp->depth = depth;
						lp->parent = parent;
					}
					parent->ordered_sections.push_back(parent->sections.find(sname));
					parse(*lp);
				}
				else
				{
					size_t n = line_.find(charT_cast<charT>('='));
					if (n == std::string::npos)
						err("no '=' found");
					stringT v = value(trim(line_.substr(n + 1, line_.length() - n - 1)));
					std::pair<Level::value_map_t::const_iterator, bool> res =
						l.values.insert(std::make_pair(trim(line_.substr(0, n)),
							v));
					if (!res.second)
						err("duplicated key found");
					l.ordered_values.push_back(res.first);
				}
			}
		}
		inline void parseSLine(stringT& sname, size_t& depth)
		{
			depth = 0;
			for (; depth < line_.length(); ++depth)
				if (line_[depth] != charT_cast<charT>('[')) break;

			sname = line_.substr(depth, line_.length() - 2 * depth);
		}
		inline void err(const char* s)
		{
			std::ostringstream os;
			os << s << " on line #" << ln_;
			throw std::runtime_error(os.str());
		}

	private:
		Level top_;
		ifstreamT f0_;
		istreamT* f_;
		stringT line_;
		size_t ln_;
	};

	using WParser = Parser<std::wstring>;

		
	inline std::string trim(const std::string& s)
	{
		char p[] = " \t\r\n";
		long sp = 0;
		long ep = s.length() - 1;
		for (; sp <= ep; ++sp)
			if (!strchr(p, s[sp])) break;
		for (; ep >= 0; --ep)
			if (!strchr(p, s[ep])) break;
		return s.substr(sp, ep - sp + 1);
	}
	
	inline std::wstring trim(const std::wstring& s)
	{
		wchar_t p[] = L" \t\r\n";
		long sp = 0;
		long ep = s.length() - 1;
		for (; sp <= ep; ++sp)
			if (!wcschr(p, s[sp])) break;
		for (; ep >= 0; --ep)
			if (!wcschr(p, s[ep])) break;
		return s.substr(sp, ep - sp + 1);
	}

	
	inline std::string value(const std::string& s)
	{
		std::string c =  "#;";
		std::string v = s;
		size_t ep = std::string::npos;
		for (size_t i = 0; i < c.size(); i++)
		{
			ep = s.find(c[i]);
			if (ep != std::string::npos)
				break;
		}
		if (ep != std::string::npos)
			v = s.substr(0, ep);
		return v;
	}

	inline std::wstring value(const std::wstring& s)
	{
		std::wstring c = L"#;";
		std::wstring v = s;
		size_t ep = std::wstring::npos;
		for (size_t i = 0; i < c.size(); i++)
		{
			ep = s.find(c[i]);
			if (ep != std::wstring::npos)
				break;
		}
		if (ep != std::wstring::npos)
			v = s.substr(0, ep);
		return v;
	}

}

#endif // INI_HPP

