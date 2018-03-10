#pragma once

#include "FilterEditor.h"
#include "Utils_qtgui.h"

namespace Ui {
class RegexpFilterEditor;
}

class RegexpFilterEditor : public FilterEditor
{
    Q_OBJECT

public:
    explicit RegexpFilterEditor(QWidget *parent = 0);
    ~RegexpFilterEditor();

	virtual std::shared_ptr<Filters::FilterBase> GetFilter() const override;
	virtual void SetFilter(const Filters::FilterBase* val) override;
public slots:
	void ValidateForm();
private:
    Ui::RegexpFilterEditor *ui;
};


class RegexpsFilterImplementator : public FilterImplementatorBase<RegexpsFilterImplementator, Filters::RegexpsFilter>
{
public:
	using FilterImplementatorBase<RegexpsFilterImplementator, Filters::RegexpsFilter>::FilterImplementatorBase;
	virtual QString GetSpecificText(Filters::FilterBase* filter) const override;
	virtual void SerializeToXML(pugi::xml_node& xmlFilters, Filters::FilterBase* filter) const override;
	virtual unique_ptr<FilterEditor> MakeEditorDialog() const  override;
};


