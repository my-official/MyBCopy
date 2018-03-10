#pragma once
#include "BackupModel.h"


namespace Ui {
class StorageFrame;
}

class StorageFrame : public QWidget
{
    Q_OBJECT

public:
    explicit StorageFrame(QWidget *parent = 0);
    ~StorageFrame();
	
	void SetStorages(list< shared_ptr<Storage> >* storages);
private slots:
    void on_NewStorageButton_clicked();

    void on_SetAction_triggered();

    void on_RemoveAction_triggered();

    void on_StoragesTableView_doubleClicked(const QModelIndex &index);

    void on_StoragesTableView_customContextMenuRequested(const QPoint &pos);

private:
    Ui::StorageFrame *ui;
};




Q_DECLARE_METATYPE(ListModel< shared_ptr<Storage> >::iterator)

class StorageListModel : public ListModel< shared_ptr<Storage> >
{
	Q_OBJECT
public:
	explicit StorageListModel(QObject*parent = Q_NULLPTR);
protected:
	virtual void ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& srcPathIter) override;
};


