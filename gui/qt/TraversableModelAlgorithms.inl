#pragma once




template <class AttrContext, class AttrContextBase>
Qt::CheckState TraverserAbstract::TraverseAsDirEntry(const fs::path& srcPathAbsolute)
{
	fs::directory_iterator it(srcPathAbsolute);
	fs::directory_iterator end_it;


	EntryCheckStateRec dirCheckState;
	
	Qt::CheckState firstCheckState = IndeterminateCheckState;

	using FTraverseFiles_First = Traverser_Base<AttrContext, FileEntry, FlowFirst, AttrContextBase>;
	using FTraverseFiles_WhileParentIndeterminate = Traverser_Base<AttrContext, FileEntry, FlowWhileParentIndeterminate, AttrContextBase>;
	using FTraverseFiles_All = Traverser_Base<AttrContext, FileEntry, FlowAll, AttrContextBase>;




	FTraverseFiles_First(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();

	if (firstCheckState != IndeterminateCheckState)
	{
		FTraverseFiles_WhileParentIndeterminate(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();
		FTraverseFiles_All(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();
	}

	///File Traverse Finished
	using FTraverseDirs_First = Traverser_Base<AttrContext, DirEntry, FlowFirst, AttrContextBase>;
	using FTraverseDirs_WhileParentIndeterminate = Traverser_Base<AttrContext, DirEntry, FlowWhileParentIndeterminate, AttrContextBase>;
	using FTraverseDirs_All = Traverser_Base<AttrContext, DirEntry, FlowAll, AttrContextBase>;

	it = fs::directory_iterator(srcPathAbsolute);

	if (firstCheckState == IndeterminateCheckState)
	{
		FTraverseDirs_First(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();

		if (firstCheckState == IndeterminateCheckState)
		{
			dirCheckState.m_CheckState = Qt::Checked;
		}
		else
		{
			FTraverseDirs_WhileParentIndeterminate(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();
			FTraverseDirs_All(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();
		}
	}
	else
	{
		if (dirCheckState.m_CheckState != Qt::PartiallyChecked)
		{
			FTraverseDirs_WhileParentIndeterminate(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();
			FTraverseDirs_All(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();
		}
		else
		{
			FTraverseDirs_All(static_cast<AttrContext*>(this), it, dirCheckState, firstCheckState)();
		}
	}

	auto result = dirCheckState.m_CheckState;

	if (dirCheckState.m_CheckState != Qt::Unchecked)
	{
		QString qFileName;
		if (srcPathAbsolute != srcPathAbsolute.root_path())
			qFileName = QString::fromStdWString(srcPathAbsolute.filename());
		else
			qFileName = QString::fromStdWString(srcPathAbsolute.root_path());
				

		(*m_Entry2CheckState)[qFileName] = move(dirCheckState);
	}

	return result;
}


template <class AttrContext, class AttrContextBase>
void TraverserScoped::TraverseSrcPathEntry()
{
	if (!fs::exists(m_TraversePath))
	{
		return;
	}

	if (!m_SrcDesc->m_Include)
	{
		EntryCheckStateRec dirCheckState;
		dirCheckState.m_CheckState = Qt::Unchecked;
		vector<SrcPathDesc*> subSrcDescs;
		GetDeeperSrcDescs(fs::path(), subSrcDescs);

		if (!subSrcDescs.empty())
			m_SrcPathCheckState = OnDeeperSrcDescsReached(m_TraversePath, subSrcDescs, dirCheckState);
		else
			m_SrcPathCheckState = Qt::CheckState::Unchecked;
		
		if (dirCheckState.m_CheckState != Qt::Unchecked)
		{
			QString qFileName;
			if (m_TraversePath != m_TraversePath.root_path())
				qFileName = QString::fromStdWString(m_TraversePath.filename());
			else
				qFileName = QString::fromStdWString(m_TraversePath.root_path());


			(*m_Entry2CheckState)[qFileName] = move(dirCheckState);
		}

		return;
	}

	
	if (fs::is_directory(m_TraversePath))
	{
		m_SrcPathCheckState = TraverseAsDirEntry<AttrContext, AttrContextBase>(m_TraversePath);
	}
	else
	{
		if (FileFilters(m_TraversePath))
		{
			QString qFileName = QString::fromStdWString(m_TraversePath.filename());
			(*m_Entry2CheckState)[qFileName].m_CheckState = Qt::CheckState::Checked;
			m_SrcPathCheckState = Qt::CheckState::Checked;
		}
		else
		{
			m_SrcPathCheckState = Qt::CheckState::Unchecked;
		}
	}
}