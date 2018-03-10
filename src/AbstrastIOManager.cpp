#include "MyBCopy_pch.h"
#include "AbstrastIOManager.h"

IOManagerEvent::IOManagerEvent(nullptr_t) : m_Type(EventType::Quit), m_Completion(Completion::CreateEmpty())
{

}



template <>
void ThreadedEventLoop<IOManagerEvent>::ThreadMain()
{
	try
	{

		bool quit = false;

		do
		{
			decltype(m_Events) events;
			{
				unique_lock<std::mutex> lck(m_EventsMutex);

				while (m_Events.empty())
				{
					m_EventsCV.wait(lck);
				}

				m_Events.swap(events);
			}

			do
			{
				IOManagerEvent& event = events.front();

				if (event.m_Type != EventType::Quit)
				{
					try
					{
						if (!HandleEvent(event))
						{
							quit = true;
						}
					}
					catch (...)
					{
						event.m_Completion.m_ExceptionPtr = std::current_exception();
					}
					event.m_Completer->CompleteSignaled(event.m_Completion);
				}
				else
				{
					quit = true;
				}

				events.pop();
			} while (!events.empty());
		} while (!quit);

	}	
	catch (...)
	{
		
	}

}



template <>
void ThreadedEventLoop<IOManagerEvent>::SendQuit()
{
	IOManagerEvent e(nullptr);
	PushEvent(move(e));
}