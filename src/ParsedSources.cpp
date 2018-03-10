#include "MyBCopy_pch.h"
#include "ParsedSources.h"
#include "Utils.h"
#include "Reserver.h"
#include "UtilsMyBCopy.h"






void ParsedSources::Normalize()
{
	m_SrcPaths.sort([](const SrcPathDesc& l, const SrcPathDesc& r)->bool
	{
		return l.m_Path < r.m_Path;
	});

	erase_if_swallowed(m_SrcPaths, &SrcPathDesc::TrySwallow);
}

void ParsedSources::NormalizeRecursive()
{	
	Normalize();
	for (auto& srcPath : m_SrcPaths)
	{
		srcPath.NormalizeRecursive();
	}
}



bool ParsedSources::Empty()
{
	return m_SrcPaths.empty();
}





SrcPathDesc::SrcPathDesc(const fs::path& path) : m_Path(path)
{

}

void SrcPathDesc::Normalize()
{
	m_SubSrcPathDescs.sort([](const SrcPathDesc& l, const SrcPathDesc& r)->bool
	{
		return l.m_Path < r.m_Path;
	});
	erase_if_swallowed(m_SubSrcPathDescs, &SrcPathDesc::TrySwallow);
}

void SrcPathDesc::NormalizeRecursive()
{
	Normalize();
	for (auto& srcPath : m_SubSrcPathDescs)
	{
		srcPath.NormalizeRecursive();
	}
}

SrcPathDesc SrcPathDesc::CreateViaInheritance(const wstring& dirPath, const SrcPathDesc& srcPath)
{
	SrcPathDesc result;
	result.m_Include = srcPath.m_Include;
	result.m_Path = dirPath;
	result.m_Filters = srcPath.m_Filters;	
	result.m_Depth = (srcPath.m_Depth == SrcPathDesc::Depthless) ? SrcPathDesc::Depthless : srcPath.m_Depth - 1;

	for (auto& subSrcPath : srcPath.m_SubSrcPathDescs)
	{
		fs::path subSrcPathOriginal = (subSrcPath.m_Path);
		auto it = subSrcPathOriginal.begin();

		
		if (dirPath == *it)
		{
			assert( distance(subSrcPath.m_Path.begin(), subSrcPath.m_Path.end()) > 1);

			SrcPathDesc inheritedSubSrcPath = subSrcPath;

			++it;
			auto init = *it;

			inheritedSubSrcPath.m_Path = accumulate(++it, subSrcPathOriginal.end(), init, [](const fs::path& l, const fs::path& r)-> fs::path
			{
				return l / r;
			});

			result.m_SubSrcPathDescs.push_back(inheritedSubSrcPath);
		}
	}

	return result;
}

bool SrcPathDesc::HasFilters() const
{
	return !m_Filters.empty();	
}


bool SrcPathDesc::TrySwallow(SrcPathDesc& lhs, SrcPathDesc& rhs)
{
	fs::path lhsPath(lhs.m_Path);
	fs::path rhsPath(rhs.m_Path);

	auto it = search(rhsPath.begin(), rhsPath.end(), lhsPath.begin(), lhsPath.end());

	if (it != rhsPath.begin())
	{
		return false;
	}

	auto lhsDistance = distance(lhsPath.begin(), lhsPath.end());
	auto rhsDistance = distance(rhsPath.begin(), rhsPath.end());

	if (lhsDistance < rhsDistance)
	{
		advance(it, lhsDistance);
		auto init = *it;

		fs::path rhsRelativePath = accumulate(++it, rhsPath.end(), init, [](const fs::path& l, const fs::path& r)-> fs::path
		{
			return l / r;
		});

		rhs.m_Path = rhsRelativePath;

		lhs.m_SubSrcPathDescs.emplace_back(move(rhs));
	}
	else
	{//lhsDistance == rhsDistance
		assert(lhsDistance == rhsDistance);

		///Ignoring duplicate of entry
		WriteToStdout(wstring(L"Ignoring duplicate of src entry ") + rhs.m_Path.c_str());

		////move(rhs.Filters.begin(), rhs.Filters.end(), back_inserter(lhs.Filters) );
		//lhs.Depth = std::max(lhs.Depth, rhs.Depth);

		//move(rhs.IncludeFilters.begin(), rhs.IncludeFilters.end(), back_inserter(lhs.IncludeFilters));
		//move(rhs.ExcludeFilters.begin(), rhs.ExcludeFilters.end(), back_inserter(lhs.ExcludeFilters));
	}

	return true;
}

//
//bool SrcPathDesc::HasSubPath(const fs::path& subPath) const
//{
//	throw EXCEPTION(logic_error("Not implemented"));
//}



void Linearize(const list<SrcPathDesc>& srcPaths, std::list<SrcPathDesc>& result, const fs::path& parentPath /*= fs::path()*/)
{
	for (auto& srcPath : srcPaths)
	{
		SrcPathDesc resultPathDesc = srcPath;		
		resultPathDesc.m_Path = parentPath / srcPath.m_Path;
		resultPathDesc.m_SubSrcPathDescs.clear();
		result.emplace_back(move(resultPathDesc));

		Linearize(srcPath.m_SubSrcPathDescs, result, parentPath / srcPath.m_Path);
	}
}
