#pragma once
#include "TraversableFSModel.h"

class TraversableModelThread : public QObject
{
	Q_OBJECT
public:	
	explicit TraversableModelThread(TraversableFSModel* model, QObject *parent = nullptr);
	virtual ~TraversableModelThread();
signals:
	void TraverseCompleted(int requestId, const DirName2EntryCheckStateMap& cache);
public slots:
	void on_TraverseRequired(int requestId, const ContainerOfSrcPathDesc& parsedSources);	
private:
	TraversableFSModel* m_TraversableFSModel;	
	void ConvertTreeToLinear(const unordered_map<QString, EntryCheckStateRec>& rootEntries2CheckState, DirName2EntryCheckStateMap& checkedEntries, const QString& parentPath = QString());
	void MarkSrcPaths(const ContainerOfSrcPathDesc& srcList, DirName2EntryCheckStateMap& checkedEntries);
};