#include "SchemeFrame.h"
#include "ui_SchemeFrame.h"

SchemeFrame::SchemeFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SchemeFrame)
{
    ui->setupUi(this);

	error_code ec;
	m_DefaultTempPath = QString::fromStdWString(fs::temp_directory_path(ec));

	
	ui->DefaultTemporaryPathAction->setEnabled(!m_DefaultTempPath.isEmpty());
	if (!m_DefaultTempPath.isEmpty())
		ui->TemporaryDirEdit->setPlaceholderText(tr("(Use default)"));

	QMenu* tpMenu = new QMenu(this);
	tpMenu->addAction(ui->DefaultTemporaryPathAction);
	ui->BrowseTemporaryDirButton->setMenu(tpMenu);	
}

SchemeFrame::~SchemeFrame()
{
    delete ui;
}

void SchemeFrame::Set(const Settings& settings)
{
	//set defaults
	SetDefaults();
	
	//set settings

	for (auto& storage : settings.m_Storages)
		ui->ReferenceStorageComboBox->addItem(QString::fromStdWString(storage->m_Name));

	ui->TemporaryDirEdit->setText(QString::fromStdWString(settings.m_TemporaryDir));

	
	if (settings.m_SchemeParams)
	{

		RegularScheme* regularScheme = settings.m_SchemeParams.get();

		IncrementalScheme* incrementalScheme = dynamic_cast<IncrementalScheme*>(regularScheme);
		if (incrementalScheme)
		{
			ui->IncrementalRadio->setChecked(true);			
			ui->LengthChain->setValue(incrementalScheme->m_LengthIncrementChain);

			if (!incrementalScheme->m_ReferenceStorageName.empty())
			{
				auto& storages = settings.m_Storages;

				auto it = settings.FindStorageByName(incrementalScheme->m_ReferenceStorageName);

				if (it != storages.end())
				{
					auto index = distance(storages.begin(), it);
					ui->ReferenceStorageComboBox->setCurrentIndex(index);
				}
			}
	/*		else
			{
				if (ui->ReferenceStorageComboBox->count() > 0)
					ui->ReferenceStorageComboBox->setCurrentIndex(0);
			}*/
		}

		ui->NumStored->setValue(regularScheme->m_NumStoredOldRegularBackups);
	}

}

void SchemeFrame::GetResult(Settings& settings)
{
	shared_ptr<RegularScheme> regularScheme;
	if (ui->IncrementalRadio->isChecked())
	{
		shared_ptr<IncrementalScheme> incrementalScheme = make_shared<IncrementalScheme>();

		incrementalScheme->m_NumIncrementBranches = 1;
		incrementalScheme->m_LengthIncrementChain = ui->LengthChain->value();

		if (ui->ReferenceStorageComboBox->count() > 0)
			incrementalScheme->m_ReferenceStorageName = ui->ReferenceStorageComboBox->currentText().toStdWString();

		regularScheme = static_pointer_cast<RegularScheme>(incrementalScheme);
	}
	else
	{
		regularScheme = make_shared<RegularScheme>();
	}

	regularScheme->m_NumStoredOldRegularBackups = ui->NumStored->value();
	settings.m_SchemeParams = regularScheme;	
	settings.m_TemporaryDir = ui->TemporaryDirEdit->text().toStdWString();
}

void SchemeFrame::on_DefaultTemporaryPathAction_triggered()
{	
	ui->TemporaryDirEdit->clear();	
}

void SchemeFrame::on_BrowseTemporaryDirButton_clicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Specify directory for temporary files emplacement"));
	if (directory.isEmpty())
		return;
		
	ui->TemporaryDirEdit->setText(directory);	
}

void SchemeFrame::SetDefaults()
{
	ui->ReferenceStorageComboBox->clear();
	
	ui->RegularRadio->setChecked(true);		
	ui->LengthChain->setValue(3);
	ui->ReferenceStorageComboBox->setCurrentIndex(0);
	ui->NumStored->setValue(5);	
	ui->TemporaryDirEdit->clear();
}

