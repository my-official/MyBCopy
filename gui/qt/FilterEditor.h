#pragma once


class FilterEditor : public QDialog
{
	Q_OBJECT
public:
	explicit FilterEditor(QWidget *parent = Q_NULLPTR);

	virtual std::shared_ptr<Filters::FilterBase> GetFilter() const = 0;
	virtual void SetFilter(const Filters::FilterBase* val) = 0;	
};