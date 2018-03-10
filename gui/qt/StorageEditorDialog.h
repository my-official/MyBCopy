#pragma once


namespace Ui {
class StorageEditorDialog;
}

class StorageListModel;

class StorageEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StorageEditorDialog(QAbstractItemModel* model);
    ~StorageEditorDialog();
		
	void SetIndex(const QModelIndex& val);
private slots:
    void on_StorageEditorDialog_accepted();    

    
	void ValidateOk();
    void on_BrowseButton_clicked();

    void on_TestWebDavButton_clicked();

private:
    Ui::StorageEditorDialog *ui;
	StorageListModel* m_Model;
	QModelIndex m_Index;

	shared_ptr<Storage> GetStorage() const;
	void SetStorage(const std::shared_ptr<Storage>& _storage);
	
};
