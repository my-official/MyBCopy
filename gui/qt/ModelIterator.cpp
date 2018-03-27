#include "qtgui_pch.h"
#include "ModelIterator.h"


		ModelIterator::ModelIterator(const QAbstractItemModel *model,
			int row, int col, const QModelIndex& parent)
			: Model_(model)
			, Parent_(parent)
			, Row_(row)
			, Col_(col)			
		{
		}


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
			return left.Row_ < right.Row_;			
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
			return Row_;
		}

		int ModelIterator::GetIncrementable() const
		{
			return Row_;
		}