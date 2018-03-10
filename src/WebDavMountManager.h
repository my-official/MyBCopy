#pragma once
#include "Utils.h"

class WebDavMount;


class MyBCopy_API WebDavMountManager : public Singleton<WebDavMountManager>
{
public:	
	WebDavMount Mount(const wstring& address, const wstring& username, const wstring& password);	
	void EraseUnused();
	

	static const size_t MaxMounts;
private:
	friend class WebDavMount;

	class MountPointDesc
	{
	public:
		MountPointDesc(MountPointDesc&&) noexcept = default;
		MountPointDesc& operator=(MountPointDesc&&) noexcept = default;

		MountPointDesc(const MountPointDesc&) = delete;
		MountPointDesc& operator=(const MountPointDesc&) = delete;
		~MountPointDesc();

		size_t  m_UseCounter;
		wstring m_Address;
		wstring m_MountPoint;
		MountPointDesc(const wstring& address, const wstring& username, const wstring& password);
		void Unmount();
		
	};

	mutex				 m_UseCountersMutex;
	map<wstring, MountPointDesc> m_UseCounters;

	void IncConter(const wstring& address);
	void Release(const wstring& address) noexcept;
	void DoEraseUnused();	
};





class MyBCopy_API WebDavMount
{
public:
	WebDavMount(WebDavMountManager* owner, const wstring& address, const wstring& mountPoint);
	WebDavMount(WebDavMount&&) noexcept = default;
	WebDavMount& operator=(WebDavMount&&) noexcept = default;

	WebDavMount(const WebDavMount&) = delete;
	WebDavMount& operator=(const WebDavMount&) = delete;

	virtual ~WebDavMount();

	std::wstring MountPoint() const;
private:
	WebDavMountManager* m_Owner;
	wstring m_Address;
	wstring m_MountPoint;
};
