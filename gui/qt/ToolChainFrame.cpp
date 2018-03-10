#include "ToolChainFrame.h"
#include "ui_ToolChainFrame.h"
#include "ToolEditorDialog.h"
#include "Utils_qtgui.h"
#include "BackupModel.h"

ToolChainFrame::ToolChainFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolChainFrame)
{
    ui->setupUi(this);
	
}

ToolChainFrame::~ToolChainFrame()
{
    delete ui;
}

void ToolChainFrame::Set(list< shared_ptr<Tool> >* toolchain)
{
	auto prevModel = ui->TableView->model();

	ToolListModel* newModel = new ToolListModel(ui->TableView);
	newModel->SetSourceList(toolchain);
	ui->TableView->setModel(newModel);

	SafeDelete(prevModel);
}

void ToolChainFrame::on_NewButton_clicked()
{
	ToolEditorDialog editor(qchecked_cast<ToolListModel>(ui->TableView->model()));
	editor.exec();
}

void ToolChainFrame::on_SetAction_triggered()
{
	auto selectedRows = ui->TableView->selectionModel()->selectedRows();
	assert(selectedRows.size() == 1);
	on_TableView_doubleClicked(selectedRows.front());
}

void ToolChainFrame::on_RemoveAction_triggered()
{
	RemoveSelectedRowsFromModel(ui->TableView);
}

void ToolChainFrame::on_TableView_doubleClicked(const QModelIndex &idx)
{
	ToolEditorDialog editor(qchecked_cast<ToolListModel>(ui->TableView->model()));
	editor.SetIndex(idx.sibling(idx.row(), 0));
	editor.exec();
}

void ToolChainFrame::on_TableView_customContextMenuRequested(const QPoint &pos)
{
	QMenu* menu = new QMenu();
	connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));

	auto selectedRows = ui->TableView->selectionModel()->selectedRows();

	if (selectedRows.empty())
		return;

	if (selectedRows.size() == 1)
	{
		menu->addAction(ui->SetAction);
	}

	menu->addAction(ui->RemoveAction);
	menu->popup(QCursor::pos());
}



ToolListModel::ToolListModel(QObject*parent /*= Q_NULLPTR*/)
	: ListModel< shared_ptr<Tool> >(parent)
{
	setHorizontalHeaderLabels({ tr("Type"), tr("Path") });
}

void ToolListModel::ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& srcPathIter)
{
	Tool* tool = srcPathIter->get();
	ToolImplementator* toolImplementator = ToolImplementatorSelector::Instance().GetToolImplementator(tool);

	QString exePath = QString::fromStdWString(tool->m_ExecutablePath);
	if (exePath.isEmpty())
		exePath = QObject::tr("(Use system default)");

	newRow << new QStandardItem(toolImplementator->GetTypeName())
		<< new QStandardItem(exePath);
}
