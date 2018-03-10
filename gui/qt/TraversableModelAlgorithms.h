#pragma once
#include "TraversableFSModel.h"



	enum class EFlowControl
	{
		//Break;
		Return,
		Continue,
		NoAction
	};



	class FlowFirst
	{
	public:
		static EFlowControl ResultFlow(EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState, Qt::CheckState checkState)
		{
			firstCheckState = checkState;
			return EFlowControl::Return;
		}
	};

	class FlowWhileParentIndeterminate
	{
	public:
		static EFlowControl ResultFlow(EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState, Qt::CheckState checkState)
		{
			if (firstCheckState != checkState && checkState != IndeterminateCheckState)
			{
				dirCheckState.m_CheckState = Qt::CheckState::PartiallyChecked;
				return EFlowControl::Return;
			}
			else
			{
				return EFlowControl::NoAction;
			}
		}
	};

	class FlowAll
	{
	public:
		static EFlowControl ResultFlow(EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState, Qt::CheckState checkState)
		{
			return EFlowControl::NoAction;
		}
	};





	class FileEntry
	{
	public:
		static bool IsEntryTypeSatisfied(const fs::path& entryPath)
		{
			return !fs::is_directory(entryPath);
		}
	};


	class DirEntry
	{
	public:
		static bool IsEntryTypeSatisfied(const fs::path& entryPath)
		{
			return fs::is_directory(entryPath);
		}
	};








	template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
	class Traverser_Base;


	template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
	class OnEndReached
	{
	public:		//Friend this Call
		static void FC(Traverser_Base<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>* FThis)
		{
			
		}
	};

	
	template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
	class ApplyFilters
	{
	public:		
		static bool FC(Traverser_Base<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>* FThis);
	};


	template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
	class OnFiltersPassed
	{
	public:
		static Qt::CheckState FC(Traverser_Base<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>* FThis);
	};


	template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
	class OnFilterFailed
	{
	public:
		static Qt::CheckState FC(Traverser_Base<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>* FThis)
		{
			return Qt::Unchecked;
		}
	};
	




	template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase = AttrContext>
	class Traverser_Base
	{
	public:
		Traverser_Base(AttrContext* parent, fs::directory_iterator& it, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState)
			: m_Parent(parent)
			, m_It(it)
			, m_DirCheckState(dirCheckState)
			, m_FirstCheckState(firstCheckState)
		{

		}

	private:
		AttrContext* m_Parent;

		fs::directory_iterator& m_It;
		EntryCheckStateRec& m_DirCheckState;
		Qt::CheckState& m_FirstCheckState;

		fs::path m_EntityPath;
		fs::path m_EntityName;

		fs::path m_SrcEntryPath;

		friend class OnEndReached<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>;
		friend class ApplyFilters<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>;
		friend class OnFiltersPassed<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>;
		friend class OnFilterFailed<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>;

	public:

		virtual void operator()()
		{
#define GOTO_FLOW if (flow == EFlowControl::Return)\
			{\
			++m_It; \
				return; \
			}\
				\
	if (flow == EFlowControl::Continue)\
		continue

			EFlowControl flow;
			fs::directory_iterator end_it;

			for (; m_It != end_it; ++m_It)
			{
				m_EntityPath = m_It->path();

				if (!AttrEntryType::IsEntryTypeSatisfied(m_EntityPath))
					continue;

				m_EntityName = m_EntityPath.filename();
				m_SrcEntryPath = m_Parent->m_SrcOriginPath / m_EntityName;

				SrcPathDesc* srcDesc = m_Parent->FindSubSrcDescForEntry(m_EntityName);
				if (srcDesc)
				{
					Qt::CheckState checkState = m_Parent->OnSrcDescReached(srcDesc, m_EntityName, m_DirCheckState, m_FirstCheckState);
					flow = AttrFlow::ResultFlow(m_DirCheckState, m_FirstCheckState, checkState);
					GOTO_FLOW;
					continue;
				}


				if (ApplyFilters<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>::FC(this))
				{
					Qt::CheckState checkState = OnFiltersPassed<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>::FC(this);
					flow = AttrFlow::ResultFlow(m_DirCheckState, m_FirstCheckState, checkState);
					GOTO_FLOW;
				}
				else
				{
					Qt::CheckState checkState = OnFilterFailed<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>::FC(this);
					flow = AttrFlow::ResultFlow(m_DirCheckState, m_FirstCheckState, checkState);
					GOTO_FLOW;
				}
			}


			OnEndReached<AttrContext, AttrEntryType, AttrFlow, AttrContextBase>::FC(this);
		}		
	};


	
	


	template <class AttrContext, class AttrContextBase>
	class OnEndReached<AttrContext, DirEntry, FlowWhileParentIndeterminate, AttrContextBase>
	{
	public:
		static void FC(Traverser_Base<AttrContext, DirEntry, FlowWhileParentIndeterminate, AttrContextBase>* FThis)
		{
			//		EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState
			FThis->m_DirCheckState.m_CheckState = FThis->m_FirstCheckState;
		}
	};

	template <class AttrContext, class AttrFlow, class AttrContextBase>
	class OnFilterFailed<AttrContext, DirEntry, AttrFlow, AttrContextBase>
	{
	public:
		static Qt::CheckState FC(Traverser_Base<AttrContext, DirEntry, AttrFlow, AttrContextBase>* FThis)
		{
			vector<SrcPathDesc*> subSrcDescs;
			FThis->m_Parent->GetDeeperSrcDescs(FThis->m_SrcEntryPath, subSrcDescs);

			if (!subSrcDescs.empty())
				return FThis->m_Parent->OnDeeperSrcDescsReached(FThis->m_EntityName, subSrcDescs, FThis->m_DirCheckState);
			else
				return Qt::CheckState::Unchecked;
		}
	};

	









	class TraverserAbstract
	{
	protected:
		TraverserAbstract(TraversableModelThread* traversableThread, const fs::path& traversePath, const fs::path& srcOriginPath, unordered_map<QString, EntryCheckStateRec>* entry2CheckState);
	public:		
		Qt::CheckState m_SrcPathCheckState;
	protected:
		TraversableModelThread*						m_TraversableThread;
		fs::path									m_TraversePath;
		fs::path									m_SrcOriginPath;
		//EntryCheckStateRec*							m_ParentEntryCheckState;
		unordered_map<QString, EntryCheckStateRec>*	m_Entry2CheckState;

		
		template <class AttrContext, class AttrContextBase>
		Qt::CheckState TraverseAsDirEntry(const fs::path& srcPathAbsolute);


		Qt::CheckState ScopedRecursion(const fs::path& entityName, SrcPathDesc* srcPath, EntryCheckStateRec& dirCheckState);
		Qt::CheckState InitialRecursion(const fs::path& entityName, vector<SrcPathDesc*>* srcDescs, EntryCheckStateRec& dirCheckState);

		Qt::CheckState UnscopedRecursion(const fs::path& entityName, SrcPathDesc* srcPath, EntryCheckStateRec& dirCheckState);
		Qt::CheckState ForwardRecursion(const fs::path& entityName, vector<SrcPathDesc*>* srcDescs, EntryCheckStateRec& dirCheckState);
	};





	class TraverserInitial : public TraverserAbstract
	{
	public:
		TraverserInitial(TraversableModelThread* traversableThread, const fs::path& traversePath, const fs::path& srcOriginPath, vector<SrcPathDesc*>* srcDescs, unordered_map<QString, EntryCheckStateRec>* entry2CheckState);		
		void Traverse();	
	protected:
		vector<SrcPathDesc*>*						m_SrcDescs;
								

		//////
		SrcPathDesc* FindSubSrcDescForEntry(const fs::path& entryName);
		Qt::CheckState OnSrcDescReached(SrcPathDesc* srcPath, const fs::path& entryName, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState);
		void GetDeeperSrcDescs(const fs::path& srcEntryPath, vector<SrcPathDesc*>& subSrcDescs);
		Qt::CheckState OnDeeperSrcDescsReached(const fs::path& entryName, vector<SrcPathDesc*>& subSrcDescs, EntryCheckStateRec& dirCheckState);

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class Traverser_Base;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class OnFilterFailed;
	};
		
	template <class AttrContext, class AttrEntryType, class AttrFlow>
	class ApplyFilters<AttrContext, AttrEntryType, AttrFlow, TraverserInitial>
	{
	public:		
		static bool FC(Traverser_Base<AttrContext, AttrEntryType, AttrFlow, TraverserInitial>*)
		{
			return false;
		}
	};

	template <class AttrContext, class AttrEntryType, class AttrFlow>
	class OnFiltersPassed<AttrContext, AttrEntryType, AttrFlow, TraverserInitial>
	{
	public:
		static Qt::CheckState FC(Traverser_Base<AttrContext, AttrEntryType, AttrFlow, TraverserInitial>* FThis)
		{
			assert(0);
			throw std::logic_error("Unreachable code reached");
		}
	};





	class TraverserScoped : public TraverserAbstract, protected TraversedBackup::Filtering
	{
	public:
		TraverserScoped(TraversableModelThread* traversableThread, const fs::path& originPath, const fs::path& srcOriginPath, SrcPathDesc* srcPath, unordered_map<QString, EntryCheckStateRec>* entry2CheckState);
		void Traverse();
		
	protected:
		template <class AttrContext, class AttrContextBase>
		void TraverseSrcPathEntry();
	private:
		SrcPathDesc*								m_SrcDesc;
				

		//////
		SrcPathDesc* FindSubSrcDescForEntry(const fs::path& entryName);
		Qt::CheckState OnSrcDescReached(SrcPathDesc* srcPath, const fs::path& entryName, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState);
		void GetDeeperSrcDescs(const fs::path& srcEntryPath, vector<SrcPathDesc*>& subSrcDescs);
		Qt::CheckState OnDeeperSrcDescsReached(const fs::path& entryName, vector<SrcPathDesc*>& subSrcDescs, EntryCheckStateRec& dirCheckState);
		
		
		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class Traverser_Base;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class ApplyFilters;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class OnFiltersPassed;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class OnFilterFailed;		
	};



	template <class AttrContext, class AttrFlow>
	class ApplyFilters<AttrContext, DirEntry, AttrFlow, TraverserScoped>
	{
	public:		
		static bool FC(Traverser_Base<AttrContext, DirEntry, AttrFlow, TraverserScoped>* FThis)
		{			
			if (FThis->m_Parent->m_SrcDesc->m_Depth != 0)
				return FThis->m_Parent->DirFilters(FThis->m_EntityPath);
			else
				return false;		
		}
	};

	template <class AttrContext, class AttrFlow>
	class ApplyFilters<AttrContext, FileEntry, AttrFlow, TraverserScoped>
	{
	public:		
		static bool FC(Traverser_Base<AttrContext, FileEntry, AttrFlow, TraverserScoped>* FThis)
		{			
			return FThis->m_Parent->FileFilters(FThis->m_EntityPath);
		}
	};
	

	template <class AttrFlow>
	class OnFiltersPassed<TraverserScoped, DirEntry, AttrFlow, TraverserScoped>
	{
	public:
		static Qt::CheckState FC(Traverser_Base<TraverserScoped, DirEntry, AttrFlow, TraverserScoped>* FThis)
		{
			SrcPathDesc inheritedSrcPath = SrcPathDesc::CreateViaInheritance(FThis->m_EntityName, *FThis->m_Parent->m_SrcDesc);
			return FThis->m_Parent->ScopedRecursion(FThis->m_EntityName, &inheritedSrcPath, FThis->m_DirCheckState);
		}
	};

	template <class AttrContext, class AttrFlow>
	class OnFiltersPassed<AttrContext, FileEntry, AttrFlow, TraverserScoped>
	{
	public:
		static Qt::CheckState FC(Traverser_Base<AttrContext, FileEntry, AttrFlow, TraverserScoped>* FThis)
		{
			QString qFileName = QString::fromStdWString(FThis->m_EntityName);
			FThis->m_DirCheckState.m_SubEntries[qFileName].m_CheckState = Qt::Checked;
			return Qt::Checked;
		}
	};



	


	class TraverserForward : public TraverserInitial
	{
	public:
		using TraverserInitial::TraverserInitial;
		void Traverse();
	private:		
		//////	
		Qt::CheckState OnSrcDescReached(SrcPathDesc* srcPath, const fs::path& entryName, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState);
		Qt::CheckState OnDeeperSrcDescsReached(const fs::path& entryName, vector<SrcPathDesc*>& subSrcDescs, EntryCheckStateRec& dirCheckState);

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class Traverser_Base;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class ApplyFilters;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class OnFiltersPassed;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class OnFilterFailed;
	};



	class TraverserUnscoped : public TraverserScoped
	{
	public:
		using TraverserScoped::TraverserScoped;	
		void Traverse();
	private:		

		//////	
		Qt::CheckState OnSrcDescReached(SrcPathDesc* srcPath, const fs::path& entryName, EntryCheckStateRec& dirCheckState, Qt::CheckState& firstCheckState);		
		Qt::CheckState OnDeeperSrcDescsReached(const fs::path& entryName, vector<SrcPathDesc*>& subSrcDescs, EntryCheckStateRec& dirCheckState);

		
		
		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class Traverser_Base;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class ApplyFilters;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class OnFiltersPassed;

		template <class AttrContext, class AttrEntryType, class AttrFlow, class AttrContextBase>
		friend class OnFilterFailed;
	};



	template <class AttrFlow>
	class OnFiltersPassed<TraverserUnscoped, DirEntry, AttrFlow, TraverserScoped>
	{
	public:
		static Qt::CheckState FC(Traverser_Base<TraverserUnscoped, DirEntry, AttrFlow, TraverserScoped>* FThis)
		{
			SrcPathDesc inheritedSrcPath = SrcPathDesc::CreateViaInheritance(FThis->m_EntityName, *FThis->m_Parent->m_SrcDesc);
			return FThis->m_Parent->UnscopedRecursion(FThis->m_EntityName, &inheritedSrcPath, FThis->m_DirCheckState);
		}
	};




#include "TraversableModelAlgorithms.inl"