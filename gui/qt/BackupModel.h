#pragma once
#include "Utils_qtgui.h"
#include "ModelIterator.h"

class ParsedSources;



class ContainerBasedModel : public QStandardItemModel
{
	Q_OBJECT
public:
	explicit ContainerBasedModel(QObject*parent = Q_NULLPTR);

signals:
	void ContainerReset();
	void ElementAdded(QModelIndex idx);
	void ElementChanged(QModelIndex idx);
	void ElementBeforeRemove(int row, int count);
	void ElementAfterRemove(int row, int count);

protected:
	void Emit_ContainerReset();
	void Emit_ElementAdded(QModelIndex idx);
	void Emit_ElementChanged(QModelIndex idx);
	void Emit_ElementBeforeRemove(int row, int count);
	void Emit_ElementAfterRemove(int row, int count);	
};




template<typename T>
class ListModel : public ContainerBasedModel
{	
public:	
	explicit ListModel(QObject*parent = Q_NULLPTR)
		: ContainerBasedModel(parent)
		, m_List(&m_NewList)		
	{
		
	}


	enum ERoles : int
	{
		IteratorRole = Qt::UserRole + 1
	};

	using iterator = typename std::list< T >::iterator;

	Qt::ItemFlags flags(const QModelIndex & index) const override
	{
		Qt::ItemFlags inheritedFlags = QStandardItemModel::flags(index);
		inheritedFlags ^= Qt::ItemFlag::ItemIsEditable;
		return inheritedFlags;
	}

	void SetSourceList(list < T >* srcList)
	{		
		m_List = srcList;

		ConvertAllItems();
		
		Emit_ContainerReset();
	}

	const list < T >* GetSourceList() const
	{
		return m_List;
	}

	iterator GetIterator(const QModelIndex& idx) const
	{
		QVariant itv = data(index(idx.row(), 0), ERoles::IteratorRole);
		assert(itv.isValid());
		return itv.value<iterator>();
	}

	iterator GetIterator(int row) const
	{
		QVariant itv = data(index(row, 0), ERoles::IteratorRole);
		assert(itv.isValid());
		return itv.value<iterator>();
	}


	QModelIndex GetIndex(const iterator& it) const
	{
		ModelIterator ret = find_if(begin(), end(), [&it](const QModelIndex& idx)->bool
		{
			return it == GetIterator(idx.row());
		});

		if (ret != end_it)
			return *ret;
		else
			return QModelIndex();
	}

	void Add(const T& storage)
	{
		iterator it;
		
		int row = rowCount();
		insertRow(row);
		m_List->push_back(storage);
		it = --m_List->end();

		QList<QStandardItem *> newRow;
		ConvertToItemRow(newRow, it);
		assert(!newRow.isEmpty());
		newRow.front()->setData(QVariant::fromValue(it), ERoles::IteratorRole);

		//appendRow(newRow);

		for (int c = 0, size = newRow.size(); c < size; c++)
			setItem(row, c, newRow[c]);

		Emit_ElementAdded(index(row, 0));
	}

	void Insert(int row, const T& storage)
	{
		insertRow(row);
		iterator  it = m_List->begin();
		advance(it, row);
		it = m_List->insert(it,storage);

		QList<QStandardItem *> newRow;
		ConvertToItemRow(newRow, it);
		assert(!newRow.isEmpty());
		newRow.front()->setData(QVariant::fromValue(it), ERoles::IteratorRole);

		for (int c = 0, size = newRow.size(); c < size; c++)
			setItem(row, c, newRow[c]);

		Emit_ElementAdded(index(row, 0));
	}

	void Set(const QModelIndex& idx, const T& storage)
	{
		iterator it;
		int row = idx.row();
		it = GetIterator(idx.row());
		*it = storage;		

		QList<QStandardItem *> newRow;
		ConvertToItemRow(newRow, it);
		assert(!newRow.isEmpty());
		newRow.front()->setData(QVariant::fromValue(it), ERoles::IteratorRole);

		for (int c = 0, size = newRow.size(); c < size; c++)
			setItem(row, c, newRow[c]);
		
		QModelIndex _idx = idx;
		Emit_ElementChanged(_idx);
	}


	void AddOrSet(const QModelIndex& idx, const T& storage)
	{
		if (!idx.isValid())		
			Add(storage);		
		else		
			Set(idx, storage);
	}

	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) override
	{
		if (parent.isValid())
			return QStandardItemModel::removeRows(row, count, parent);

		auto first = GetIterator(row);
		auto last = first;
		advance(last, count);
		
		Emit_ElementBeforeRemove(row, count);
		if (!QStandardItemModel::removeRows(row, count, parent))
			return false;

		m_List->erase(first, last);
		Emit_ElementAfterRemove(row, count);
		return true;
	}

protected:
	void ConvertAllItems()
	{
		int count = rowCount();
		if (count > 0)
			QStandardItemModel::removeRows(0, count);

		for (auto it = m_List->begin(); it != m_List->end(); ++it)
		{
			QList<QStandardItem*> newRow;
			ConvertToItemRow(newRow, it);
			assert(!newRow.isEmpty());
			newRow.front()->setData(QVariant::fromValue(it), ERoles::IteratorRole);
			this->appendRow(newRow);
		}
	}
	virtual void ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& srcPathIter) = 0;
private:
	list < T > m_NewList;	
protected:
	list < T >* m_List;

	
};





Q_DECLARE_METATYPE(ListModel< ParsedSources >::iterator)











Q_DECLARE_METATYPE(ListModel< SrcPathDesc >::iterator)
Q_DECLARE_METATYPE(FilterIterator)

class BackupModel : public ListModel< SrcPathDesc >
{
	Q_OBJECT
public:
	explicit BackupModel(QObject*parent = Q_NULLPTR);
	
	enum EBackupItemRole : int
	{
		ItemTypeRole = ERoles::IteratorRole + 1,		
		FilterIteratorRole = ItemTypeRole + 1,
	};

	enum EItemType : int
	{
		EIT_Depth = 0,
		EIT_Filter = 1,
		EIT_SrcDescPath = 2,
	};

	void SetBackup(ParsedSources* backup);	
	
	
	QModelIndex FindSrcDescByPath(const fs::path& fsPath) const;		

	SrcPathDescIterator GetSrcDescIteratorByIndex(const QModelIndex& idx) const;
	SrcPathDescIterator GetSrcDescIteratorByItem(const QStandardItem* item) const;

	SrcPathDesc& GetSrcDescByIndex(const QModelIndex& idx) const;
	SrcPathDesc& GetSrcDescByItem(const QStandardItem* item) const;

	FilterIterator GetFilterIteratorByIndex(const QModelIndex& idx) const;	

	EItemType GetItemTypeByIndex(const QModelIndex& srcPathIdx) const;
	EItemType GetItemTypeByItem(const QStandardItem* item) const;

	QModelIndex FindDepthRowIdx(const QModelIndex& idxPath) const;


	shared_ptr<Filters::FilterBase> GetFilter(const QModelIndex& rowIdx) const;
		
	void AppendFilter(const QModelIndex& idxSrcPath, const shared_ptr<Filters::FilterBase>& newFilter);
	void UpdateFilter(const QModelIndex& idxFilter, const shared_ptr<Filters::FilterBase>& filter);

	void SetDepth(const QModelIndex& idxPath, size_t depth);//Add, update or delete item		

	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;

	Qt::ItemFlags flags(const QModelIndex & index) const override;

//	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
protected:
	using ListModel< SrcPathDesc >::SetSourceList;

	virtual void ConvertToItemRow(QList<QStandardItem *>& newRow, iterator& srcPathIter) override;

private:
	void AppendFilterItems(QStandardItem* itemPath, ContainerOfFilters& filters, FilterIterator* pBegin_it = nullptr);
	void AppendDepthItem(QStandardItem* itemPath, iterator& srcPathIter);			

	static QString DepthString(size_t depth);	

	QPixmap m_IncludePixmap;
	QPixmap m_ExcludePixmap;	
};