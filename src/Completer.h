#pragma once

struct BackupJob;
class Storage;

class Completion
{
public:
	virtual ~Completion() = default;

	BackupJob* job;
	Storage* storage;

	void RethrowException();
//private:
//	friend class Completer;
	exception_ptr m_ExceptionPtr;
};


class Completer
{
public:
	void CompleteSignaled(const Completion& completion);
	bool WaitForComplete(Completion& completion);	
	void RegisterWait();
private:
	condition_variable		m_CompletedQueueCV;
	mutex					m_CompletedQueueMutex;
	queue< Completion >		m_CompletedQueue;

	size_t					m_NumWaits = 0;

};


//
//
//class CompleterBase
//{
//public:
//	virtual ~CompleterBase() = default;
//};
//
//
//template <class Type>
//class Completer : public CompleterBase
//{
//public:
//	void CompleteSignaled(Type job);
//	Type WaitForComplete();
//private:
//	condition_variable		m_CompletedQueueCV;
//	mutex					m_CompletedQueueMutex;
//	queue< Type >			m_CompletedQueue;
//};
//
//
//
//
//template <class Type>
//void Completer<Type>::CompleteSignaled(Type job)
//{
//	unique_lock<std::mutex> lck(m_CompletedQueueMutex);
//	m_CompletedQueue.push(job);
//	m_CompletedQueueCV.notify_one();
//}
//
//template <class Type>
//Type Completer<Type>::WaitForComplete()
//{
//	unique_lock<std::mutex> lck(m_CompletedQueueMutex);
//
//	while (m_CompletedQueue.empty())
//	{
//		m_CompletedQueueCV.wait(lck);
//	}
//
//	Type result = m_CompletedQueue.back();
//	m_CompletedQueue.pop();
//	return move(result);
//}
