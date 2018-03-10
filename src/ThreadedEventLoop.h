#pragma once






template <typename EventT>
class ThreadedEventLoop
{
public:
	ThreadedEventLoop();
	virtual ~ThreadedEventLoop();
	
	//void PushEvent(const EventT& event);	
	void PushEvent(EventT&& event);
protected:
	void StartEventLoop();
	virtual void ThreadMain();
	virtual bool HandleEvent(EventT& event) = 0;

	
private:
	condition_variable		m_EventsCV;
	mutex					m_EventsMutex;
	queue<EventT>			m_Events;
	thread					m_Thread;

	void Quit();
	void SendQuit();
};

template <typename EventT>
void ThreadedEventLoop<EventT>::StartEventLoop()
{
	if (m_Thread.joinable())
		return;		

	m_Thread = thread(&ThreadedEventLoop<EventT>::ThreadMain, this);
}

















//template <typename EventT>
//void ThreadedEventLoop<EventT>::ThreadMain()
//{
//	bool quit = false;
//
//	do
//	{
//		decltype(m_Events) events;
//		{
//			unique_lock<std::mutex> lck(m_EventsMutex);
//
//			while (m_Events.empty())
//			{
//				m_EventsCV.wait(lck);
//			}
//
//			m_Events.swap(events);
//		}
//
//		do
//		{
//			if (!HandleEvent(events.front()))
//			{
//				events.pop();
//			}
//			else
//			{
//				quit = true;
//			}
//
//		} while (events.empty());
//	} while (!quit);
//}



template <typename EventT>
ThreadedEventLoop<EventT>::ThreadedEventLoop()
{
	
}

template <typename EventT>
ThreadedEventLoop<EventT>::~ThreadedEventLoop()
{
	Quit();
}


template <typename EventT>
void ThreadedEventLoop<EventT>::Quit()
{
	if (!m_Thread.joinable())
		return;

	SendQuit();
	m_Thread.join();


}

//
//template <typename EventT>
//void ThreadedEventLoop<EventT>::PushEvent(const EventT& event)
//{
//	//unique_lock<std::mutex> lck(m_EventsMutex);
//	//m_Events.emplace(event);
//	//m_EventsCV.notify_one();
//}



template <typename EventT>
void ThreadedEventLoop<EventT>::PushEvent(EventT&& event)
{
	unique_lock<std::mutex> lck(m_EventsMutex);
	m_Events.emplace(forward<EventT>(event));
	m_EventsCV.notify_one();
}





