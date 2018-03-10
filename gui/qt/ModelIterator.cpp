#include "qtgui_pch.h"
#include "ModelIterator.h"


		ModelIterator::ModelIterator(const QAbstractItemModel *model,
			int row, int col, const QModelIndex& parent, ModelIterator::Direction dir)
			: Model_(model)
			, Parent_(parent)
			, Row_(row)
			, Col_(col)
			, Dir_(dir)
		{
		}

		//ModelIterator::ModelIterator(QAbstractItemModel *model, const QModelIndex& idx, Direction dir /*= Direction::Rows*/)
		//	: Model_(model)
		//	, Parent_(idx.parent())
		//	, Row_(idx.row())
		//	, Col_(idx.column())
		//	, Dir_(dir)
		//{

		//}

		ModelIterator& ModelIterator::operator++ ()
		{
			++GetIncrementable();
			return *this;
		}

		ModelIterator ModelIterator::operator++ (int)
		{
			ModelIterator oldThis(*this);
			++GetIncrementable();
			return oldThis;
		}

		ModelIterator& ModelIterator::operator-- ()
		{
			--GetIncrementable();
			return *this;
		}

		ModelIterator ModelIterator::operator-- (int)
		{
			ModelIterator oldThis(*this);
			--GetIncrementable();
			return oldThis;
		}

		ModelIterator& ModelIterator::operator+= (int diff)
		{
			GetIncrementable() += diff;
			return *this;
		}

		ModelIterator& ModelIterator::operator-= (int diff)
		{
			GetIncrementable() -= diff;
			return *this;
		}

		int ModelIterator::operator- (const ModelIterator& other) const
		{
			return GetIncrementable() - other.GetIncrementable();
		}

		bool operator== (const ModelIterator& left, const ModelIterator& right)
		{
			return left.Row_ == right.Row_ &&
				left.Col_ == right.Col_ &&
				left.Model_ == right.Model_ &&
				left.Parent_ == right.Parent_;
		}

		bool operator!= (const ModelIterator& left, const ModelIterator& right)
		{
			return !(left == right);
		}
		bool operator< (const ModelIterator& left, const ModelIterator& right)
		{
			assert(left.Dir_ == right.Dir_);
			if (left.Dir_ == ModelIterator::Direction::Rows)
				return left.Row_ < right.Row_;
			else
				return left.Col_ < right.Col_;
		}

		QModelIndex ModelIterator::operator*() const
		{
			return Model_->index(Row_, Col_, Parent_);
		}

		int ModelIterator::GetRow() const
		{
			return Row_;
		}

		int ModelIterator::GetCol() const
		{
			return Col_;
		}

		int& ModelIterator::GetIncrementable()
		{
			switch (Dir_)
			{
			case Direction::Rows:
				return Row_;
			case Direction::Cols:
				return Col_;
			}

			qWarning() << Q_FUNC_INFO
				<< "unknown direction";
			return Row_;
		}

		int ModelIterator::GetIncrementable() const
		{
			switch (Dir_)
			{
			case Direction::Rows:
				return Row_;
			case Direction::Cols:
				return Col_;
			}

			qWarning() << Q_FUNC_INFO
				<< "unknown direction";
			return Row_;
		}