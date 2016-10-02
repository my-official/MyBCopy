#include "stdafx.h"
#include "Completer.h"

void Completion::RethrowException()
{
	if (m_ExceptionPtr)
		rethrow_exception(m_ExceptionPtr);
}

void Completer::CompleteSignaled(const Completion& completion)
{
	unique_lock<std::mutex> lck(m_CompletedQueueMutex);
	m_CompletedQueue.push(completion);
	m_CompletedQueueCV.notify_one();
}

bool Completer::WaitForComplete(Completion& completion)
{
	unique_lock<std::mutex> lck(m_CompletedQueueMutex);

	if (m_NumWaits == 0)
		return false;

	while (m_CompletedQueue.empty())
	{
		m_CompletedQueueCV.wait(lck);
	}

	completion = m_CompletedQueue.back();
	m_CompletedQueue.pop();
	--m_NumWaits;

	return true;
}

void Completer::RegisterWait()
{
	unique_lock<std::mutex> lck(m_CompletedQueueMutex);
	++m_NumWaits;
}
