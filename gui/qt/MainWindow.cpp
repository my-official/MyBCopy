#include "qtgui_pch.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "XmlSeriallizator.h"

MainWindow* MainWindow::sm_Instance = nullptr;

void MainWindow::RecreateAndShowMainWindow()
{
	if (MainWindow::sm_Instance)
		MainWindow::sm_Instance->deleteLater();
	
	MainWindow::sm_Instance = new MainWindow();
	QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), MainWindow::sm_Instance, SLOT(deleteLater()));
	MainWindow::sm_Instance->show();
	MainWindow::sm_Instance->LoadLastFile();
}

namespace Config
{
	const QString LastOpenFile("LastOpenFile");
}

namespace
{

QTranslator* myappTranslator = nullptr;


void InstallRuTranslators()
{
	if (!myappTranslator)
	{
		myappTranslator = new QTranslator();
		QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), myappTranslator, SLOT(deleteLater()));
		myappTranslator->load("MyBCopy-qt5_ru");
	}

	QApplication& a = *qchecked_cast<QApplication>(QCoreApplication::instance());
	a.installTranslator(myappTranslator);
}

void RemoveRuTranslators()
{
	QApplication& a = *qchecked_cast<QApplication>(QCoreApplication::instance());
	a.removeTranslator(myappTranslator);
}

}


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
    , ui(new Ui::MainWindow)	
{
    ui->setupUi(this);		
	m_Model = new MainWindowBackupModel(this);
	m_Model->SetSourceList(&m_MyBCopyFile.m_Backups);
	ui->BackupTable->setModel(m_Model);
	ui->BackupTable->addAction(ui->SetAction);
	ui->BackupTable->addAction(ui->RemoveAction);
	NewFile();	
	UpdateLanguageFlagImage();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::LoadUserSpecificXml()
{
	auto xmlFile = QString::fromStdWString( Reserver::GetUserSpecificXMLPath() );
	if (Reserver::IsUserSpecificXMLExist())
	{		
		LoadFile(xmlFile);
	}
	else
	{
		SetCurrentFile(xmlFile);
	}
}

void MainWindow::LoadFile(const QString& file)
{
	//SetCurrentFile(QString());
	//m_MyBCopyFile = MyBCopyFile();

	if (!m_MyBCopyFile.LoadFromXMLFile(file.toStdWString()))
	{	
		QMessageBox::critical(this, tr("Error"), tr("Can't load file ") + file);
		return;
	}
		
	m_Model->SetSourceList(&m_MyBCopyFile.m_Backups);
	SetCurrentFile(file);
	QSettings().setValue(Config::LastOpenFile, m_CurrentFile);
}

void MainWindow::Save()
{
	QString destFile = m_CurrentFile;
	if (destFile.isEmpty())
	{		
		if (!ExecSaveDialog(destFile))
			return;
	}

	do
	{
		if (SaveMyBCopyFile(destFile.toStdWString(), m_MyBCopyFile))
		{			
			SetCurrentFile(destFile);
			QSettings().setValue(Config::LastOpenFile, m_CurrentFile);			
			return;
		}
		
		QMessageBox::critical(this, tr("Error"), tr("Can't write to file ") + destFile);	
	}
	while (ExecSaveDialog(destFile));
}

void MainWindow::SetCurrentFile(const QString& val)
{
	m_CurrentFile = val;
	if (val.isEmpty())
		setWindowTitle("New Backup List - MyBCopy");
	else
	{
		QString filename = QFileInfo(val).fileName();
		
		assert(!filename.isEmpty());
		if (filename.isEmpty())
			filename = val;

		setWindowTitle(filename + " - MyBCopy");
	}
}

void MainWindow::NewFile()
{
	SetCurrentFile("");
	m_MyBCopyFile = MyBCopyFile();
	m_Index = QModelIndex();	
	m_Model->SetSourceList(&m_MyBCopyFile.m_Backups);
}


void MainWindow::UpdateLanguageFlagImage()
{
	QString iconName;
	if (sm_English)			
		iconName = QStringLiteral(":/Custom-Icon-Design-All-Country-Flag-Russia-Flag.ico");	
	else	
		iconName = QStringLiteral(":/Custom-Icon-Design-All-Country-Flag-United-Kingdom-flag.ico");	

	QIcon icon2;
	icon2.addFile(iconName, QSize(), QIcon::Normal, QIcon::Off);
	ui->LanguageButton->setIcon(icon2);
}

void MainWindow::SwitchToStartupPage()
{
	ui->MainStack->setCurrentWidget(ui->StartPage);
}

void MainWindow::LoadLastFile()
{
	QSettings settings;
	QString lastOpenFile = settings.value(Config::LastOpenFile).toString();
	if (lastOpenFile.isEmpty())
	{		 
		LoadUserSpecificXml();
		return;
	}

				
	if (!QFile::exists(lastOpenFile))
	{
		NewFile();
		return;
	}

	LoadFile(lastOpenFile);		
}

void MainWindow::on_BackupTable_doubleClicked(const QModelIndex& idx)
{	
	ParsedSources& backup = *m_Model->GetIterator(idx.row());
	ui->EditBackupPage->SetBackup(&m_MyBCopyFile, backup);
	m_Index = idx;
	ui->MainStack->setCurrentWidget(ui->EditBackupPage);
}

void MainWindow::on_SettingsButton_clicked()
{
	ui->SettingsPage->Set(m_MyBCopyFile.m_Settings);
	ui->MainStack->setCurrentWidget(ui->SettingsPage);
}

void MainWindow::on_NewBackupButton_clicked()
{	
	m_Index = QModelIndex();
	ui->EditBackupPage->SetNewBackup(&m_MyBCopyFile);
	ui->MainStack->setCurrentWidget(ui->EditBackupPage);
}



void MainWindow::on_LoadButton_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, QString(), QString(), tr("MyBCopy Backup List (*.mybcopy *.xml);;All files (*.*)"));
	if (file.isEmpty())
		return;
		
	LoadFile(file);
}

void MainWindow::on_BackupButton_clicked()
{
	ui->ProgressPage->SetInputFile(m_MyBCopyFile);
    ui->MainStack->setCurrentWidget(ui->ProgressPage);
}

void MainWindow::on_SettingsPage_accepted()
{
	m_MyBCopyFile.m_Settings = ui->SettingsPage->GetResult();
	Save();	
}


void MainWindow::on_EditBackupPage_accepted()
{
	m_Model->AddOrSet(m_Index, ui->EditBackupPage->GetResult());
	Save();	
}

bool MainWindow::sm_English = true;

bool MainWindow::ExecSaveDialog(QString& destFile)
{
	destFile = QFileDialog::getSaveFileName(this, QString(), QString(), tr("MyBCopy Backup List (*.mybcopy)"));
	return !destFile.isEmpty();
}

void MainWindow::on_RemoveAction_triggered()
{
	RemoveSelectedRowsFromModel(ui->BackupTable);
	Save();
}

void MainWindow::on_SetAction_triggered()
{
	auto selectedRows = ui->BackupTable->selectionModel()->selectedRows();
	assert(selectedRows.size() == 1);
	on_BackupTable_doubleClicked(selectedRows.front());	
}

void MainWindow::on_AboutButton_clicked()
{
	QMessageBox::information(this, tr("About"),	tr("MyBCopy - backup tool.  Version 2.0\n\nThanks to http://www.iconarchive.com"));
}

void MainWindow::on_LanguageButton_clicked()
{
	sm_English = !sm_English;
	if (sm_English)
	{
		RemoveRuTranslators();		
	}
	else
	{
		InstallRuTranslators();		
	}

	RecreateAndShowMainWindow();	
}

