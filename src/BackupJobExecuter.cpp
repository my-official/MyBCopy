#include "MyBCopy_pch.h"
#include "BackupJobExecuter.h"
#include "Reserver.h"
#include "Utils.h"
#include "UtilsMyBCopy.h"
#include "Completer.h"






void BackupJobExecuter::Execute()
{
	StartAllJobs();			

	Completion completion = Completion::CreateEmpty();
	
	while (m_Completer.WaitForComplete(completion))
	{						
		if (completion.m_ExceptionPtr)
		{			
			completion.m_CompletionError(completion.m_ExceptionPtr);
		}
		else
		{
			if (completion.m_CompletionFunc)
			{
				try
				{
					completion.m_CompletionFunc();
				}
				catch (...)
				{
					completion.m_CompletionError(std::current_exception());
				}
			}
		}
	}	

	m_Jobs.clear();
}

void BackupJobExecuter::StartAllJobs()
{
	for (auto& job : m_Jobs)
	{
		try
		{
			job->Start();
		}
		catch (...)
		{
			job->ExceptionHandler(std::current_exception());
		}
	}		
}



