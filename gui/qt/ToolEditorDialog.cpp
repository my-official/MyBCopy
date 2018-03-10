#include "ToolEditorDialog.h"
#include "ui_ToolEditorDialog.h"
#include "Utils_qtgui.h"
#include "ToolChainFrame.h"

ToolEditorDialog::ToolEditorDialog(ToolListModel *model)
	: QDialog(nullptr, Qt::Dialog | Qt::WindowCloseButtonHint)
    , ui(new Ui::ToolEditorDialog)
	, m_Model(model)
{
    ui->setupUi(this);
		

	map<type_index, ToolImplementator*> implementatorsMap = ToolImplementatorSelector::Instance().GetMap();
	assert(!implementatorsMap.empty());

	QStringList comboBoxItems;
	for (auto& p : implementatorsMap)
	{
		comboBoxItems << p.second->GetTypeName();
	}

	ui->TypeComboBox->addItems(comboBoxItems);
	ui->TypeComboBox->setCurrentIndex(0);	
	
	QMenu* menu = new QMenu(this);	
	menu->addAction(ui->SetSystemAction);
	ui->BrowsePathButton->setMenu(menu);
}

ToolEditorDialog::~ToolEditorDialog()
{
    delete ui;
}

void ToolEditorDialog::SetIndex(const QModelIndex& val)
{
	m_Index = val;
	Set(*m_Model->GetIterator(val.row()));
}


bool ToolEditorDialog::CheckPasswords()
{
	bool empty = ui->PassEdit->text().isEmpty();
	if (ui->ShowPasswordCheckBox->isChecked())
	{
		return !empty;
	}
	else
	{
		if (empty)
			return false;

		return (ui->PassEdit->text() == ui->PassRetryEdit->text());
	}	
}

void ToolEditorDialog::UpdatePassRetryEdits()
{
	bool checked = ui->ShowPasswordCheckBox->isChecked();

	QString styleSheet;

	if (ui->PassEdit->text() != ui->PassRetryEdit->text() && !checked)
		styleSheet = "background-color: rgba(255, 0, 0, 200);";
	ui->PassRetryEdit->setStyleSheet(styleSheet);
	ui->buttonBox->setEnabled(CheckPasswords());
}

void ToolEditorDialog::on_ShowPasswordCheckBox_toggled(bool checked)
{
	QLineEdit::EchoMode echoMode = checked ? QLineEdit::EchoMode::Normal : QLineEdit::EchoMode::Password;
	ui->PassEdit->setEchoMode(echoMode);

	if (checked)
	{
		ui->PassRetryLabel->hide();
		ui->PassRetryEdit->hide();	
	}
	else
	{
		ui->PassRetryEdit->setText(ui->PassEdit->text());		

		ui->PassRetryLabel->show();
		ui->PassRetryEdit->show();		
	}

	UpdatePassRetryEdits();
}


void ToolEditorDialog::on_BrowsePathButton_clicked()
{
	ToolImplementator* toolImpl = ToolImplementatorSelector::Instance().GetToolImplementatorByTypename(ui->TypeComboBox->currentText());
	
	QString filters = toolImpl->GetFileDialogFilter() + tr(";;Executable files (*.exe);;All files (*.*)");

	QString file = QFileDialog::getOpenFileName(this, QString(), QString(), filters);
	if (file.isEmpty())
		return;
		
	ui->PathEdit->setText(file);	
}
void ToolEditorDialog::Set(const std::shared_ptr<Tool>& pTool)
{
	Tool* tool = pTool.get();
	auto toolImplementator = ToolImplementatorSelector::Instance().GetToolImplementator(tool);
	QString toolTypeName = toolImplementator->GetTypeName();
	
	auto model = ui->TypeComboBox->model();
	ui->TypeComboBox->setCurrentText(toolTypeName);
		
	
	ui->PathEdit->clear();
	if (!tool->m_ExecutablePath.empty())
	{
		ui->PathEdit->setText(QString::fromStdWString(tool->m_ExecutablePath));
	}	

	ui->PassEdit->setText(QString::fromStdWString(tool->m_Password));
	ui->PassRetryEdit->setText(ui->PassEdit->text());	

	on_ShowPasswordCheckBox_toggled(ui->ShowPasswordCheckBox->checkState());
}


std::shared_ptr<Tool> ToolEditorDialog::GetResult() const
{
	auto toolImpl = ToolImplementatorSelector::Instance().GetToolImplementatorByTypename(ui->TypeComboBox->currentText());
	return toolImpl->MakeTool(ui->PathEdit->text(), ui->PassEdit->text());
}

void ToolEditorDialog::on_TypeComboBox_currentIndexChanged(int)
{	
	ToolImplementator* toolImpl = ToolImplementatorSelector::Instance().GetToolImplementatorByTypename(ui->TypeComboBox->currentText());
	QString systemExePath = toolImpl->GetDefaultExecutablePath();
		
	ui->SetSystemAction->setEnabled(!systemExePath.isEmpty());	
	ui->PathEdit->setPlaceholderText(!systemExePath.isEmpty() ? tr("(Use system default)") : "");
}

void ToolEditorDialog::on_SetSystemAction_triggered()
{
	ui->PathEdit->clear();
}

void ToolEditorDialog::on_ToolEditorDialog_accepted()
{
	m_Model->AddOrSet(m_Index, GetResult());
}
