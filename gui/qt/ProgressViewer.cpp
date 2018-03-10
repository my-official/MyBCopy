#include "qtgui_pch.h"
#include "ProgressViewer.h"
#include "ui_ProgressViewer.h"
#include "MainWindow.h"
#include "ModelIterator.h"



Q_DECLARE_METATYPE(ParsedSources)
Q_DECLARE_METATYPE(string)
Q_DECLARE_METATYPE(wstring)
Q_DECLARE_METATYPE(EBackupError)
Q_DECLARE_METATYPE(EBackupStage)
Q_DECLARE_METATYPE(EStorageStage)
Q_DECLARE_METATYPE(const MyBCopyFile*)



static bool InitModuleHandler()
{
	qRegisterMetaType<ParsedSources>("ParsedSources");
	qRegisterMetaType<string>("string");
	qRegisterMetaType<wstring>("wstring");
	qRegisterMetaType<EBackupError>("EBackupError");
	qRegisterMetaType<EBackupStage>("EBackupStage");
	qRegisterMetaType<EStorageStage>("EStorageStage");
	qRegisterMetaType<const MyBCopyFile*>("const MyBCopyFile*");
	return true;
}

static void InitModule()
{
	static bool wasInit = InitModuleHandler();
}



ProgressViewer::ProgressViewer(QWidget *parent)
	: QWidget(parent)
    , ui(new Ui::ProgressViewer)	
{	
	ui->setupUi(this);

	auto progressModel = new ProgressModel(this);
	ui->TreeView->setModel(progressModel);
	m_ProgressModel = progressModel;

	InitModule();

	m_Thread = new QThread;
	connect(m_Thread, SIGNAL(finished()), m_Thread, SLOT(deleteLater()));
	m_ReserverThread = new ReserverThread(this);
	m_ReserverThread->moveToThread(m_Thread);
	connect(m_Thread, SIGNAL(finished()), m_ReserverThread, SLOT(deleteLater()));
	
	connect(m_ReserverThread, SIGNAL(Progress(const ParsedSources&, EBackupStage)), this, SLOT(OnProgress(const ParsedSources&, EBackupStage)));
	connect(m_ReserverThread, SIGNAL(Finish(const ParsedSources&, EBackupError)), this, SLOT(OnFinish(const ParsedSources&, EBackupError)));
	
	connect(m_ReserverThread, SIGNAL(ErrorStorage(const ParsedSources&, const wstring&, EBackupError)), this, SLOT(OnErrorStorage(const ParsedSources&, const wstring&, EBackupError)));
	connect(m_ReserverThread, SIGNAL(ProgressStorage(const ParsedSources&, const wstring&, EStorageStage)), this, SLOT(OnProgressStorage(const ParsedSources&, const wstring&, EStorageStage)));
	
	m_Thread->start();
}

ProgressViewer::~ProgressViewer()
{
	if (m_Thread)
	{
		m_Thread->quit();
		m_Thread = nullptr;
	}

    delete ui;
}

void ProgressViewer::SetInputFile(const MyBCopyFile& myBCopyFile)
{
	m_MyBCopyFile = myBCopyFile;
	m_ProgressModel->SetMyBCopyFile(&m_MyBCopyFile);	
	ui->TreeView->expandAll();
		
	ui->ProgressBar->setValue(0);	
	

	//int countStorages = 0;
	//
	//for (auto& backup : m_MyBCopyFile.m_Backups)
	//{
	//	countStorages += Settings::Override(m_MyBCopyFile.m_Settings, backup.m_Settings).m_Storages.size();
	//}
	//countStorages = std::max<int>(1, countStorages);

	ui->ProgressBar->setMaximum(m_MyBCopyFile.m_Backups.size());
	ui->PushButton->setDisabled(true);
	QMetaObject::invokeMethod(m_ReserverThread, "Backup", Qt::QueuedConnection, Q_ARG(const MyBCopyFile*, &m_MyBCopyFile));

}

void ProgressViewer::on_PushButton_clicked()
{
	MainWindow::sm_Instance->SwitchToStartupPage();
}


void ProgressViewer::OnProgress(const ParsedSources& backup, EBackupStage stage)
{
	QString	stageText = EBackupStage2String(stage);
	m_ProgressModel->SetBackupStageText(backup.m_Name, stageText);
}

void ProgressViewer::OnFinish(const ParsedSources& backup, EBackupError error)
{
	QString stageText = EBackupError2String(error);	
	m_ProgressModel->SetBackupStageText(backup.m_Name, stageText);	
	ui->ProgressBar->setValue(ui->ProgressBar->value() + 1);
}

void ProgressViewer::OnErrorStorage(const ParsedSources& backup, const wstring& storageName, EBackupError stage)
{
	QString	stageText = EBackupError2String(stage);
	m_ProgressModel->SetStorageStageText(backup.m_Name, storageName, stageText);	
}

void ProgressViewer::OnProgressStorage(const ParsedSources& backup, const wstring& storageName, EStorageStage stage)
{
	QString	stageText = EStorageStage2String(stage);
	m_ProgressModel->SetStorageStageText(backup.m_Name, storageName, stageText);		
}

void ProgressViewer::OnBackupFinish()
{	
	ui->PushButton->setEnabled(true);
}

ProgressModel::ProgressModel(QObject*parent /*= Q_NULLPTR*/)
	: ListModel< ParsedSources >(parent)
{
	setHorizontalHeaderLabels({ tr("Backup"), tr("Stage") });
}


void ProgressModel::SetMyBCopyFile(MyBCopyFile* mbcFile)
{
	m_MyBCopyFile = mbcFile;
	SetSourceList(&m_MyBCopyFile->m_Backups);
}

void ProgressModel::SetBackupStageText(const wstring& backupName, const QString& stageText)
{
	QString qBackupName = QString::fromStdWString(backupName);
	QModelIndex idx = GetIndexByText(qBackupName);	
	assert(idx.isValid());
	idx = idx.sibling(idx.row(), 1);
	setData(idx, stageText);
}

void ProgressModel::SetStorageStageText(const wstring& backupName, const wstring& storageName, const QString& stageText)
{
	QString qBackupName = QString::fromStdWString(backupName);
	QModelIndex idx = GetIndexByText(qBackupName);

	QString qStorageName = QString::fromStdWString(storageName);
	idx = GetIndexByText(qStorageName, idx);
	assert(idx.isValid());
	idx = idx.sibling(idx.row(), 1);
	setData(idx, stageText);
}

QModelIndex ProgressModel::GetIndexByText(const QString& text, const QModelIndex& parentIdx /*= QModelIndex()*/)
{
	ModelIterator begin_it(this, 0, 0, parentIdx);
	ModelIterator end_it(this, rowCount(parentIdx), 0, parentIdx);

	ModelIterator it = find_if(begin_it, end_it, [&text](const QModelIndex& idx)->bool
	{
		return text == idx.data().toString();
	});

	assert(it != end_it);

	if (it == end_it)
		throw EXCEPTION( BaseException() );
	
	return *it;
}

void ProgressModel::ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& it)
{	
	newRow << new QStandardItem(QString::fromStdWString(it->m_Name))
		<< new QStandardItem(tr("Preparing"));
		
	auto settings = Settings::Override(m_MyBCopyFile->m_Settings, it->m_Settings);

	QStandardItem* col0Item = newRow.front();

	for (auto& storage : settings.m_Storages)
	{
		QList<QStandardItem *> newStoarge;
		newStoarge << new QStandardItem(QString::fromStdWString(storage->m_Name))
			<< new QStandardItem(QString(""));

		col0Item->appendRow(newStoarge);
	}
}

ReserverThread::ReserverThread(ProgressViewer* progressViewer)
	: QObject(nullptr)
	, m_ProgressViewer(progressViewer)
{
	InitModule();

	m_Notificators.m_ProgressNotificator = [&](const ParsedSources& backup, EBackupStage stage)
	{
		emit Progress(backup, stage);
		//QMetaObject::invokeMethod(this, "Progress", Qt::QueuedConnection, Q_ARG(const ParsedSources&, backup), Q_ARG(EBackupStage, stage));
	};

	m_Notificators.m_FinishNotificator = [&](const ParsedSources& backup, EBackupError error)
	{
		emit Finish(backup, error);
		//QMetaObject::invokeMethod(this, "Finish", Qt::QueuedConnection, Q_ARG(const ParsedSources&, backup), Q_ARG(EBackupError, error));
	};

	m_Notificators.m_ErrorStorageNotificator = [&](const ParsedSources& backup, const wstring& storageName, EBackupError error)
	{
		emit ErrorStorage(backup, storageName, error);
		//QMetaObject::invokeMethod(this, "ErrorStorage", Qt::QueuedConnection, Q_ARG(const ParsedSources&, backup), Q_ARG(const wstring&, storageName), Q_ARG(EBackupError, error));
	};

	m_Notificators.m_ProgressStorageNotificator = [&](const ParsedSources& backup, const wstring& storageName, EStorageStage stage)
	{
		emit ProgressStorage(backup, storageName, stage);
		//QMetaObject::invokeMethod(this, "ProgressStorage", Qt::QueuedConnection, Q_ARG(const ParsedSources&, backup), Q_ARG(const wstring&, storageName), Q_ARG(EStorageStage, stage));
	};

}

void ReserverThread::Backup(const MyBCopyFile* myBCopyFile)
{
	m_Reserver.Backup(*myBCopyFile,m_Notificators);
	QMetaObject::invokeMethod(m_ProgressViewer, "OnBackupFinish", Qt::QueuedConnection);	
}



