#include "qtgui_pch.h"
#include "WildcardFilterEditor.h"
#include "ui_WildcardFilterEditor.h"


using namespace Filters;


WildcardFilterEditor::WildcardFilterEditor(QWidget *parent)
	: FilterEditor(parent)
    , ui(new Ui::WildcardFilterEditor)
{
    ui->setupUi(this);
}

WildcardFilterEditor::~WildcardFilterEditor()
{
    delete ui;
}

std::shared_ptr<Filters::FilterBase> WildcardFilterEditor::GetFilter() const
{
	QString wildcards = ui->lineEdit->text();
	return make_shared<WildcardsFilter>(wildcards.toStdWString());
}

void WildcardFilterEditor::SetFilter(const Filters::FilterBase* val)
{
	auto filter = static_cast<const WildcardsFilter*>(val);
	QString wildcards = QString::fromStdWString( filter->GetWildcardsAsString() );
	ui->lineEdit->setText(wildcards);
}




QString WildcardsFilterImplementator::GetSpecificText(Filters::FilterBase* filter) const
{
	WildcardsFilter* wildcardsFilter = hl_cast(filter);
	QString wildcardLine = QString::fromStdWString(wildcardsFilter->GetWildcardsAsString().c_str());
	return QObject::tr("by wildcards: ") + " " + wildcardLine;
}

void WildcardsFilterImplementator::SerializeToXML(pugi::xml_node& xmlFilters, Filters::FilterBase* filter) const
{
	auto xmlFilter = xmlFilters.append_child(L"wildcards");
	xmlFilter.append_attribute(L"ignore_case").set_value(L"yes");
	xmlFilter.text().set(hl_cast(filter)->GetWildcardsAsString().c_str());
}

std::unique_ptr<FilterEditor> WildcardsFilterImplementator::MakeEditorDialog() const
{
	return make_unique<WildcardFilterEditor>();
}

const WildcardsFilterImplementator& g_WildcardsFilterImplementator = WildcardsFilterImplementator::Instance();