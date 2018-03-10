#pragma once

#include "BackupBase.h"
#include "Storage.h"
#include "Completer.h"
#include "Utils.h"
#include "ParsedSources.h"
#include "TraversedBackup.h"






class BaseJob
{
public:
	//	BaseJob(const wstring& tmpDirPath, const wstring& backupSubDirName = L"");
	BaseJob() = default;
	BaseJob(const BaseJob& rhs) = default;
	BaseJob(BaseJob&& rhs) = default;
	BaseJob& operator=(const BaseJob& rhs) = default;
	BaseJob& operator=(BaseJob&& rhs) = default;
	virtual ~BaseJob() = default;


	virtual void Start() = 0;
	virtual void ExceptionHandler(exception_ptr& exceptionPtr) = 0;

};



class BackupJobExecuter
{
public:
	void Execute();		
protected:
	list< shared_ptr<BaseJob> > m_Jobs;
	Completer m_Completer;
private:
	void StartAllJobs();
};


