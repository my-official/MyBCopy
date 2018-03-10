#pragma once

#include "FilterEditor.h"
#include "Utils_qtgui.h"

namespace Ui {
class WildcardFilterEditor;
}

class WildcardFilterEditor : public FilterEditor
{
    Q_OBJECT

public:
    explicit WildcardFilterEditor(QWidget *parent = 0);
    ~WildcardFilterEditor();

	virtual std::shared_ptr<Filters::FilterBase> GetFilter() const override;
	virtual void SetFilter(const Filters::FilterBase* val) override;
private:
    Ui::WildcardFilterEditor *ui;
};




class WildcardsFilterImplementator : public FilterImplementatorBase<WildcardsFilterImplementator, Filters::WildcardsFilter>
{
public:
	using FilterImplementatorBase<WildcardsFilterImplementator, Filters::WildcardsFilter>::FilterImplementatorBase;
	virtual QString GetSpecificText(Filters::FilterBase* filter) const override;
	virtual void SerializeToXML(pugi::xml_node& xmlFilters, Filters::FilterBase* filter) const override;
	virtual unique_ptr<FilterEditor> MakeEditorDialog() const  override;
};
