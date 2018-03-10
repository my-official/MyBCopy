#pragma once

namespace Ui {
class ToolEditorDialog;
}
class ToolListModel;

class ToolEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ToolEditorDialog(ToolListModel *model);
    ~ToolEditorDialog();

	void SetIndex(const QModelIndex& val);	
	bool CheckPasswords();
public slots:
	void UpdatePassRetryEdits();
private slots:
	void on_ShowPasswordCheckBox_toggled(bool checked);
	
	void on_BrowsePathButton_clicked();
	void on_SetSystemAction_triggered();

    void on_TypeComboBox_currentIndexChanged(int);
    
    void on_ToolEditorDialog_accepted();

private:
    Ui::ToolEditorDialog *ui;
	ToolListModel* m_Model;
	QModelIndex m_Index;
	
	void Set(const std::shared_ptr<Tool>& pTool);	
	std::shared_ptr<Tool> GetResult() const;
};

