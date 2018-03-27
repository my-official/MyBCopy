#include "BackupModel.h"
#include "ParsedSources.h"
#include "ModelIterator.h"





ContainerBasedModel::ContainerBasedModel(QObject*parent /*= Q_NULLPTR*/) : QStandardItemModel(parent)
{

}

void ContainerBasedModel::Emit_ContainerReset()
{
	emit ContainerReset();
}

void ContainerBasedModel::Emit_ElementAdded(QModelIndex idx)
{
	emit ElementAdded(idx);
}

void ContainerBasedModel::Emit_ElementChanged(QModelIndex idx)
{
	emit ElementChanged(idx);
}

void ContainerBasedModel::Emit_ElementBeforeRemove(int row, int count)
{
	emit ElementBeforeRemove(row, count);
}









void ContainerBasedModel::Emit_ElementAfterRemove(int row, int count)
{
	emit ElementAfterRemove(row, count);
}

BackupModel::BackupModel(QObject *parent /*= Q_NULLPTR*/)
	: ListModel< SrcPathDesc >(parent)
	, m_IncludePixmap(":/Button-Add-icon.png")
	, m_ExcludePixmap(":/Button-Delete-icon.png")
{
	assert(!m_IncludePixmap.isNull());
	assert(!m_ExcludePixmap.isNull());
	//setColumnCount(2);
	setHorizontalHeaderLabels({ tr("Path") });
}


void BackupModel::SetBackup(ParsedSources* backup)
{	
	backup->NormalizeRecursive();
	decltype(backup->m_SrcPaths) linearizedList;
	Linearize(backup->m_SrcPaths, linearizedList);
	swap(backup->m_SrcPaths, linearizedList);
	SetSourceList(&backup->m_SrcPaths);
}


void BackupModel::ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& it)
{
	QStandardItem* itemPath = new QStandardItem(QString::fromStdWString(it->m_Path));
	newRow << itemPath;
	itemPath->setData(EItemType::EIT_SrcDescPath, EBackupItemRole::ItemTypeRole);		
	itemPath->setData(GetInclusionPixmap(it->m_Include), Qt::DecorationRole);

	if (it->m_Depth != SrcPathDesc::Depthless)
	{
		AppendDepthItem(itemPath, it);
	}

	AppendFilterItems(itemPath, it->m_Filters);
}

std::shared_ptr<Filters::FilterBase> BackupModel::GetFilter(const QModelIndex& rowIdx) const
{
	if (GetItemTypeByIndex(rowIdx) != EItemType::EIT_Filter)
		return nullptr;
		
	return *GetFilterIteratorByIndex(rowIdx);
}


QString BackupModel::DepthString(size_t depth)
{
	assert(depth != SrcPathDesc::Depthless);
	return tr("Depth: ") + QString::number(depth);
}


const QPixmap& BackupModel::GetInclusionPixmap(bool include) const
{
	return include ? m_IncludePixmap : m_ExcludePixmap;
}

QModelIndex BackupModel::FindSrcDescByPath(const fs::path& fsPath) const
{	
	ModelIterator begin_it(this, 0);
	ModelIterator end_it(this, rowCount());
	ModelIterator ret = find_if(begin_it, end_it, [this,&fsPath](const QModelIndex& idx)->bool
	{
		return fsPath == GetIterator(idx.row())->m_Path;
	});

	if (ret != end_it)
		return *ret;
	else
		return QModelIndex();
}



void BackupModel::AppendDepthItem(QStandardItem* itemPath, iterator& srcPathIter)
{		
	QStandardItem* itemDepth = new QStandardItem(DepthString(srcPathIter->m_Depth));
	itemDepth->setData(EItemType::EIT_Depth, EBackupItemRole::ItemTypeRole);
	//itemDepth->setData(itemPath->data(ERoles::IteratorRole), ERoles::IteratorRole);	
	itemPath->insertRow(0, itemDepth);
}

void BackupModel::AppendFilter(const QModelIndex& idxSrcPath, const shared_ptr<Filters::FilterBase>& newFilter)
{
	auto srcDesc_it = GetIterator(idxSrcPath.row());
	SrcPathDesc& srcPath = *srcDesc_it;
	ContainerOfFilters& filterList = srcPath.m_Filters;
		
	filterList.push_back(newFilter);

	AppendFilterItems(itemFromIndex(idxSrcPath), filterList, &(--filterList.end()));

	Emit_ElementChanged(idxSrcPath);
}

void BackupModel::UpdateFilter(const QModelIndex& idxFilter, const shared_ptr<Filters::FilterBase>& filter)
{
	FilterIterator filter_it = GetFilterIteratorByIndex(idxFilter);
	*filter_it = filter;
	setData(idxFilter, GetFilterSpecificText(filter.get()));
	
	Emit_ElementChanged(idxFilter.parent());	
}

void BackupModel::SetDepth(const QModelIndex& idxPath, size_t depth)
{
	auto srcDesc_it = GetIterator(idxPath);
	srcDesc_it->m_Depth = depth;

	QModelIndex idxDepth = FindDepthRowIdx(idxPath);	

	if (depth == SrcPathDesc::Depthless)
	{///Delete
		if (idxDepth.isValid())
		{
			removeRow(idxDepth.row(), idxPath);
		}
	}
	else
	{///Add or update
		if (idxDepth.isValid())
		{		
			setData(idxDepth, DepthString(depth));			
		}
		else
		{
			AppendDepthItem(itemFromIndex(idxPath), srcDesc_it);
		}
		Emit_ElementChanged(idxPath);
	}
}



bool BackupModel::removeRows(int row, int count, const QModelIndex & parent /*= QModelIndex()*/)
{	
	if (!parent.isValid())
	{		
		return ListModel::removeRows(row, count, parent);		
	}	

	auto srcDesc_it = GetIterator(parent);

	for (int c_row = row, size_row = row + count; c_row < size_row; c_row++)
	{
		QModelIndex idx = index(c_row, 0, parent);

		EItemType itemType = GetItemTypeByIndex(idx);

		switch (itemType)
		{
		case BackupModel::EIT_Depth:
		{
			srcDesc_it->m_Depth = SrcPathDesc::Depthless;
		}
		break;
		case BackupModel::EIT_Filter:
		{
			FilterIterator filter_it = GetFilterIteratorByIndex(idx);
			srcDesc_it->m_Filters.erase(filter_it);
		}
		break;		
		default:
			assert(0);
			break;
		}	
	}	
	
	if (!ListModel::removeRows(row, count, parent))
		return false;	

	Emit_ElementChanged(parent);
	return true;
}

Qt::ItemFlags BackupModel::flags(const QModelIndex & index) const
{
	return ListModel::flags(index) ^= Qt::ItemFlag::ItemIsEditable;	
}

//void BackupModel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
//{
//	if (order == Qt::AscendingOrder)
//	{
//		m_List->sort([](const SrcPathDesc& lhs, const SrcPathDesc& rhs)->bool
//		{
//		/*	if (!lhs.m_Include)
//				return false;
//
//			if (!rhs.m_Include)
//				return true;*/
//
//			return lhs.m_Path < rhs.m_Path;
//		});
//	}
//	else
//	{
//		m_List->sort([](const SrcPathDesc& lhs, const SrcPathDesc& rhs)->bool
//		{
//			/*if (!lhs.m_Include)
//				return true;
//
//			if (!rhs.m_Include)
//				return false;*/
//
//			return lhs.m_Path > rhs.m_Path;
//		});
//	}
//
//	ConvertAllItems();
//}

void BackupModel::AppendFilterItems(QStandardItem* itemPath, ContainerOfFilters& filters, FilterIterator* pBegin_it /*= nullptr*/)
{	
	FilterIterator it = pBegin_it ? *pBegin_it : filters.begin();
	for (; it != filters.end(); ++it)
	{
		auto& filter = *it;		
		QStandardItem* itemFilter = new QStandardItem(GetFilterSpecificText(filter.get()));
		//itemFilter->setData(GetInclusionPixmap(filter->IsInclusion()), Qt::DecorationRole);
		itemFilter->setData(EItemType::EIT_Filter, EBackupItemRole::ItemTypeRole);
		itemFilter->setData(QVariant::fromValue(it), EBackupItemRole::FilterIteratorRole);						
		//itemFilter->setData(itemPath->data(ERoles::IteratorRole), ERoles::IteratorRole);
		itemPath->appendRow(itemFilter);
	}
}

SrcPathDesc& BackupModel::GetSrcDescByIndex(const QModelIndex& idx) const
{
	return *GetSrcDescIteratorByIndex(idx);
}

SrcPathDescIterator BackupModel::GetSrcDescIteratorByIndex(const QModelIndex& idx) const
{
	QVariant contV = idx.data(ListModel::IteratorRole);
	assert(contV.isValid());
	return contV.value<SrcPathDescIterator>();
}


SrcPathDesc& BackupModel::GetSrcDescByItem(const QStandardItem* item) const
{	
	return *GetSrcDescIteratorByItem(item);
}

SrcPathDescIterator BackupModel::GetSrcDescIteratorByItem(const QStandardItem* item) const
{
	QVariant contV = item->data(ListModel::IteratorRole);
	assert(contV.isValid());
	return contV.value<SrcPathDescIterator>();
}



FilterIterator BackupModel::GetFilterIteratorByIndex(const QModelIndex& idx) const
{
	QVariant contV = idx.data(EBackupItemRole::FilterIteratorRole);
	assert(contV.isValid());
	return contV.value<FilterIterator>();
}

BackupModel::EItemType BackupModel::GetItemTypeByIndex(const QModelIndex& srcPathIdx) const
{
	QVariant contV = srcPathIdx.data(EBackupItemRole::ItemTypeRole);
	assert(contV.isValid());
	return static_cast<EItemType>(contV.toInt());
}

BackupModel::EItemType BackupModel::GetItemTypeByItem(const QStandardItem* item) const
{
	QVariant contV = item->data(EBackupItemRole::ItemTypeRole);
	assert(contV.isValid());
	return static_cast<EItemType>(contV.toInt());
}

QModelIndex BackupModel::FindDepthRowIdx(const QModelIndex& idxPath) const
{
	ModelIterator begin_it(this, 0, 0, idxPath);
	ModelIterator end_it(this, rowCount(idxPath), 0, idxPath);

	auto it = find_if(begin_it, end_it, [this](const QModelIndex& idx)->bool
	{
		return GetItemTypeByIndex(idx) == EIT_Depth;
	});

	if (it != end_it)
		return *it;
	else
		return QModelIndex();
}
