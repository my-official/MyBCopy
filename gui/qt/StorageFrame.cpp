#include "qtgui_pch.h"
#include "StorageFrame.h"
#include "ui_StorageFrame.h"
#include "BackupModel.h"
#include "StorageEditorDialog.h"

StorageFrame::StorageFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StorageFrame)
{
    ui->setupUi(this);

	
}

StorageFrame::~StorageFrame()
{
    delete ui;
}

void StorageFrame::SetStorages(list< shared_ptr<Storage> >* storages)
{
	auto prevModel = ui->StoragesTableView->model();
	
	StorageListModel* storageListModel = new StorageListModel(ui->StoragesTableView);
	storageListModel->SetSourceList(storages);
	ui->StoragesTableView->setModel(storageListModel);
	
	SafeDelete(prevModel);
}

void StorageFrame::on_NewStorageButton_clicked()
{
    StorageEditorDialog editor(ui->StoragesTableView->model());
    editor.exec();
}

void StorageFrame::on_SetAction_triggered()
{
	auto selectedRows = ui->StoragesTableView->selectionModel()->selectedRows();
	assert(selectedRows.size() == 1);
	on_StoragesTableView_doubleClicked(selectedRows.front());
}

void StorageFrame::on_RemoveAction_triggered()
{
	RemoveSelectedRowsFromModel(ui->StoragesTableView);
}

void StorageFrame::on_StoragesTableView_doubleClicked(const QModelIndex &idx)
{
	StorageEditorDialog editor(ui->StoragesTableView->model());
	editor.SetIndex(idx.sibling(idx.row(), 0));
	editor.exec();
}

void StorageFrame::on_StoragesTableView_customContextMenuRequested(const QPoint &pos)
{
	QMenu* menu = new QMenu();
	connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));

	auto selectedRows = ui->StoragesTableView->selectionModel()->selectedRows();

	if (selectedRows.empty())
		return;

	if (selectedRows.size() == 1)
	{
		menu->addAction(ui->SetAction);
	}

	menu->addAction(ui->RemoveAction);
	menu->popup(QCursor::pos());
}


StorageListModel::StorageListModel(QObject*parent /*= Q_NULLPTR*/)
	: ListModel< shared_ptr<Storage> >(parent)
{
	setHorizontalHeaderLabels({ tr("Name"),tr("Type"), tr("Address"), tr("Parameters") });
}


void StorageListModel::ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& srcPathIter)
{
	Storage* storage = srcPathIter->get();
	StorageImplementator* storageImplementator = StorageImplementatorSelector::Instance().GetStorageImplementator(storage);

	newRow << new QStandardItem(QString::fromStdWString(storage->m_Name))
		<< new QStandardItem(storageImplementator->GetTypeName(storage))
		<< new QStandardItem(storageImplementator->GetAddress(storage))
		<< new QStandardItem(storageImplementator->GetSpecificText(storage));
}
