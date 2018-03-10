#pragma once

template <typename Type>
void SafeDelete(Type*& p)
{
	if (p)
	{
		delete p;
		p = nullptr;
	}
}

list<ParsedSources>::iterator findBackupByName(MyBCopyFile* mbcFile, const wstring& backupName);



QString EBackupStage2String(EBackupStage stage);
QString EStorageStage2String(EStorageStage stage);

QString EBackupError2String(EBackupError error);



QString EntryTypeToString(Filters::EntryType entryType);

QString AppendSpaceIfNotEmpty(const QString& s);
void SpaceIfNotEmpty(QString& s);

QString GetFilterSpecificText(Filters::FilterBase* filter);


bool LessByPath(const SrcPathDesc& lhs, const SrcPathDesc& rhs);



template <class Type>
inline Type* qchecked_cast(QObject* ptr)
{
	assert(qobject_cast<Type*>(ptr) != nullptr);
	return static_cast<Type*>(ptr);
}

template <class Type>
inline Type* qchecked_cast(const QObject* ptr)
{
	assert(qobject_cast<const Type*>(ptr) != nullptr);
	return static_cast<const Type*>(ptr);
}

void RemoveSelectedRowsFromModel(QAbstractItemView* view);

using ContainerOfSrcPathDesc = list<SrcPathDesc>;
using SrcPathDescIterator = list<SrcPathDesc>::iterator;
using ContainerOfFilters = list< shared_ptr<Filters::FilterBase> >;
using FilterIterator = list< shared_ptr<Filters::FilterBase> >::iterator;

namespace std
{

	template <>
	struct hash<SrcPathDescIterator>
	{
		typedef SrcPathDescIterator argument_type;
		typedef size_t result_type;

		size_t operator()(const SrcPathDescIterator& _Keyval) const
		{	
			return std::hash<decltype(&*_Keyval)>()(&*_Keyval);
		}
	};


	template <>
	struct hash<QString>
	{
		typedef QString argument_type;
		typedef size_t result_type;

		size_t operator()(const QString& _Keyval) const
		{
			return qHash(_Keyval);
		}
	};

	/*template <>
	struct less<SrcPathDescIterator>
	{
		typedef SrcPathDescIterator argument_type;
		typedef size_t result_type;

		size_t operator()(const SrcPathDescIterator& _Keyval) const
		{
			return std::hash<decltype(&*_Keyval)>()(&*_Keyval);
		}
	};*/
}






class StorageImplementator;

class StorageImplementatorSelector : public Singleton<StorageImplementatorSelector>
{
public:	
	StorageImplementator* GetStorageImplementator(Storage* storage);
	StorageImplementator* GetStorageImplementator(type_index storageTypeIndex);
	static type_index GetStorageTypeIndex(const Storage* storage);	
private:
	map<type_index, StorageImplementator*> m_TypeIndex2Implementator;
	friend class StorageImplementator;
};


class StorageImplementator
{
public:
	virtual ~StorageImplementator() = default;		
	virtual QString GetSpecificText(Storage* storage) const = 0;
	virtual QString GetAddress(Storage* storage) const = 0;
	virtual QString GetTypeName(Storage* storage) const = 0;
	virtual void SerializeToXML(pugi::xml_node& xmlStorage, Storage* storage) const = 0;
protected:
	void RegisterStorageImplementator(type_index implementableStorageTypeIndex, StorageImplementator* performer);
};



template <class StorageImplementatorT, class ImplementableStorage>
class StorageImplementatorBase : public StorageImplementator, public Singleton<StorageImplementatorT>
{
public:
	StorageImplementatorBase()
	{
		RegisterStorageImplementator(typeid(ImplementableStorage), this);
	}
protected:	
	static ImplementableStorage* hl_cast(Storage* storage)
	{
		assert(dynamic_cast<ImplementableStorage*>(storage));
		return static_cast<ImplementableStorage*>(storage);
	}
	//static const StorageImplementatorT sc_HLImplementator;
};

class WebDavImplementator : public StorageImplementatorBase<WebDavImplementator, WebDavStorage>
{
protected:
	virtual QString GetSpecificText(Storage* storage) const override;
	virtual QString GetAddress(Storage* storage) const override;
	virtual QString GetTypeName(Storage* storage) const override;
	virtual void SerializeToXML(pugi::xml_node& xmlStorage, Storage* storage) const override;	
	
};


class LocalDiskImplementator : public StorageImplementatorBase<LocalDiskImplementator, LocalDiskStorage>
{
protected:
	virtual QString GetSpecificText(Storage* storage) const override;
	virtual QString GetAddress(Storage* storage) const override;
	virtual QString GetTypeName(Storage* storage) const override;
	virtual void SerializeToXML(pugi::xml_node& xmlStorage, Storage* storage) const override;
};








class FilterImplementator;

class FilterImplementatorSelector : public Singleton<FilterImplementatorSelector>
{
public:
	FilterImplementator* GetFilterImplementator(Filters::FilterBase* filter);
	FilterImplementator* GetFilterImplementator(type_index filterTypeIndex);
	static type_index GetFilterTypeIndex(const Filters::FilterBase* filter);
private:
	map<type_index, FilterImplementator*> m_TypeIndex2Implementator;
	friend class FilterImplementator;
};

class FilterEditor;

class FilterImplementator
{
public:	
	virtual ~FilterImplementator() = default;
	virtual QString GetSpecificText(Filters::FilterBase* filter) const = 0;
	virtual void SerializeToXML(pugi::xml_node& xmlFilters, Filters::FilterBase* filter) const = 0;
	virtual unique_ptr<FilterEditor> MakeEditorDialog() const = 0;
protected:
	void RegisterFilterImplementator(type_index implementableStorageTypeIndex, FilterImplementator* performer);
};


template <class FilterImplementatorT, class ImplementableFilter>
class FilterImplementatorBase : public FilterImplementator, public Singleton<FilterImplementatorT>
{
public:
	FilterImplementatorBase()
	{
		RegisterFilterImplementator(typeid(ImplementableFilter), this);
	}
protected:
	static ImplementableFilter* hl_cast(Filters::FilterBase* storage)
	{
		assert(dynamic_cast<ImplementableFilter*>(storage));
		return static_cast<ImplementableFilter*>(storage);
	}
	//static const FilterImplementatorT sc_HLImplementator;
};












class ToolImplementator;

class ToolImplementatorSelector : public Singleton<ToolImplementatorSelector>
{
public:
	ToolImplementator* GetToolImplementator(Tool* tool);
	ToolImplementator* GetToolImplementator(type_index toolTypeIndex);
	ToolImplementator* GetToolImplementatorByTypename(const QString& typeName);
	static type_index GetToolTypeIndex(const Tool* tool);
	const map<type_index, ToolImplementator*>& GetMap() const;
private:
	map<type_index, ToolImplementator*> m_TypeIndex2Implementator;
	friend class ToolImplementator;
};


class ToolImplementator
{
public:
	virtual ~ToolImplementator() = default;		
	virtual QString GetDefaultExecutablePath() const = 0;
	virtual QString GetFileDialogFilter() const = 0;
	virtual QString GetTypeName() const = 0;
	virtual void SerializeToXML(pugi::xml_node& xmlTool, Tool* tool) const = 0;
	virtual std::shared_ptr<Tool> MakeTool(const QString& exePath, const QString& passwd) = 0;
protected:
	void RegisterToolImplementator(type_index implementableToolTypeIndex, ToolImplementator* performer);
};



template <class ToolImplementatorT, class ImplementableTool>
class ToolImplementatorBase : public ToolImplementator, public Singleton<ToolImplementatorT>
{
public:
	ToolImplementatorBase()
	{
		RegisterToolImplementator(typeid(ImplementableTool), this);
	}
protected:
	static ImplementableTool* hl_cast(Tool* tool)
	{
		assert(dynamic_cast<ImplementableTool*>(tool));
		return static_cast<ImplementableTool*>(tool);
	}
	//static const ToolImplementatorT sc_HLImplementator;
};

class ArchiverRarImplementator : public ToolImplementatorBase<ArchiverRarImplementator, ArchiverRar>
{
protected:
	virtual QString GetDefaultExecutablePath() const override;
	virtual QString GetFileDialogFilter() const override;
	virtual QString GetTypeName() const override;
	virtual void SerializeToXML(pugi::xml_node& xmlTool, Tool* tool) const override;
	virtual std::shared_ptr<Tool> MakeTool(const QString& exePath, const QString& passwd) override;
};


class Archiver7zImplementator : public ToolImplementatorBase<Archiver7zImplementator, Archiver7z>
{
protected:
	virtual QString GetDefaultExecutablePath() const override;
	virtual QString GetFileDialogFilter() const override;
	virtual QString GetTypeName() const override;
	virtual void SerializeToXML(pugi::xml_node& xmlTool, Tool* tool) const override;
	virtual std::shared_ptr<Tool> MakeTool(const QString& exePath, const QString& passwd) override;
};



