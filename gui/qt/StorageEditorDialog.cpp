#include "StorageEditorDialog.h"
#include "ui_StorageEditorDialog.h"
#include "StorageFrame.h"

StorageEditorDialog::StorageEditorDialog(QAbstractItemModel* model)
	: QDialog(nullptr, Qt::Dialog | Qt::WindowCloseButtonHint)
    , ui(new Ui::StorageEditorDialog)
	, m_Model(qchecked_cast<StorageListModel>(model))
{
    ui->setupUi(this);
	connect(ui->comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(ValidateOk()));
	connect(ui->StorageNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(ValidateOk()));
	connect(ui->AddressLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(ValidateOk()));
	connect(ui->UsernameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(ValidateOk()));
	connect(ui->PasswordLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(ValidateOk()));
	connect(ui->PathLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(ValidateOk()));	
}

StorageEditorDialog::~StorageEditorDialog()
{
    delete ui;
}

std::shared_ptr<Storage> StorageEditorDialog::GetStorage() const
{
	if (ui->comboBox->currentIndex() == 0)
	{
		auto hlStorage = make_shared<WebDavStorage>();
		hlStorage->m_Name = ui->StorageNameLineEdit->text().toStdWString();
		hlStorage->m_UserName = ui->UsernameLineEdit->text().toStdWString();
		hlStorage->m_Password = ui->PasswordLineEdit->text().toStdWString();
		hlStorage->m_Url = ui->AddressLineEdit->text().toStdWString();
		hlStorage->m_BackupDirName = ui->WebDavDirectoryEdit->text().toStdWString();
		return hlStorage;
	}
	else
	{
		auto hlStorage = make_shared<LocalDiskStorage>();
		hlStorage->m_Name = ui->StorageNameLineEdit->text().toStdWString();
		hlStorage->m_Path = ui->PathLineEdit->text().toStdWString();
		return hlStorage;
	}
}

 void StorageEditorDialog::SetStorage(const std::shared_ptr<Storage>& _storage)
{
	 const Storage* storage = _storage.get();
	 
	 type_index storageType = StorageImplementatorSelector::GetStorageTypeIndex(storage);

	if (storageType == typeid(WebDavStorage))
	{		
		ui->comboBox->setCurrentIndex(0);
		auto hlStorage = static_cast<const WebDavStorage*>(storage);
		ui->StorageNameLineEdit->setText( QString::fromStdWString(hlStorage->m_Name) );
		ui->UsernameLineEdit->setText(QString::fromStdWString(hlStorage->m_UserName));
		ui->PasswordLineEdit->setText(QString::fromStdWString(hlStorage->m_Password));
		ui->AddressLineEdit->setText(QString::fromStdWString(hlStorage->m_Url));
		ui->WebDavDirectoryEdit->setText(QString::fromStdWString(hlStorage->m_BackupDirName));
	}
	else
	{
		ui->comboBox->setCurrentIndex(1);
		auto hlStorage = static_cast<const LocalDiskStorage*>(storage);
		ui->StorageNameLineEdit->setText(QString::fromStdWString(hlStorage->m_Name));
		ui->PathLineEdit->setText(QString::fromStdWString(hlStorage->m_Path));
	}
}
/*
 void StorageEditorDialog::SetDefaults()
 {
	 ui->comboBox->setCurrentIndex(0);
	 
	 ui->StorageNameLineEdit->clear();
	 ui->UsernameLineEdit->clear();
	 ui->PasswordLineEdit->clear();
	 ui->AddressLineEdit->clear();
	 	
	ui->StorageNameLineEdit->clear();
	ui->PathLineEdit->clear();

	m_Index = QModelIndex();
 }
*/

void StorageEditorDialog::SetIndex(const QModelIndex& val)
{
	m_Index = val;
	SetStorage(*m_Model->GetIterator(val.row()));
}

void StorageEditorDialog::on_StorageEditorDialog_accepted()
{	
	m_Model->AddOrSet(m_Index, GetStorage());	
}




void StorageEditorDialog::ValidateOk()
{
	bool valid = true;
	
	 valid &= !ui->StorageNameLineEdit->text().isEmpty();

	if (ui->comboBox->currentIndex() == 0)
	{
		valid &= !ui->AddressLineEdit->text().isEmpty();
		valid &= !ui->UsernameLineEdit->text().isEmpty();
		valid &= !ui->PasswordLineEdit->text().isEmpty();
		
		ui->TestWebDavButton->setEnabled(valid);
	}
	else
	{
		valid &= !ui->PathLineEdit->text().isEmpty();
	}

	ui->buttonBox->setEnabled(valid);	
}

void StorageEditorDialog::on_BrowseButton_clicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Specify directory for backup storage"));
	if (!directory.isEmpty())
		ui->PathLineEdit->setText(directory);
}

void StorageEditorDialog::on_TestWebDavButton_clicked()
{
	wstring userName = ui->UsernameLineEdit->text().toStdWString();
	wstring password = ui->PasswordLineEdit->text().toStdWString();
	wstring address = ui->AddressLineEdit->text().toStdWString();

	bool success = false;

	try
	{
		WebDavMountManager& mountManager = WebDavMountManager::Instance();
		mountManager.Mount(address, userName, password);		
		mountManager.EraseUnused();
		success = true;
	}
	catch (BaseException&)
	{

	}

	if (success)
		QMessageBox::information(this, tr("Connection Test"), tr("Successfully"));	
	else
		QMessageBox::critical(this, tr("Connection Test"), tr("Unsuccessfully"));	
}
