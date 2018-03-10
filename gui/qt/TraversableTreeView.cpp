#include "qtgui_pch.h"
#include "TraversableTreeView.h"
#include "TraversableFSModel.h"




TraversableTreeView::TraversableTreeView(QWidget *parent /*= Q_NULLPTR*/)
	: QTreeView(parent)
	, m_TimerId(startTimer(msc_AnimationInterval))
	, m_TraversableDelegate(new TraversableDelegate(this))
{	
	setItemDelegateForColumn(0, m_TraversableDelegate);
}

void TraversableTreeView::timerEvent(QTimerEvent * event)
{
	if (m_TimerId != event->timerId())
		return;
	
	m_TraversableDelegate->SwitchToNextFrame();
	viewport()->update();
}

const int TraversableTreeView::msc_AnimationInterval = 50;

//TraversableTreeView::~TraversableTreeView()
//{
//}




TraversableDelegate::TraversableDelegate(QObject *parent /*= Q_NULLPTR*/)
	: QItemDelegate(parent)
	, m_Degree(0)
	, m_DeltaDegPerFrame(qreal(360) / qreal(2000 / TraversableTreeView::msc_AnimationInterval))
	, m_IndeterminatePixmap(":/Refresh-icon.png")
{
}

void TraversableDelegate::SwitchToNextFrame()
{
	m_Degree += m_DeltaDegPerFrame;

	if (m_Degree >= 360.f)
		m_Degree = 0;
}

void TraversableDelegate::drawCheck(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, Qt::CheckState state) const
{
	if (state != IndeterminateCheckState)
	{
		QItemDelegate::drawCheck(painter, option, rect, state);		
	}
	else
	{
		painter->save();
		painter->translate(rect.center());
		painter->rotate(m_Degree);
		int x = rect.width() / -2;
		int y = rect.height() / -2;
		painter->drawPixmap(x, y, rect.width(), rect.height(), m_IndeterminatePixmap);
		painter->restore();
	}
}
