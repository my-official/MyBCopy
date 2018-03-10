#include "MyBCopy_pch.h"
#include "WebDavMountManager.h"
#include "UtilsMyBCopy.h"
#include "Utils.h"
#include "Exceptions.h"


const size_t WebDavMountManager::MaxMounts = 2;


WebDavMount WebDavMountManager::Mount(const wstring& address, const wstring& username, const wstring& password)
{
	unique_lock<mutex> lock(m_UseCountersMutex);

	auto it = m_UseCounters.find(address);
	if (it == m_UseCounters.end())
	{
		DoEraseUnused();

		MountPointDesc desc(address, username, password);
		WebDavMount result(this, address, desc.m_MountPoint);
		m_UseCounters.emplace(address, move(desc));
		return move(result);
	}
	else
	{
		MountPointDesc& desc = it->second;
		++desc.m_UseCounter;
		return move( WebDavMount(this, address, desc.m_MountPoint) );
	}
}



void WebDavMountManager::EraseUnused()
{
	unique_lock<mutex> lock(m_UseCountersMutex);
	DoEraseUnused();
}

void WebDavMountManager::IncConter(const wstring& address)
{
	unique_lock<mutex> lock(m_UseCountersMutex);

	auto it = m_UseCounters.find(address);

	assert(it != m_UseCounters.end());

	MountPointDesc& desc = it->second;

	assert(desc.m_UseCounter > 0);

	++desc.m_UseCounter;
}

void WebDavMountManager::Release(const wstring& address) noexcept
{
	unique_lock<mutex> lock(m_UseCountersMutex);

	auto it = m_UseCounters.find(address);

	assert(it != m_UseCounters.end());

	MountPointDesc& desc = it->second;

	assert(desc.m_UseCounter > 0);

	--desc.m_UseCounter;
}

void WebDavMountManager::DoEraseUnused()
{	
	//if (m_UseCounters.size() < MaxMounts)
	//	return;

	for (auto it = m_UseCounters.begin(); it != m_UseCounters.end(); )
	{
		if (it->second.m_UseCounter == 0)
		{
			it = m_UseCounters.erase(it);
		}
		else
		{
			++it;
		}
	}

	//if (m_UseCounters.size() >= MaxMounts)
	//{
	//	throw EXCEPTION( BaseException("bad alloc of new mount point") );
	//}
}

WebDavMountManager::MountPointDesc::MountPointDesc(const wstring& address, const wstring& username, const wstring& password) : m_UseCounter(1), m_Address(address)
{
	wstring mountPoint = GetFreeMountPoint();
	ExecuteProgram_except(L"net", L"use " + mountPoint + L" \"" + m_Address + L"\" /USER:\"" + username + L"\" \"" + password + L"\" /PERSISTENT:NO");
	m_MountPoint.swap(mountPoint);
}

void WebDavMountManager::MountPointDesc::Unmount()
{
	if (!m_MountPoint.empty())
		ExecuteProgram_except(L"net", L"use " + m_MountPoint + L" /DELETE /y");
}

WebDavMountManager::MountPointDesc::~MountPointDesc()
{
	try
	{
		Unmount();
	}
	catch (...)
	{
	}
}

WebDavMount::WebDavMount(WebDavMountManager* owner, const wstring& address, const wstring& mountPoint) : m_Owner(owner), m_Address(address), m_MountPoint(mountPoint)
{

}

WebDavMount::~WebDavMount()
{
	if (!m_MountPoint.empty())
		m_Owner->Release(m_Address);
}

std::wstring WebDavMount::MountPoint() const
{
	return m_MountPoint;
}
