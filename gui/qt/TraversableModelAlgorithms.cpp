#include "qtgui_pch.h"
#include "TraversableModelAlgorithms.h"
#include "TraversableModelThread.h"




TraverserAbstract::TraverserAbstract(TraversableModelThread* traversableThread, const fs::path& traversePath, const fs::path& srcOriginPath, unordered_map<QString, EntryCheckStateRec>* entry2CheckState)
	: m_SrcPathCheckState(IndeterminateCheckState),
	m_TraversableThread(traversableThread),
	m_TraversePath(traversePath),
	m_SrcOriginPath(srcOriginPath),
	m_Entry2CheckState(entry2CheckState)
{

}
//
//Qt::CheckState TraverserAbstract::TraverseAsDirEntry(const fs::path& srcPathAbsolute)
//{
//	fs::directory_iterator it(srcPathAbsolute);
//	fs::directory_iterator end_it;
//
//	if (it == end_it)
//		return Qt::Unchecked;
//
//	EntryCheckStateRec dirCheckState;
//
//	Qt::CheckState firstCheckState = IndeterminateCheckState;
//
//	TraverseFiles_First(it, end_it, dirCheckState, firstCheckState);
//
//	if (firstCheckState != IndeterminateCheckState)
//	{
//		TraverseFiles_WhileParentIndeterminate(it, end_it, dirCheckState, firstCheckState);
//		TraverseFiles_All(it, end_it, dirCheckState, firstCheckState);
//	}
//
//	///File Traverse Finished
//	it = fs::directory_iterator(srcPathAbsolute);
//
//	if (firstCheckState == IndeterminateCheckState)
//	{
//		TraverseDirs_First(it, end_it, dirCheckState, firstCheckState);
//
//		if (firstCheckState == IndeterminateCheckState)
//		{
//			dirCheckState.m_CheckState = Qt::Unchecked;
//		}
//		else
//		{
//			TraverseDirs_WhileParentIndeterminate(it, end_it, dirCheckState, firstCheckState);
//			TraverseDirs_All(it, end_it, dirCheckState, firstCheckState);
//		}
//	}
//	else
//	{
//		if (dirCheckState.m_CheckState != Qt::PartiallyChecked)
//		{
//			TraverseDirs_WhileParentIndeterminate(it, end_it, dirCheckState, firstCheckState);
//			TraverseDirs_All(it, end_it, dirCheckState, firstCheckState);
//		}
//		else
//		{
//			TraverseDirs_All(it, end_it, dirCheckState, firstCheckState);
//		}
//	}
//
//	auto result = dirCheckState.m_CheckState;
//
//	if (dirCheckState.m_CheckState != Qt::Unchecked)
//	{
//		QString qFileName;
//		if (srcPathAbsolute != srcPathAbsolute.root_path())
//			qFileName = QString::fromStdWString(srcPathAbsolute.filename());
//		else
//			qFileName = QString::fromStdWString(srcPathAbsolute.root_name());
//
//		(*m_Entry2CheckState)[qFileName] = move(dirCheckState);
//	}
//
//	return result;
//}

Qt::CheckState TraverserAbstract::ScopedRecursion(const fs::path& entityName, SrcPathDesc* srcPath, EntryCheckStateRec& dirCheckState)
{
	TraverserScoped traverser(m_TraversableThread, m_TraversePath / entityName, fs::path(), srcPath, &dirCheckState.m_SubEntries);
	traverser.Traverse();
	return traverser.m_SrcPathCheckState;
}

Qt::CheckState TraverserAbstract::InitialRecursion(const fs::path& entityName, vector<SrcPathDesc*>* srcDescs, EntryCheckStateRec& dirCheckState)
{
	TraverserInitial traverser(m_TraversableThread, m_TraversePath / entityName, m_SrcOriginPath / entityName, srcDescs, &dirCheckState.m_SubEntries);
	traverser.Traverse();
	return traverser.m_SrcPathCheckState;
}


Qt::CheckState TraverserAbstract::UnscopedRecursion(const fs::path& entityName, SrcPathDesc* srcPath, EntryCheckStateRec& dirCheckState)
{
	TraverserUnscoped traverser(m_TraversableThread, m_TraversePath / entityName, fs::path(), srcPath, &dirCheckState.m_SubEntries);
	traverser.Traverse();
	return traverser.m_SrcPathCheckState;
}

Qt::CheckState TraverserAbstract::ForwardRecursion(const fs::path& entityName, vector<SrcPathDesc*>* srcDescs, EntryCheckStateRec& dirCheckState)
{
	TraverserForward traverser(m_TraversableThread, m_TraversePath / entityName, m_SrcOriginPath / entityName, srcDescs, &dirCheckState.m_SubEntries);
	traverser.Traverse();
	return traverser.m_SrcPathCheckState;
}

TraverserInitial::TraverserInitial(TraversableModelThread* traversableThread, const fs::path& traversePath, const fs::path& srcOriginPath, vector<SrcPathDesc*>* srcDescs, unordered_map<QString, EntryCheckStateRec>* entry2CheckState)
	: TraverserAbstract(traversableThread, traversePath, srcOriginPath, entry2CheckState),
	m_SrcDescs(srcDescs)
{

}


void TraverserInitial::Traverse()
{
	m_SrcPathCheckState = TraverseAsDirEntry<TraverserInitial, TraverserInitial>(m_TraversePath);
}


SrcPathDesc* TraverserInitial::FindSubSrcDescForEntry(const fs::path& entryName)
{
	auto srcEntryPath = m_SrcOriginPath / entryName;

	auto ret = find_if(m_SrcDescs->begin(), m_SrcDescs->end(), [&srcEntryPath](const SrcPathDesc* e)->bool
	{
		return e->m_Path == srcEntryPath;
	});

	if (ret != m_SrcDescs->end())
	{
		return *ret;
	}
	else
	{
		return nullptr;
	}
}

Qt::CheckState TraverserInitial::OnSrcDescReached(SrcPathDesc* srcPath, const fs::path& entryName, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState)
{
	return ScopedRecursion(entryName, srcPath, dirCheckState);
}

void TraverserInitial::GetDeeperSrcDescs(const fs::path& srcEntryPath, vector<SrcPathDesc*>& subSrcDescs)
{
	copy_if(m_SrcDescs->begin(), m_SrcDescs->end(), back_inserter(subSrcDescs), [&srcEntryPath](const SrcPathDesc* e)->bool
	{
		auto p = mismatch(e->m_Path.begin(), e->m_Path.end(), srcEntryPath.begin(), srcEntryPath.end());
		return p.second == srcEntryPath.end();
	});
}

Qt::CheckState TraverserInitial::OnDeeperSrcDescsReached(const fs::path& entryName, vector<SrcPathDesc*>& subSrcDescs, EntryCheckStateRec& dirCheckState)
{
	return InitialRecursion(entryName, &subSrcDescs, dirCheckState);
}







TraverserScoped::TraverserScoped(TraversableModelThread* traversableThread, const fs::path& originPath, const fs::path& srcOriginPath, SrcPathDesc* srcPath, unordered_map<QString, EntryCheckStateRec>* entry2CheckState)
	: TraverserAbstract(traversableThread, originPath, srcOriginPath, entry2CheckState)
	, m_SrcDesc(srcPath)
{	
	SortFilters(*m_SrcDesc);
}

void TraverserScoped::Traverse()
{
	TraverseSrcPathEntry<TraverserScoped, TraverserScoped>();
	/*if (m_SrcDesc->m_Include)
		TraverseSrcPathEntry<TraverserScoped, TraverserScoped>();
	else
		TraverseSrcPathEntry<TraverserInitial, TraverserInitial>();*/
}




SrcPathDesc* TraverserScoped::FindSubSrcDescForEntry(const fs::path& entryName)
{
	auto ret = find_if(m_SrcDesc->m_SubSrcPathDescs.begin(), m_SrcDesc->m_SubSrcPathDescs.end(), [&entryName](const SrcPathDesc& e)->bool
	{
		 return e.m_Path == entryName;
	});

	if (ret != m_SrcDesc->m_SubSrcPathDescs.end())
	{
		return &*ret;
	}
	else
	{
		return nullptr;
	}
}

Qt::CheckState TraverserScoped::OnSrcDescReached(SrcPathDesc* srcPath, const fs::path& entryName, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState)
{
	return ScopedRecursion(entryName, srcPath, dirCheckState);
}

void TraverserScoped::GetDeeperSrcDescs(const fs::path& srcEntryPath, vector<SrcPathDesc*>& subSrcDescs)
{	
	for (SrcPathDesc& e : m_SrcDesc->m_SubSrcPathDescs)
	{
		auto p = mismatch(e.m_Path.begin(), e.m_Path.end(), srcEntryPath.begin(), srcEntryPath.end());
		if (p.second == srcEntryPath.end())
		{
			subSrcDescs.push_back(&e);
		}
	}
}

Qt::CheckState TraverserScoped::OnDeeperSrcDescsReached(const fs::path& entryName, vector<SrcPathDesc*>& subSrcDescs, EntryCheckStateRec& dirCheckState)
{
	return InitialRecursion(entryName, &subSrcDescs, dirCheckState);
}


void TraverserUnscoped::Traverse()
{


	TraverseSrcPathEntry<TraverserUnscoped,TraverserScoped>();
}

Qt::CheckState TraverserUnscoped::OnSrcDescReached(SrcPathDesc* srcPath, const fs::path& entryName, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState)
{
	QString qEntryName = QString::fromStdWString(entryName);
	auto it = m_Entry2CheckState->find(qEntryName);
	assert(it != m_Entry2CheckState->end());
	dirCheckState.m_SubEntries.emplace(qEntryName, it->second);
	return it->second.m_CheckState;
}

Qt::CheckState TraverserUnscoped::OnDeeperSrcDescsReached(const fs::path& entryName, vector<SrcPathDesc*>& subSrcDescs, EntryCheckStateRec& dirCheckState)
{
	return ForwardRecursion(entryName, &subSrcDescs, dirCheckState);
}

void TraverserForward::Traverse()
{
	m_SrcPathCheckState = TraverseAsDirEntry<TraverserForward, TraverserInitial>(m_TraversePath);
}

Qt::CheckState TraverserForward::OnSrcDescReached(SrcPathDesc* srcPath, const fs::path& entryName, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState)
{
	QString qEntryName = QString::fromStdWString(entryName);
	auto it = m_Entry2CheckState->find(qEntryName);
	assert(it != m_Entry2CheckState->end());
	dirCheckState.m_SubEntries.emplace(qEntryName, it->second);
	return it->second.m_CheckState;
}

Qt::CheckState TraverserForward::OnDeeperSrcDescsReached(const fs::path& entryName, vector<SrcPathDesc*>& subSrcDescs, EntryCheckStateRec& dirCheckState)
{
	return ForwardRecursion(entryName, &subSrcDescs, dirCheckState);
}
