#include "MyBCopy_pch.h"
#include "Completer.h"

Completion::Completion(const CompletionFunc& callbackFunction, const CompletionError& errorFunction)
	: m_CompletionFunc(callbackFunction)
	, m_CompletionError(errorFunction)
{

}

void Completion::RethrowException()
{
	if (m_ExceptionPtr)
		rethrow_exception(m_ExceptionPtr);
}

Completion Completion::CreateEmpty()
{
	return Completion();
}

void Completer::CompleteSignaled(const Completion& completion)
{
	unique_lock<std::mutex> lck(m_CompletedQueueMutex);
	m_CompletedQueue.push(completion);
	m_CompletedQueueCV.notify_one();
}

bool Completer::WaitForComplete(Completion& completion)
{
	if (m_NumWaits == 0)
		return false;

	{
		unique_lock<std::mutex> lck(m_CompletedQueueMutex);

		while (m_CompletedQueue.empty())
		{
			m_CompletedQueueCV.wait(lck);
		}

		completion = m_CompletedQueue.front();
		m_CompletedQueue.pop();
	}
	--m_NumWaits;

	return true;
}

void Completer::RegisterWait()
{	
	++m_NumWaits;
}

unsigned int Completer::NumRegisterWaits() const
{
	return m_NumWaits;
}
