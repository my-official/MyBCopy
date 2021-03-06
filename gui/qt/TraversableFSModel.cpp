#include "BackupModel.h"
#include "ParsedSources.h"
#include "TraversableModelThread.h"



Q_DECLARE_METATYPE(ContainerOfSrcPathDesc)
Q_DECLARE_METATYPE(DirName2EntryCheckStateMap)



static bool InitModuleHandler()
{
	qRegisterMetaType<ContainerOfSrcPathDesc>("ContainerOfSrcPathDesc");
	qRegisterMetaType<DirName2EntryCheckStateMap>("DirName2EntryCheckStateMap");
	return true;
}

static void InitModule()
{
	static bool wasInit = InitModuleHandler();
}




int TraversableFSModel::GenerateNextRequestId()
{
	int result = m_NextRequestId++;

	if (m_NextRequestId >= 10000)
		m_NextRequestId = 1;

	return result;
}


void TraversableFSModel::emitCheckStateChanged_AllChildren(const QModelIndex& parentIdx)
{
	int numRows = rowCount(parentIdx);

	if (numRows <= 0)
		return;
	

	QModelIndex first = index(0, 0, parentIdx);
	QModelIndex last = index(numRows, 0, parentIdx);

	emit dataChanged(first, last, { Qt::CheckStateRole });


	for (int c = 0, size = numRows; c < size; c++)
	{
		emitCheckStateChanged_AllChildren(index(c, 0, parentIdx));
	}
}


void TraversableFSModel::InvokeTraverse()
{
	auto parsedSources = m_BackupModel->GetSourceList();
	m_CurrRequestId = GenerateNextRequestId();

	emitCheckStateChanged_AllChildren(QModelIndex());
	QMetaObject::invokeMethod(m_Traverser, "on_TraverseRequired", Qt::QueuedConnection, Q_ARG(int, m_CurrRequestId), Q_ARG(const ContainerOfSrcPathDesc&, *parsedSources));
}

void TraversableFSModel::on_TraverseCompleted(int requestId, const DirName2EntryCheckStateMap& checkedEntries)
{
	if (requestId != m_CurrRequestId)
		return;

	m_Cache.swap(const_cast<DirName2EntryCheckStateMap&>(checkedEntries));
	m_CurrRequestId = 0;
	emitCheckStateChanged_AllChildren(QModelIndex());
	emit TraverseCompleted();
}

void TraversableFSModel::on_ContainerReset()
{
	m_Cache.clear();
	emitCheckStateChanged_AllChildren(QModelIndex());
	auto srcList = m_BackupModel->GetSourceList();
	for (auto& srcDesc : *srcList)
	{
		QString qPath = QDir::fromNativeSeparators(QString::fromStdWString(srcDesc.m_Path));
		m_Cache[qPath] = srcDesc.m_Include ? InclusionCheckState : ExclusionCheckState;

		QModelIndex idx = index(qPath);
		if (idx.isValid())
			emit dataChanged(idx, idx, { Qt::CheckStateRole });
	}
}

void TraversableFSModel::on_ElementAdded(const QModelIndex& backupModelIdx)
{
	SrcPathDesc& srcDesc = m_BackupModel->GetSrcDescByIndex(backupModelIdx);
	QString qPath = QDir::fromNativeSeparators(QString::fromStdWString(srcDesc.m_Path));
	m_Cache[qPath] = srcDesc.m_Include ? InclusionCheckState : ExclusionCheckState;

	QModelIndex idx = index(qPath);
	if (idx.isValid())
		emit dataChanged(idx, idx, { Qt::CheckStateRole });
}

void TraversableFSModel::on_ElementChanged(const QModelIndex& backupModelIdx)
{

}

void TraversableFSModel::on_ElementBeforeRemove(int row, int count)
{
	for (int c = 0; c < count; c++)
	{
		SrcPathDesc& srcDesc = m_BackupModel->GetSrcDescByIndex(m_BackupModel->index(row + c, 0));
		QString qPath = QDir::fromNativeSeparators(QString::fromStdWString(srcDesc.m_Path));
		m_Cache.erase(qPath);

		QModelIndex idx = index(qPath);
		if (idx.isValid())
			emit dataChanged(idx, idx, { Qt::CheckStateRole });
	}
}

TraversableFSModel::TraversableFSModel(BackupModel* backupModel, QObject* parent /*= Q_NULLPTR*/)
	: QFileSystemModel(parent)
	, m_BackupModel(backupModel)
	, m_CurrRequestId(0)
	, m_NextRequestId(1)
{
	connect(m_BackupModel, SIGNAL(ContainerReset()), this, SLOT(on_ContainerReset()));
	connect(m_BackupModel, SIGNAL(ElementAdded(const QModelIndex&)), this, SLOT(on_ElementAdded(const QModelIndex&)));
	//connect(m_BackupModel, SIGNAL(ElementChanged(const QModelIndex&)), this, SLOT(InvokeTraverse()));
	connect(m_BackupModel, SIGNAL(ElementBeforeRemove(int, int)), this, SLOT(on_ElementBeforeRemove(int, int)));

	
	InitModule();

	m_Thread = new QThread;
	connect(m_Thread, SIGNAL(finished()), m_Thread, SLOT(deleteLater()));
	m_Traverser = new TraversableModelThread(this, nullptr);
	m_Traverser->moveToThread(m_Thread);	
	connect(m_Traverser, SIGNAL(TraverseCompleted(int,const DirName2EntryCheckStateMap&)), this, SLOT(on_TraverseCompleted(int, const DirName2EntryCheckStateMap&)));
	
	connect(m_Thread, SIGNAL(finished()), m_Traverser, SLOT(deleteLater()));
	m_Thread->start();
}

TraversableFSModel::~TraversableFSModel()
{
	if (m_Thread)
	{
		m_Thread->quit();
		m_Thread = nullptr;
	}
}

void TraversableFSModel::SetBackupModel(BackupModel* backup)
{
	m_BackupModel = backup;
}

Qt::ItemFlags TraversableFSModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags result = QFileSystemModel::flags(index);
	if (index.column() == 0)
	{
		result |= /*Qt::ItemIsUserTristate | */ Qt::ItemIsUserCheckable;
	}
	return result;
}
QVariant TraversableFSModel::data(const QModelIndex &index, int role) const
{
	if (role != Qt::CheckStateRole || index.column() != 0)
		return QFileSystemModel::data(index, role);
	
	if (m_CurrRequestId != 0)
		return IndeterminateCheckState;
	
	QString entryFullPath = QDir::cleanPath(QFileSystemModel::filePath(index));
	
	auto it = m_Cache.find(entryFullPath);

	if (it != m_Cache.end())
		return it->second;

	return Qt::Unchecked;

	//decltype(m_Cache.find(entryFullPath)) it;

	//do
	//{
	//	it = m_Cache.find(entryFullPath);

	//	if (it != m_Cache.end())
	//		return it->second;

	//	int pos = entryFullPath.lastIndexOf('\\');
	//	if (pos == -1)
	//		break;

	//	entryFullPath = entryFullPath.left(pos + 1);
	//	
	//} while (true);
	//
	//return Qt::Unchecked;		
}


//
//bool TraversableFSModel::setData(const QModelIndex &index, const QVariant& value, int role)
//{
//	if (role != Qt::CheckStateRole || index.column() != 0)
//		return QFileSystemModel::setData(index, value, role);
//			
//	Qt::CheckState prevCheckState = static_cast<Qt::CheckState>(data(index, Qt::CheckStateRole).toInt());
//
//	if (prevCheckState == IndeterminateCheckState)
//		return false;
//
//	QString qFilePath = QFileSystemModel::filePath(index);
//	fs::path fsPath = qFilePath.toStdWString();	
//
//	auto idxSrcDesc = m_BackupModel->FindSrcDescByPath(fsPath);
//
//	if (!idxSrcDesc.isValid())
//	{
//		///Add	
//
//		SrcPathDesc newSrcPath;
//		newSrcPath.m_Path = fsPath;
//
//		if (prevCheckState == Qt::Checked)
//			newSrcPath.m_Include = false;//Exclusion
//
//		m_BackupModel->Add(newSrcPath);
//	}
//	else
//	{
//		
//		///Remove
//		auto result = QMessageBox::question(QApplication::activeWindow(), tr("Are you sure?"), tr("Source path rule will be remove"), QMessageBox::Yes, QMessageBox::No);
//		if (result != QMessageBox::Yes)
//		{
//			return false;
//		}
//
//		m_BackupModel->removeRow(idxSrcDesc.row(), idxSrcDesc.parent());
//	}
//
//	//emit dataChanged(index, index, { Qt::CheckStateRole });
//
//	return true;
//}
//
