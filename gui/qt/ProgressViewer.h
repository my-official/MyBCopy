#pragma once
#include "BackupModel.h"

namespace Ui {
class ProgressViewer;
}
class ProgressModel;
class ReserverThread;

class ProgressViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressViewer(QWidget *parent = 0);
    ~ProgressViewer();

	void SetInputFile(const MyBCopyFile& myBCopyFile);
private slots:
    void on_PushButton_clicked();

	void OnProgress(const ParsedSources& backup, EBackupStage stage);
	void OnFinish(const ParsedSources& backup, EBackupError error);
	void OnErrorStorage(const ParsedSources& backup, const wstring& storageName, EBackupError stage);
	void OnProgressStorage(const ParsedSources& backup, const wstring& storageName, EStorageStage stage);

	friend class ReserverThread;
	void OnBackupFinish();
private:	
    Ui::ProgressViewer *ui;
	
	MyBCopyFile m_MyBCopyFile;
	ProgressModel* m_ProgressModel;

	QThread* m_Thread;
	ReserverThread* m_ReserverThread;	
};





class ProgressModel : public ListModel< ParsedSources >
{
	Q_OBJECT
public:
	explicit ProgressModel(QObject*parent = Q_NULLPTR);	
	void SetMyBCopyFile(MyBCopyFile* mbcFile);
	void SetBackupStageText(const wstring& backupName, const QString& stageText);
	void SetStorageStageText(const wstring& backupName, const wstring& storageName, const QString& stageText);
	QModelIndex GetIndexByText(const QString& text, const QModelIndex& parentIdx = QModelIndex());
protected:
	using ListModel< ParsedSources >::SetSourceList;
	virtual void ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& it) override;
	MyBCopyFile* m_MyBCopyFile;
};




class ReserverThread : public QObject
{
	Q_OBJECT
public:
	explicit ReserverThread(ProgressViewer* progressViewer);
signals:		
	void Progress(const ParsedSources& backup, EBackupStage stage);
	void Finish(const ParsedSources& backup, EBackupError error);
	void ErrorStorage(const ParsedSources& backup, const wstring& storageName, EBackupError stage);
	void ProgressStorage(const ParsedSources& backup, const wstring& storageName, EStorageStage stage);
public slots:
	void Backup(const MyBCopyFile* myBCopyFile);
private:
	ProgressViewer* m_ProgressViewer;
	Reserver m_Reserver;
	Notificators m_Notificators;
};