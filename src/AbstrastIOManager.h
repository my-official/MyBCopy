#pragma once
#include "Utils.h"
#include "BackupJobExecuter.h"
#include "Storage.h"
#include "ThreadedEventLoop.h"













template <class DerivedT, typename EventT>
class AbstrastIOManager : public Singleton<DerivedT>, public ThreadedEventLoop<EventT>
{
protected:
	AbstrastIOManager() = default;
public:	
	virtual ~AbstrastIOManager() = default;
};


enum class EventType
{
	Downloading,
	Uploading,
	Remove,
	Enumeration,
	Quit
};

struct EventTypeSpecificData
{
	virtual ~EventTypeSpecificData() = default;
};

class Completer;
class Completion;

class IOManagerEvent
{
public:
	explicit IOManagerEvent(nullptr_t);

	template <class TYPE>
	IOManagerEvent(EventType eventType, TYPE&& specificData, Completer* completer, const Completion& completion);


	IOManagerEvent(IOManagerEvent&&) = default;
	IOManagerEvent& operator=(IOManagerEvent&&) = default;

	IOManagerEvent(const IOManagerEvent&) = delete;
	IOManagerEvent& operator=(const IOManagerEvent&) = delete;
public:
	EventType m_Type;
	unique_ptr<EventTypeSpecificData> m_SpecificData;
	Completer* m_Completer;
	Completion m_Completion;
};



////////////////////


struct CopyEvent
{
	CopyList downloadList;
};

struct RemoveEvent
{
	RemoveList removeList;
};

struct EnumEvent
{
	wstring path;
	RegularBackupList* lastBackupsList;
};


////////////////////////////////////////

template <>
void ThreadedEventLoop<IOManagerEvent>::ThreadMain();

template <>
void ThreadedEventLoop<IOManagerEvent>::SendQuit();


template <class TYPE>
IOManagerEvent::IOManagerEvent(EventType eventType, TYPE&& specificData, Completer* completer, const Completion& completion)
	: m_Type(eventType),
	m_SpecificData(new TYPE(forward<TYPE>(specificData))),
	m_Completer(completer),
	m_Completion(completion)
{
	static_assert(is_base_of<EventTypeSpecificData, TYPE>::value, "specific type must be derived from EventTypeSpecificData");
}