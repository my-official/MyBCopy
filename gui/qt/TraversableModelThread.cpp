#include "qtgui_pch.h"
#include "TraversableModelThread.h"
#include "BackupModel.h"
#include "TraversableModelAlgorithms.h"




TraversableModelThread::TraversableModelThread(TraversableFSModel* model, QObject *parent)
	: QObject(parent),
	m_TraversableFSModel(model)
{
}

TraversableModelThread::~TraversableModelThread() = default;



void TraversableModelThread::on_TraverseRequired(int requestId, const ContainerOfSrcPathDesc& _srcList)
{
	ContainerOfSrcPathDesc& srcList = const_cast<ContainerOfSrcPathDesc&>(_srcList);

	ParsedSources parsedSources;	
	parsedSources.m_SrcPaths = srcList;

	parsedSources.NormalizeRecursive();

	unordered_map<QString, EntryCheckStateRec> rootEntries2CheckState;
	
		

	map<fs::path, vector<SrcPathDesc*> > rootSet;

	for (auto& srcDesc : parsedSources.m_SrcPaths)
	{
		auto rootPath = srcDesc.m_Path.root_path();
		rootSet[rootPath].push_back(&srcDesc);
	}


	for (auto& root : rootSet)
	{
		auto& srcDescs = root.second;

		if (srcDescs.size() == 1)
		{
			SrcPathDesc* srcDesc = srcDescs.front();
			if (root.first == srcDesc->m_Path)
			{
				TraverserScoped traverser(this, root.first, root.first, srcDesc, &rootEntries2CheckState);
				traverser.Traverse();
			}
			else
			{
				TraverserInitial traverser(this, root.first, root.first, &srcDescs, &rootEntries2CheckState);
				traverser.Traverse();
			}
		}
		else
		{
			TraverserInitial traverser(this, root.first, root.first, &srcDescs, &rootEntries2CheckState);
			traverser.Traverse();
		}
	}


	DirName2EntryCheckStateMap checkedEntries;

	ConvertTreeToLinear(rootEntries2CheckState, checkedEntries);
	MarkSrcPaths(srcList,checkedEntries);
	
	emit TraverseCompleted(requestId, checkedEntries);
}

void TraversableModelThread::ConvertTreeToLinear(const unordered_map<QString, EntryCheckStateRec>& rootEntries2CheckState, DirName2EntryCheckStateMap& checkedEntries, const QString& parentPath /*= QString()*/)
{
	for (auto& p : rootEntries2CheckState)
	{		
		checkedEntries.emplace(QDir::cleanPath(parentPath + p.first), p.second.m_CheckState);
		ConvertTreeToLinear(p.second.m_SubEntries, checkedEntries, parentPath + p.first + "/");
	}

}

void TraversableModelThread::MarkSrcPaths(const ContainerOfSrcPathDesc& srcList, DirName2EntryCheckStateMap& checkedEntries)
{
	for (auto& srcPath : srcList)
	{
		QString qPath = QString::fromStdWString(srcPath.m_Path);
		qPath = QDir::cleanPath(qPath);

		auto it = checkedEntries.find(qPath);
		if (it == checkedEntries.end())
		{			
			auto p = checkedEntries.emplace(qPath, ExclusionCheckState);
			it = p.first;
		}

		if (srcPath.m_Include)
		{			
			it->second = InclusionCheckState;		
		}
		else
		{
			it->second = ExclusionCheckState;
		}
	}
}

