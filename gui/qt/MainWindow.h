#pragma once

#include "Utils_qtgui.h"
#include "RegexpFilterEditor.h"
#include "WildcardFilterEditor.h"
#include "DepthEditor.h"
#include "StorageEditorDialog.h"
#include "BackupModel.h"

namespace Ui {
class MainWindow;
}


class MainWindowBackupModel;
//#include <QWizard>



class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    explicit MainWindow(QWidget *parent = 0);
public:
	~MainWindow();

	static void RecreateAndShowMainWindow();

	void LoadLastFile();
	void SwitchToStartupPage();
	
	static MainWindow* sm_Instance;

	
private slots:
	void on_BackupTable_doubleClicked(const QModelIndex& idx);
    void on_SettingsButton_clicked();

    void on_NewBackupButton_clicked();    
	
    void on_LoadButton_clicked();

    void on_BackupButton_clicked();

	void on_SettingsPage_accepted();	
    void on_EditBackupPage_accepted();    

    void on_RemoveAction_triggered();

    void on_SetAction_triggered();

    void on_AboutButton_clicked();

    void on_LanguageButton_clicked();

private:
    Ui::MainWindow* ui;
	MainWindowBackupModel*	m_Model;
	QModelIndex				m_Index;

	QString			m_CurrentFile;
	MyBCopyFile		m_MyBCopyFile;	

	
	static bool sm_English;//TODO: better logic
	

	bool ExecSaveDialog(QString& destFile);	
	void LoadUserSpecificXml();

	void LoadFile(const QString& file);
	void Save();
	

	void SetCurrentFile(const QString& val);
	void NewFile();
	void UpdateLanguageFlagImage();
};










class MainWindowBackupModel : public ListModel< ParsedSources >
{
	Q_OBJECT
public:
	explicit MainWindowBackupModel(QObject*parent = Q_NULLPTR)
		: ListModel< ParsedSources >(parent)
	{
		setHorizontalHeaderLabels({ tr("Name"), tr("Comment") });
	}
protected:
	virtual void ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& it) override
	{
		newRow << new QStandardItem(QString::fromStdWString(it->m_Name))
			<< new QStandardItem(QString::fromStdWString(it->m_Comment));
	}
};
