#pragma once

#include <QModelIndex>
//Based on Leechcraft's ModelIterator (https://github.com/0xd34df00d/leechcraft)

		class ModelIterator
		{
		public:			
			ModelIterator(const QAbstractItemModel *model, int row, int col = 0, const QModelIndex& parent = QModelIndex());

			//ModelIterator::ModelIterator(QAbstractItemModel *model, const QModelIndex& idx, Direction dir /*= Direction::Rows*/)
			//	: Model_(model)
			//	, Parent_(idx.parent())
			//	, Row_(idx.row())
			//	, Col_(idx.column())			
			//{

			//}
			
			ModelIterator& operator++ ();

			
			ModelIterator operator++ (int);

			ModelIterator& operator-- ();

		
			ModelIterator operator-- (int);
		
			ModelIterator& operator+= (int count);
		
			ModelIterator& operator-= (int count);

			int operator- (const ModelIterator& other) const;			
			friend  bool operator== (const ModelIterator& left, const ModelIterator& right);			
			friend  bool operator!= (const ModelIterator& left, const ModelIterator& right);

			
			QModelIndex operator* () const;

			friend bool operator< (const ModelIterator& left, const ModelIterator& right);
			
			int GetRow() const;
		
			int GetCol() const;
		private:
			const QAbstractItemModel * Model_;
			QModelIndex Parent_;

			int Row_;
			int Col_;

			int& GetIncrementable();
			int GetIncrementable() const;
		};



namespace std
{
	template<>
	struct iterator_traits<ModelIterator>
	{
		typedef QModelIndex value_type;
		typedef int difference_type;

		typedef random_access_iterator_tag iterator_category;
	};
}