#include "SettingsEditor.h"
#include "ui_SettingsEditor.h"
#include "MainWindow.h"

SettingsEditor::SettingsEditor(QWidget *parent) :
	QDialog(parent),
    ui(new Ui::SettingsEditor)
{
    ui->setupUi(this);
	QPushButton* ok = ui->buttonBox->button(QDialogButtonBox::Ok);
	assert(ok);	
}

SettingsEditor::~SettingsEditor()
{
    delete ui;
}


void SettingsEditor::Set(const Settings& setting)
{
	m_Settings = setting;
	
	ui->StorageWidget->SetStorages(&m_Settings.m_Storages);
	ui->ToolChainWidget->Set(&m_Settings.m_Toolchain);
	ui->SchemeWidget->Set(m_Settings);
}

Settings SettingsEditor::GetResult() const
{
	Settings result;

	result.m_Storages = m_Settings.m_Storages;
	result.m_Toolchain = m_Settings.m_Toolchain;	
	ui->SchemeWidget->GetResult(result);

	return result;
}

void SettingsEditor::on_LinkStoragePage_clicked()
{
	ui->stackedWidget->setCurrentWidget(ui->StoragesPage);
}


void SettingsEditor::on_LinkToolChainPage_clicked()
{
	ui->stackedWidget->setCurrentWidget(ui->ToolChainPage);
}


void SettingsEditor::on_LinkSchemePage_clicked()
{
	ui->stackedWidget->setCurrentWidget(ui->SchemePage);
}




void SettingsEditor::on_SettingsEditor_finished(int)
{
	MainWindow::sm_Instance->SwitchToStartupPage();
}