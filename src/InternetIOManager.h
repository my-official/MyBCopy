#pragma once
#include "Reserver.h"
#include "BackupBase.h"
#include "BackupJobExecuter.h"
#include "AbstrastIOManager.h"


struct InternetEventData : public EventTypeSpecificData
{
	const WebDavStorage* storage;	
};




struct InternetCopyEvent : public CopyEvent, public InternetEventData
{
};
struct InternetRemoveEvent : public RemoveEvent, public InternetEventData
{
};
struct InternetEnumEvent : public EnumEvent, public InternetEventData
{
};

class WebDavMount;

class InternetIOThread : public ThreadedEventLoop<IOManagerEvent>
{
public:	
	InternetIOThread();
	virtual bool HandleEvent(IOManagerEvent& e) override;
protected:	
	void ProcessDownloading(InternetCopyEvent& event);
	void ProcessUploading(InternetCopyEvent& event);
	void ProcessRemoving(InternetRemoveEvent& event);
	void ProcessEnumeration(InternetEnumEvent& event);

	WebDavMount* m_Mount;	
	
};







class InternetIOManager : public Singleton<InternetIOManager>, public InternetIOThread
{
public:	
	InternetIOManager();

	bool GetIndependentUploadThread() const;
	void SetIndependentUploadThread(bool independent);
	void StartDownloadFiles(InternetCopyEvent&& event, Completer* completer, const Completion& completion);
	void StartUploadFiles(InternetCopyEvent&& event, Completer* completer, const Completion& completion);
	void StartRemoveFiles(InternetRemoveEvent&& event, Completer* completer, const Completion& completion);
	RegularBackupList EnumerateBackupFiles(const WebDavStorage* storage, const BackupBase& backup);
private:		
	mutable recursive_mutex				m_UploadingThreadMutex;
	unique_ptr< InternetIOThread > 		m_UploadingThread;	

};