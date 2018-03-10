#include "DepthEditor.h"
#include "ui_DepthEditor.h"

DepthEditor::DepthEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DepthEditor)
{
    ui->setupUi(this);
}

DepthEditor::~DepthEditor()
{
    delete ui;
}

size_t DepthEditor::GetDepth() const
{
	int value = ui->spinBox->value();
	assert(value >= -1);
	if (value != -1)
		return value;
	else
		return SrcPathDesc::Depthless;
}

void DepthEditor::SetDepth(size_t depth)
{
	if (depth != SrcPathDesc::Depthless)
		ui->spinBox->setValue(depth);	
	else
		ui->spinBox->setValue(-1);
}
