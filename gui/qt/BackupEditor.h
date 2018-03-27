#pragma once


namespace Ui {
class BackupEditor;
}


class FilterEditor;
class MainWindow;
class BackupModel;


class BackupEditor : public QWizard
{
    Q_OBJECT
public:
    explicit BackupEditor(QWidget *parent = 0);
	~BackupEditor();
	
	void SetNewBackup(MyBCopyFile* mbcFile);
	void SetBackup(MyBCopyFile* mbcFile, const ParsedSources& backup);
	ParsedSources GetResult() const;

	bool validateCurrentPage() override;
private slots:	
	void OverrideStorages(bool overrided);
	void OverrideToolchain(bool overrided);
	void OverrideScheme(bool overrided);
		    
	void on_rotateButton_clicked();

	void on_DepthAction_triggered();
	
	void on_AddWildcardDialogAction_triggered();
	void on_AddRegExpDialogAction_triggered();
	
	void on_SetWildcardDialogAction_triggered();
	void on_SetRegExpDialogAction_triggered();
	    
	void on_RemoveSelectedAction_triggered();

	
		
		
    void on_SrcDescTable_doubleClicked(const QModelIndex &idx);
    void on_SrcDescTable_customContextMenuRequested(const QPoint &pos);		  
	
    
    void on_BackupEditor_finished(int result);

	void on_FileSystemTreeView_customContextMenuRequested(const QPoint &pos);

	bool ValidateBackupName();

    

    void on_AddInclusionSrcPathAction_triggered();
    void on_AddExclusionSrcPathAction_triggered();
    void on_RemoveSrcPathAction_triggered();
    void on_TraverseAction_triggered();
    void on_AddWildcardDialogFSAction_triggered();
    void on_AddRegExpDialogFSAction_triggered();
    void on_DepthFSAction_triggered();
	void OnFSTraverseCompleted();

private:
    Ui::BackupEditor* ui;
	
	MyBCopyFile* m_MyBCopyFile;
	ParsedSources	m_Backup;
	BackupModel*	m_BackupModel;
	QSortFilterProxyModel* m_ProxyBackupModel;
		
	
	void ForEachSelectedSrc(function<void(const QModelIndex&, QStandardItem*)> func);		
	void ForEachSelectedSrc(const QModelIndexList& selectedRows, function<void(const QModelIndex&, QStandardItem*)> func);
				
	void ExecAddFilterDialog(FilterEditor* fileEditor, type_index filterType, const list<QModelIndex>& idxSrcs);
	void ExecEditFilterDialog(FilterEditor* fileEditor, type_index filterType);	
	void ExecDepthDialog(const list<QPersistentModelIndex>& persistentSelectedSrcs);
		
	list<QModelIndex> GetSelectedInclusionSrcPaths();
	QModelIndex GetSelectedIndexFS();
	QModelIndex GetSelectedIdxSrcInFS();
};


//class BackupNameValidator : public QValidator
//{
//	Q_OBJECT
//public:
//	explicit BackupNameValidator(BackupEditor *parent);
//	virtual State validate(QString &input, int &pos) const override;
//};
