#pragma once
#include "Utils_qtgui.h"

class ParsedSources;
class BackupModel;



const Qt::CheckState IndeterminateCheckState = static_cast<Qt::CheckState>(Qt::CheckState::Checked + 1);

class TraversableModelThread;

struct EntryCheckState
{	
	Qt::CheckState m_CheckState = IndeterminateCheckState;
	int m_TraverseId = 0;
};

struct EntryCheckStateRec : public EntryCheckState
{
	unordered_map<QString, EntryCheckStateRec> m_SubEntries;
};

using DirName2EntryCheckStateMap = unordered_map<QString, Qt::CheckState>;


class TraversableFSModel : public QFileSystemModel
{
	Q_OBJECT
public:
	TraversableFSModel(BackupModel* backupModel, QObject*parent = Q_NULLPTR);
	virtual ~TraversableFSModel();
	void SetBackupModel(BackupModel* backup);
	
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);

private slots:
	void InvokeTraverse();
	void on_TraverseCompleted(int requestId, const DirName2EntryCheckStateMap& checkedEntries);
private:
	BackupModel* m_BackupModel;
	int m_CurrRequestId;
	int m_NextRequestId;
	QThread* m_Thread;
	TraversableModelThread* m_Traverser;
	int GenerateNextRequestId();
		
	DirName2EntryCheckStateMap m_Cache;
		
	void emitCheckStateChanged_AllChildren(const QModelIndex& parentIdx);
};