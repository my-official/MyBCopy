#include "qtgui_pch.h"
#include "BackupEditor.h"
#include "ui_BackupEditor.h"
#include "RegexpFilterEditor.h"
#include "BackupModel.h"
#include "DepthEditor.h"
#include "TraversableFSModel.h"
#include "MainWindow.h"


BackupEditor::BackupEditor(QWidget *parent)
	: QWizard(parent)
    , ui(new Ui::BackupEditor)
	, m_BackupModel(new BackupModel(this))
	, m_ProxyBackupModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);
	
	
	m_ProxyBackupModel->setSourceModel(m_BackupModel);
	ui->SrcDescTable->setModel(m_ProxyBackupModel);

	ui->SrcDescTable->sortByColumn(0, Qt::AscendingOrder);	
	
	//assert(ui->SrcDescTable->isSortingEnabled());
	//ui->SrcDescTable->setSortingEnabled(true);
	//QHeaderView* header = ui->SrcDescTable->header();
	//header->setDefaultSectionSize(250);

	TraversableFSModel* newBackup_FileSystemModel = new TraversableFSModel(m_BackupModel, ui->FileSystemTreeView);
	newBackup_FileSystemModel->setRootPath("");
	ui->FileSystemTreeView->setModel(newBackup_FileSystemModel);
	ui->FileSystemTreeView->sortByColumn(0,Qt::AscendingOrder);

	connect(newBackup_FileSystemModel, SIGNAL(TraverseCompleted()), this, SLOT(OnFSTraverseCompleted()));

	QHeaderView* header = ui->FileSystemTreeView->header();
	header->setDefaultSectionSize(250);
}

BackupEditor::~BackupEditor()
{
    delete ui;
}



void BackupEditor::SetNewBackup(MyBCopyFile* mbcFile)
{
	m_MyBCopyFile = mbcFile;	
	m_Backup = ParsedSources();

	wstring archiveName = L"NewBackup";

	auto it = find_if(m_MyBCopyFile->m_Backups.begin(), m_MyBCopyFile->m_Backups.end(), [&archiveName](const ParsedSources& backup)->bool
	{
		return backup.m_Name == archiveName;
	});

	int counter = 1;
	while (it != m_MyBCopyFile->m_Backups.end())
	{
		archiveName = L"NewBackup" + to_wstring(counter++);

		it = find_if(m_MyBCopyFile->m_Backups.begin(), m_MyBCopyFile->m_Backups.end(), [&archiveName](const ParsedSources& backup)->bool
		{
			return backup.m_Name == archiveName;
		});
	}

	m_Backup.m_Name = std::move(archiveName);

	SetBackup(mbcFile, m_Backup);
}

//void BackupEditor::on_LinkNamePage_clicked()
//{
//	ui->NewBackupStack->setCurrentWidget(ui->BackupNamePage);
//}
//
//
//void BackupEditor::on_LinkSrcEditorPage_clicked()
//{
//	ui->NewBackupStack->setCurrentWidget(ui->SrcEditorPage);
//}
//
//void BackupEditor::on_LinkStoragePage_clicked()
//{
//	ui->NewBackupStack->setCurrentWidget(ui->StoragesPage);
//}
//
//
//void BackupEditor::on_LinkToolChainPage_clicked()
//{
//	ui->NewBackupStack->setCurrentWidget(ui->ToolChainPage);
//}
//
//
//void BackupEditor::on_LinkSchemePage_clicked()
//{
//	ui->NewBackupStack->setCurrentWidget(ui->SchemePage);
//}
//
//
//void BackupEditor::on_LinkOkButton_clicked()
//{
//
//}



void BackupEditor::on_rotateButton_clicked()
{
	QSplitter* FrameSplitter = ui->FrameSplitter;

	assert(FrameSplitter->count() == 2);

	bool normal = FrameSplitter->widget(0) == ui->FirstFrame;
	bool horizontal = FrameSplitter->orientation() == Qt::Orientation::Horizontal;

	if (normal)
	{
		if (horizontal)
		{
			FrameSplitter->setOrientation(Qt::Orientation::Vertical);
		}
		else
		{
			FrameSplitter->insertWidget(0, ui->SecondFrame);
			FrameSplitter->insertWidget(1, ui->FirstFrame);
			FrameSplitter->setOrientation(Qt::Orientation::Horizontal);
		}
	}
	else
	{
		if (horizontal)
		{
			FrameSplitter->setOrientation(Qt::Orientation::Vertical);
		}
		else
		{
			FrameSplitter->insertWidget(0, ui->FirstFrame);
			FrameSplitter->insertWidget(1, ui->SecondFrame);
			FrameSplitter->setOrientation(Qt::Orientation::Horizontal);
		}
	}
}

void BackupEditor::on_DepthAction_triggered()
{
	QItemSelectionModel* selectionModel = ui->SrcDescTable->selectionModel();
	assert(selectionModel->hasSelection());

	QModelIndexList selectedRows = selectionModel->selectedRows();
	list<QPersistentModelIndex> persistentSelectedSrcs;
	
	for (auto it = selectedRows.begin(); it != selectedRows.end(); ++it)
	{
		const QModelIndex& proxyIdx = *it;
		QModelIndex srcDescIdx = m_ProxyBackupModel->mapToSource(proxyIdx);
		if (srcDescIdx.parent().isValid())
			srcDescIdx = srcDescIdx.parent();
		
		const SrcPathDesc& srcPathDesc = m_BackupModel->GetSrcDescByIndex(srcDescIdx);
		if (srcPathDesc.m_Include)
			persistentSelectedSrcs.emplace_back(srcDescIdx);
	}
	
	ExecDepthDialog(persistentSelectedSrcs);
}

void BackupEditor::on_RemoveSelectedAction_triggered()
{
	RemoveSelectedRowsFromModel(ui->SrcDescTable);
}


void BackupEditor::on_SrcDescTable_doubleClicked(const QModelIndex &idx)
{
	auto itemType = static_cast<BackupModel::EItemType>(idx.data(BackupModel::ItemTypeRole).toInt());

	switch (itemType)
	{
	case BackupModel::EIT_Depth:
	{
		on_DepthAction_triggered();
		//SrcPathDesc& srcDesc = GetSrcDescByIndex(idx);
		//m_DepthEditor.SetDepth(srcDesc.m_Depth);

		//int returnCode = m_DepthEditor.exec();
		//if (returnCode != QDialog::DialogCode::Accepted)
		//	return;

		//auto depth = m_DepthEditor.GetDepth();
		//model->UpdateDepth(idx, depth);
	}
	break;
	case BackupModel::EIT_Filter:
	{
		FilterIterator filter_it = m_BackupModel->GetFilterIteratorByIndex(idx);
		Filters::FilterBase* filter = filter_it->get();

		auto* filterImpl = FilterImplementatorSelector::Instance().GetFilterImplementator(filter);

		unique_ptr<FilterEditor> filterEditor = filterImpl->MakeEditorDialog();		
		ExecEditFilterDialog(filterEditor.get(), FilterImplementatorSelector::GetFilterTypeIndex(filter));
	}
	break;
	default:break;
	}
}


//ParsedSources* BackupEditor::GetCurrentBackup()
//{
//	return m_CurrentBackup;
//}

void BackupEditor::SetBackup(MyBCopyFile* mbcFile, const ParsedSources& backup)
{
	m_MyBCopyFile =  mbcFile;	
	m_Backup = backup;	

	///Name page
	ui->BackupNameEdit->setText(QString::fromStdWString(m_Backup.m_Name));
	ui->CommentEdit->setPlainText(QString::fromStdWString(m_Backup.m_Comment));

	///Src page	
	m_BackupModel->SetBackup(&m_Backup);


	///Storages page
	bool overridedStorages = !m_Backup.m_Settings.m_Storages.empty();
	ui->StoragesGroupBox->setChecked(overridedStorages);
	OverrideStorages(overridedStorages);

	///ToolChainPage	
	bool overridedToolchain = !m_Backup.m_Settings.m_Toolchain.empty();		
	ui->ToolChainGroupBox->setChecked(overridedToolchain);
	OverrideToolchain(overridedToolchain);	

	///SchemePage
	bool overridedScheme = static_cast<bool>( m_Backup.m_Settings.m_SchemeParams );	
	ui->SchemeGroupBox->setChecked(overridedScheme);
	OverrideScheme(overridedScheme);



	ui->BackupNowCheckBox->setEnabled(false);
	ui->BackupNowCheckBox->setVisible(false);

	restart();
}

ParsedSources BackupEditor::GetResult() const
{
	ParsedSources result;
	result.m_Name = ui->BackupNameEdit->text().toStdWString();
	result.m_Comment = ui->CommentEdit->toPlainText().toStdWString();

	result.m_SrcPaths = m_Backup.m_SrcPaths;
	
	if (ui->StoragesGroupBox->isChecked())
	{
		result.m_Settings.m_Storages = m_Backup.m_Settings.m_Storages;
	}

	if (ui->ToolChainGroupBox->isChecked())
	{
		result.m_Settings.m_Toolchain = m_Backup.m_Settings.m_Toolchain;			
	}

	if (ui->SchemeGroupBox->isChecked())
	{			
		ui->SchemeWidget->GetResult(result.m_Settings);
	}

	return result;
}

bool BackupEditor::validateCurrentPage()
{
	if (currentPage() == ui->BackupNamePage)
	{
		return ValidateBackupName();		
	}

	if (currentPage() == ui->SrcEditorPage)
	{
		bool empty = m_Backup.m_SrcPaths.empty();
		if (empty)
			QMessageBox::critical(this, tr("Error"), tr("No files for backup specified"));		
		return !empty;
	}

	if (currentPage() == ui->StoragesPage && ui->StoragesGroupBox->isChecked())
	{
		bool empty = m_Backup.m_Settings.m_Storages.empty();
		if (empty)
			QMessageBox::critical(this, tr("Error"), tr("No storages specified"));
		return !empty;
	}

	if (currentPage() == ui->ToolChainPage && ui->ToolChainGroupBox->isChecked())
	{
		bool empty = m_Backup.m_Settings.m_Toolchain.empty();
		if (empty)
			QMessageBox::critical(this, tr("Error"), tr("No tools specified"));		
		return !empty;
	}

	return QWizard::validateCurrentPage();
}

//void BackupEditor::NewBackup()
//{
//	m_NewBackup = ParsedSources();	
//	SetCurrentBackup(TODO,&m_NewBackup);
//}




//void BackupEditor::InsertOrAppendFilters(QList<QStandardItem*>& newRow, bool& wasInserted, QStandardItem* itemPath, bool include, EntryType entryType, const vector<shared_ptr<Filters::FilterBase>>& filters)
//{
//	for (auto& filter : filters)
//	{
//		QString filterSpecificText = GetFilterSpecificText(include, entryType, filter.get());
//		QStandardItem* itemFilter = new QStandardItem(filterSpecificText);
//		if (wasInserted)
//		{
//			QList<QStandardItem*> newFilterRow;						
//
//			QStandardItem* item0 = new QStandardItem;
//			newFilterRow.push_back(item0);			
//			newFilterRow.push_back(itemFilter);
//			
//			itemPath->appendRow(newFilterRow);
//		}
//		else
//		{
//			newRow.push_back(itemFilter);
//			wasInserted = true;
//		}		
//	}
//}





void BackupEditor::ForEachSelectedSrc(function<void(const QModelIndex&, QStandardItem*)> func)
{		
	QItemSelectionModel* selectionModel = ui->SrcDescTable->selectionModel();
	QModelIndexList selectedRows = selectionModel->selectedRows();

	ForEachSelectedSrc(selectedRows, func);
}

void BackupEditor::ForEachSelectedSrc(const QModelIndexList& selectedRows, function<void(const QModelIndex&, QStandardItem*)> func)
{
	for (auto& selectedRow : selectedRows)
	{
		auto idx = m_ProxyBackupModel->mapToSource(selectedRow);

		if (m_BackupModel->GetItemTypeByIndex(idx) != BackupModel::EIT_SrcDescPath)
			continue;
		
		QStandardItem* item = m_BackupModel->itemFromIndex(idx);
		func(selectedRow, item);				
	}
}




void BackupEditor::ExecAddFilterDialog(FilterEditor* fileEditor, type_index filterType, const list<QModelIndex>& idxSrcs)
{
	int returnCode = fileEditor->exec();
	if (returnCode != QDialog::DialogCode::Accepted)
		return;

	auto filter = fileEditor->GetFilter();

	for (auto& idx : idxSrcs)
	{
		m_BackupModel->AppendFilter(idx, filter);
	}
}

void BackupEditor::ExecEditFilterDialog(FilterEditor* fileEditor, type_index filterType)
{
	QItemSelectionModel* selectionModel = ui->SrcDescTable->selectionModel();
	QModelIndexList selectedRows = selectionModel->selectedRows();
		
	assert(!selectedRows.empty());

	if (selectedRows.size() == 1)
	{
		QModelIndex idx = m_ProxyBackupModel->mapToSource(selectedRows.front());
		auto oldFilter = m_BackupModel->GetFilter(idx);			
		fileEditor->SetFilter(oldFilter.get());
	}	

	int returnCode = fileEditor->exec();
	if (returnCode != QDialog::DialogCode::Accepted)
		return;

	auto newFilter = fileEditor->GetFilter();

	for (auto& selectedRow : selectedRows)
	{
		selectedRow = m_ProxyBackupModel->mapToSource(selectedRow);
		if (m_BackupModel->GetItemTypeByIndex(selectedRow) != BackupModel::EIT_Filter)
			continue;

		auto oldFilter = m_BackupModel->GetFilter(selectedRow);		

		if (filterType != FilterImplementatorSelector::GetFilterTypeIndex(oldFilter.get()))
			continue;				
		
		m_BackupModel->UpdateFilter(selectedRow, newFilter);		
	}
}




void BackupEditor::on_SrcDescTable_customContextMenuRequested(const QPoint&)
{
	QItemSelectionModel* selectionModel = ui->SrcDescTable->selectionModel();
	if (!selectionModel->hasSelection())
		return;
	
	QModelIndexList selectedRows = selectionModel->selectedRows();

	bool srcDescSelected = false;
	bool filterSelected = false;
		
	map<type_index,bool> filterTypesSelected;

	bool depthSelected = false;

	for (auto& idx : selectedRows)
	{	
		idx = m_ProxyBackupModel->mapToSource(idx);
		switch (m_BackupModel->GetItemTypeByIndex(idx))
		{
		case BackupModel::EIT_Filter:
		{
			FilterIterator filter_it = m_BackupModel->GetFilterIteratorByIndex(idx);
			auto filterType = FilterImplementatorSelector::Instance().GetFilterTypeIndex(filter_it->get());
			filterTypesSelected[filterType] = true;
			filterSelected = true;
		}
		break;
		case BackupModel::EIT_Depth:
		{
			depthSelected = true;
		}
		break;
		case BackupModel::EIT_SrcDescPath:
		{
			const SrcPathDesc& srcPathDesc = m_BackupModel->GetSrcDescByIndex(idx);
			if (srcPathDesc.m_Include)
				srcDescSelected = true;
		}
		break;
		default:
			assert(0);
			break;
		}
	}

	QMenu* menu = new QMenu();
	connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));

	if (srcDescSelected)
	{
		menu->addAction(ui->AddWildcardDialogAction);
		menu->addAction(ui->AddRegExpDialogAction);		
		menu->addSeparator();
	}

	if (srcDescSelected || depthSelected)
	{
		menu->addAction(ui->DepthAction);		
	}

	if (filterSelected)
	{
		using namespace Filters;

		if (filterTypesSelected[typeid(WildcardsFilter)])
			menu->addAction(ui->SetWildcardDialogAction);
		if (filterTypesSelected[typeid(RegexpsFilter)])
			menu->addAction(ui->SetRegExpDialogAction);		
		menu->addSeparator();
	}

	menu->addAction(ui->RemoveSelectedAction);	
	menu->popup(QCursor::pos());			
}

void BackupEditor::on_AddWildcardDialogAction_triggered()
{	
	WildcardFilterEditor editorWindow;	
	ExecAddFilterDialog(&editorWindow, typeid(Filters::WildcardsFilter), GetSelectedInclusionSrcPaths());
}

void BackupEditor::on_AddRegExpDialogAction_triggered()
{
	RegexpFilterEditor editorWindow;	
	ExecAddFilterDialog(&editorWindow, typeid(Filters::RegexpsFilter), GetSelectedInclusionSrcPaths());
}




void BackupEditor::on_SetWildcardDialogAction_triggered()
{
	WildcardFilterEditor editorWindow;
	ExecEditFilterDialog(&editorWindow, typeid(Filters::WildcardsFilter));
}

void BackupEditor::on_SetRegExpDialogAction_triggered()
{
	RegexpFilterEditor editorWindow;
	ExecEditFilterDialog(&editorWindow, typeid(Filters::RegexpsFilter));
}


void BackupEditor::OverrideStorages(bool overrided)
{
	if (overrided)
	{
		auto& storages = m_Backup.m_Settings.m_Storages;
		ui->StorageWidget->SetStorages(&storages);
	}
	else
	{
		auto& mbcFileStorages = m_MyBCopyFile->m_Settings.m_Storages;
		ui->StorageWidget->SetStorages(&mbcFileStorages);
	}
}

void BackupEditor::OverrideToolchain(bool overrided)
{
	if (overrided)
	{
		auto toolchain = &m_Backup.m_Settings.m_Toolchain;
		ui->ToolChainWidget->Set(toolchain);
	}
	else
	{
		auto toolchain = &m_MyBCopyFile->m_Settings.m_Toolchain;
		ui->ToolChainWidget->Set(toolchain);
	}
}

void BackupEditor::OverrideScheme(bool overrided)
{
	if (overrided)
		ui->SchemeWidget->Set(m_Backup.m_Settings);
	else	
		ui->SchemeWidget->Set(m_MyBCopyFile->m_Settings);	
}

void BackupEditor::on_BackupEditor_finished(int)
{
	MainWindow::sm_Instance->SwitchToStartupPage();
}

bool BackupEditor::ValidateBackupName()
{	
	auto qNewName = ui->BackupNameEdit->text();
	auto newName = qNewName.toStdWString();

	if (newName.empty())
	{
		QMessageBox::critical(this, tr("Error"),tr("Backup Name must be nonempty"));
		return false;
	}

	if (!m_Backup.m_Name.empty())
	{
		if (m_Backup.m_Name == newName)
		{
			return true;
		}
	}

	auto it = findBackupByName(m_MyBCopyFile, newName);
	if (it != m_MyBCopyFile->m_Backups.end())
	{
		QMessageBox::critical(this, tr("Error"), tr("Backup '") + qNewName + tr("' already exist"));
		return false;
	}

	return true;
}
//
//BackupNameValidator::BackupNameValidator(BackupEditor *parent)
//	: QValidator(parent)	
//{
//
//}
//
//QValidator::State BackupNameValidator::validate(QString &input, int &pos) const
//{	
//	auto backupEditor = qchecked_cast<BackupEditor>(parent());
//	auto mbcFile = backupEditor->m_MyBCopyFile;
//	return mbcFile->m_Backups.end() == findBackupByName(mbcFile,input.toStdWString()) ? QValidator::State::Acceptable : QValidator::State::Invalid;
//}



QModelIndex BackupEditor::GetSelectedIndexFS()
{
	QItemSelectionModel* selectionModel = ui->FileSystemTreeView->selectionModel();
	TraversableFSModel* traversableFSModel = qchecked_cast<TraversableFSModel>(ui->FileSystemTreeView->model());
	QModelIndexList selectedRows = selectionModel->selectedRows();
	assert(selectedRows.size() == 1);
	return selectedRows.front();
}

void BackupEditor::ExecDepthDialog(const list<QPersistentModelIndex>& persistentSelectedSrcs)
{
	DepthEditor depthEditor;

	if (persistentSelectedSrcs.size() == 1)
	{
		QModelIndex idx = persistentSelectedSrcs.front();
		QModelIndex idxPath = idx;

		if (idx.parent().isValid())
			idxPath = idx.parent();

		SrcPathDesc& srcDesc = *m_BackupModel->GetIterator(idxPath);
		depthEditor.SetDepth(srcDesc.m_Depth);
	}

	int returnCode = depthEditor.exec();
	if (returnCode != QDialog::DialogCode::Accepted)
		return;

	auto depth = depthEditor.GetDepth();

	unordered_set<SrcPathDescIterator> srcDescIters;

	for (const QPersistentModelIndex& idx : persistentSelectedSrcs)
	{
		if (!idx.isValid())
			continue;

		QModelIndex idxPath = idx;

		if (idx.parent().isValid())
			idxPath = idx.parent();

		auto srcIter = m_BackupModel->GetIterator(idxPath);

		auto p = srcDescIters.insert(srcIter);

		if (!p.second)
			continue;

		m_BackupModel->SetDepth(idxPath, depth);
	}
}

std::list<QModelIndex> BackupEditor::GetSelectedInclusionSrcPaths()
{
	list<QModelIndex> result;
	ForEachSelectedSrc([&result, this](const QModelIndex& idx, QStandardItem* item)
	{
		QModelIndex srcDescIdx = m_ProxyBackupModel->mapToSource(idx);
		const SrcPathDesc& srcPathDesc = m_BackupModel->GetSrcDescByIndex(srcDescIdx);
		if (srcPathDesc.m_Include)
			result.push_back(srcDescIdx);
	});
	return result;
}

QModelIndex BackupEditor::GetSelectedIdxSrcInFS()
{
	QModelIndex idx = GetSelectedIndexFS();

	TraversableFSModel* traversableFSModel = qchecked_cast<TraversableFSModel>(ui->FileSystemTreeView->model());
	fs::path fsPath = traversableFSModel->filePath(idx).toStdWString();

	QModelIndex result = m_BackupModel->FindSrcDescByPath(fsPath);
	assert(result.isValid());
	return result;
}

void BackupEditor::on_FileSystemTreeView_customContextMenuRequested(const QPoint&)
{
	QItemSelectionModel* selectionModel = ui->FileSystemTreeView->selectionModel();
	if (!selectionModel->hasSelection())
		return;

	QModelIndex idx = GetSelectedIndexFS();

	QMenu* menu = new QMenu();
	connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));
	menu->addAction(ui->TraverseAction);
	menu->addSeparator();
	
	
	TraversableFSModel* traversableFSModel = qchecked_cast<TraversableFSModel>(ui->FileSystemTreeView->model());
	fs::path fsPath = traversableFSModel->filePath(idx).toStdWString();
	
	QModelIndex idxSrcPath = m_BackupModel->FindSrcDescByPath(fsPath);

	if (idxSrcPath.isValid())
	{
		const SrcPathDesc& srcPathDesc = m_BackupModel->GetSrcDescByIndex(idxSrcPath);
		if (srcPathDesc.m_Include)
		{
			menu->addAction(ui->AddWildcardDialogFSAction);
			menu->addAction(ui->AddRegExpDialogFSAction);
			menu->addSeparator();		
			menu->addAction(ui->DepthFSAction);
			menu->addSeparator();
		}
		menu->addAction(ui->RemoveSrcPathAction);
	}
	else
	{
		menu->addAction(ui->AddInclusionSrcPathAction);
		menu->addAction(ui->AddExclusionSrcPathAction);	
	}

	menu->popup(QCursor::pos());
}

void BackupEditor::on_AddInclusionSrcPathAction_triggered()
{
	QModelIndex idx = GetSelectedIndexFS();	

	TraversableFSModel* traversableFSModel = qchecked_cast<TraversableFSModel>(ui->FileSystemTreeView->model());
	fs::path fsPath = traversableFSModel->filePath(idx).toStdWString();
		
	SrcPathDesc newSrcPath;
	newSrcPath.m_Path = fsPath;
	newSrcPath.m_Include = true;
	m_BackupModel->Add(newSrcPath);
}

void BackupEditor::on_AddExclusionSrcPathAction_triggered()
{
	QModelIndex idx = GetSelectedIndexFS();

	TraversableFSModel* traversableFSModel = qchecked_cast<TraversableFSModel>(ui->FileSystemTreeView->model());
	fs::path fsPath = traversableFSModel->filePath(idx).toStdWString();	

	SrcPathDesc newSrcPath;
	newSrcPath.m_Path = fsPath;
	newSrcPath.m_Include = false;
	m_BackupModel->Add(newSrcPath);
}

void BackupEditor::on_RemoveSrcPathAction_triggered()
{
	QModelIndex idx = GetSelectedIndexFS();

	TraversableFSModel* traversableFSModel = qchecked_cast<TraversableFSModel>(ui->FileSystemTreeView->model());
	fs::path fsPath = traversableFSModel->filePath(idx).toStdWString();

	QModelIndex idxSrcPath = m_BackupModel->FindSrcDescByPath(fsPath);
	m_BackupModel->removeRow(idxSrcPath.row());
}

void BackupEditor::on_TraverseAction_triggered()
{
	TraversableFSModel* traversableFSModel = qchecked_cast<TraversableFSModel>(ui->FileSystemTreeView->model());	
	traversableFSModel->InvokeTraverse();

	ui->SrcDescTable->setDisabled(true);
	ui->FileSystemTreeView->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
}

void BackupEditor::on_AddWildcardDialogFSAction_triggered()
{
	WildcardFilterEditor editorWindow;
	ExecAddFilterDialog(&editorWindow, typeid(Filters::WildcardsFilter), { GetSelectedIdxSrcInFS() });
}

void BackupEditor::on_AddRegExpDialogFSAction_triggered()
{
	RegexpFilterEditor editorWindow;
	ExecAddFilterDialog(&editorWindow, typeid(Filters::RegexpsFilter), { GetSelectedIdxSrcInFS() });
}

void BackupEditor::on_DepthFSAction_triggered()
{
	//QModelIndex idx = GetSelectedIndexFS();

	//TraversableFSModel* traversableFSModel = qchecked_cast<TraversableFSModel>(ui->FileSystemTreeView->model());
	//fs::path fsPath = traversableFSModel->fileName(idx).toStdWString();

	//QModelIndex idxSrcPath = m_BackupModel->FindSrcDescByPath(fsPath);
	//list<QPersistentModelIndex> indexes;
	//indexes.push_back(idxSrcPath);
	ExecDepthDialog({ GetSelectedIdxSrcInFS() });
}

void BackupEditor::OnFSTraverseCompleted()
{
	ui->SrcDescTable->setDisabled(false);
	ui->FileSystemTreeView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
}
