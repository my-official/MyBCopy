#ifndef DEPTHEDITOR_H
#define DEPTHEDITOR_H

#include <QDialog>

namespace Ui {
class DepthEditor;
}

class DepthEditor : public QDialog
{
    Q_OBJECT
public:
    explicit DepthEditor(QWidget *parent = 0);
    ~DepthEditor();
	size_t GetDepth() const;
	void SetDepth(size_t depth);
private:
    Ui::DepthEditor *ui;
};

#endif // DEPTHEDITOR_H
