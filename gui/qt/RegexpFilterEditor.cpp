#include "qtgui_pch.h"
#include "RegexpFilterEditor.h"
#include "ui_RegexpFilterEditor.h"

using namespace Filters;

RegexpFilterEditor::RegexpFilterEditor(QWidget *parent)
	: FilterEditor(parent)
	, ui(new Ui::RegexpFilterEditor)
{
    ui->setupUi(this);
}

RegexpFilterEditor::~RegexpFilterEditor()
{
    delete ui;
}

std::shared_ptr<Filters::FilterBase> RegexpFilterEditor::GetFilter() const
{
	QString regexps = ui->lineEdit->text();
	auto result = make_shared<RegexpsFilter>(regexps.toStdWString());
	result->SetInclusion(GetComboBox_Inclusion(ui->InclusionComboBox));
	result->SetEntryType(GetComboBox_EntryType(ui->EntryTypeComboBox));
	return result;
}

void RegexpFilterEditor::SetFilter(const Filters::FilterBase* val)
{
	auto filter = static_cast<const RegexpsFilter*>(val);
	QString wildcards = QString::fromStdWString(filter->GetRegexsAsString());
	ui->lineEdit->setText(wildcards);
	SetComboBox_EntryType(ui->EntryTypeComboBox, val->GetEntryType());
	SetComboBox_Inclusion(ui->InclusionComboBox, val->IsInclusion());
	ValidateForm();
}



void RegexpFilterEditor::ValidateForm()
{	
	bool correct = false;
	QString regexps = ui->lineEdit->text();

	if (!regexps.isEmpty())
	{
		try
		{
			auto filter = make_shared<RegexpsFilter>(regexps.toStdWString());
			correct = true;
		}
		catch (regex_error&)
		{

		}
	}

	auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(correct);
}

QString RegexpsFilterImplementator::GetSpecificText(Filters::FilterBase* filter) const
{
	RegexpsFilter* regexpsFilter = hl_cast(filter);
	QString regexLine = QString::fromStdWString(regexpsFilter->GetRegexsAsString());
	return QObject::tr("by regular expressions: ") + " " + regexLine;
}

void RegexpsFilterImplementator::SerializeToXML(pugi::xml_node& xmlFilters, Filters::FilterBase* filter) const
{
	auto xmlFilter = xmlFilters.append_child(L"regexps");
	xmlFilter.append_attribute(L"ignore_case").set_value(L"yes");
	xmlFilter.text().set(hl_cast(filter)->GetRegexsAsString().c_str());
}

std::unique_ptr<FilterEditor> RegexpsFilterImplementator::MakeEditorDialog() const
{
	return make_unique<RegexpFilterEditor>();
}

const RegexpsFilterImplementator& g_RegexpsFilterImplementator = RegexpsFilterImplementator::Instance();