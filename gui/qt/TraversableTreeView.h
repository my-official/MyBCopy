#pragma once

class TraversableDelegate;

class TraversableTreeView :	public QTreeView
{
	Q_OBJECT
public:
	explicit TraversableTreeView(QWidget *parent = Q_NULLPTR);
	//~TraversableTreeView();

	static const int msc_AnimationInterval;
protected:	
	virtual void timerEvent(QTimerEvent * event) override;
private:
	QPixmap m_IndeterminateCheckStateImage;	
	int m_TimerId;
	TraversableDelegate* m_TraversableDelegate;

	
};


class TraversableDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	explicit TraversableDelegate(QObject *parent = Q_NULLPTR);
	//~TraversableDelegate();
	void SwitchToNextFrame();
protected:
	virtual void drawCheck(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, Qt::CheckState state) const override;
private:
	qreal m_Degree;
	qreal m_DeltaDegPerFrame;
	QPixmap m_IndeterminatePixmap;
};