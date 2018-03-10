#pragma once
#include "BackupModel.h"

namespace Ui {
class ToolChainFrame;
}

class ToolChainFrame : public QWidget
{
    Q_OBJECT

public:
    explicit ToolChainFrame(QWidget *parent = 0);
    ~ToolChainFrame();

	void Set(list< shared_ptr<Tool> >* toolchain);
private slots:
	void on_NewButton_clicked();

	void on_SetAction_triggered();

	void on_RemoveAction_triggered();

	void on_TableView_doubleClicked(const QModelIndex &idx);

	void on_TableView_customContextMenuRequested(const QPoint &pos);
private:
    Ui::ToolChainFrame *ui;	
};





Q_DECLARE_METATYPE(ListModel< shared_ptr<Tool> >::iterator)

class ToolListModel : public ListModel< shared_ptr<Tool> >
{
	Q_OBJECT
public:
	explicit ToolListModel(QObject*parent = Q_NULLPTR);
protected:
	virtual void ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& srcPathIter) override;
};

