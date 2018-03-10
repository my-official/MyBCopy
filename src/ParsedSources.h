#pragma once

#include "BackupBase.h"
#include "Filters.h"





class SrcPathDesc;

class MyBCopy_API SrcPathDesc
{
public:
	SrcPathDesc() = default;
	SrcPathDesc(const fs::path& path);
	void Normalize();
	void NormalizeRecursive();
	static SrcPathDesc CreateViaInheritance(const wstring& dirPath, const SrcPathDesc& srcPath);

	bool HasFilters() const;	

	static bool TrySwallow(SrcPathDesc& lhs, SrcPathDesc& rhs);

	//bool HasSubPath(const fs::path& subPath) const;
	
	fs::path								m_Path;
	bool									m_Include = true;
	
	list< shared_ptr<Filters::FilterBase> >	m_Filters;

	static const uint32_t						Depthless = UINT32_MAX;
	uint32_t									m_Depth = Depthless;
	
	list<SrcPathDesc>							m_SubSrcPathDescs;	
};

void MyBCopy_API Linearize(const list<SrcPathDesc>& srcPaths, std::list<SrcPathDesc>& result, const fs::path& parentPath = fs::path());


class MyBCopy_API ParsedSources : public BackupBase
{
public:
	ParsedSources() = default;
		
	wstring			m_Comment;
	list<SrcPathDesc>	m_SrcPaths;

	void Normalize();
	void NormalizeRecursive();

	

	//vector< shared_ptr<ConditionalPath::ConditionalPathBase> > m_ExclFilesConditional;

	virtual bool Empty() override;	

};

