/*! 
Packet for the EpServerEngine

The MIT License (MIT)

Copyright (c) 2012-2013 Woong Gyu La <juhgiyo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "epPacket.h"

#if defined(_DEBUG) && defined(EP_ENABLE_CRTDBG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // defined(_DEBUG) && defined(EP_ENABLE_CRTDBG)

using namespace epse;

Packet::Packet(const void *packet, unsigned int byteSize, bool shouldAllocate, epl::LockPolicy lockPolicyType):SmartObject(lockPolicyType)
{
	m_packet=NULL;
	m_packetSize=0;
	m_isAllocated=shouldAllocate;
	if(shouldAllocate)
	{
		if(byteSize>0)
		{
			m_packet=EP_NEW char[byteSize];
			if(packet)
				epl::System::Memcpy(m_packet,packet,byteSize);
			else
				epl::System::Memset(m_packet,0,byteSize);
			m_packetSize=byteSize;
		}
	}
	else
	{
		m_packet=reinterpret_cast<char*>(const_cast<void*>(packet));
		m_packetSize=byteSize;
	}
	m_lockPolicy=lockPolicyType;
	switch(lockPolicyType)
	{
	case epl::LOCK_POLICY_CRITICALSECTION:
		m_packetLock=EP_NEW epl::CriticalSectionEx();
		break;
	case epl::LOCK_POLICY_MUTEX:
		m_packetLock=EP_NEW epl::Mutex();
		break;
	case epl::LOCK_POLICY_NONE:
		m_packetLock=EP_NEW epl::NoLock();
		break;
	default:
		m_packetLock=NULL;
		break;
	}
}

Packet::Packet(const Packet& b):SmartObject(b)
{
	m_lockPolicy=b.m_lockPolicy;
	switch(m_lockPolicy)
	{
	case epl::LOCK_POLICY_CRITICALSECTION:
		m_packetLock=EP_NEW epl::CriticalSectionEx();
		break;
	case epl::LOCK_POLICY_MUTEX:
		m_packetLock=EP_NEW epl::Mutex();
		break;
	case epl::LOCK_POLICY_NONE:
		m_packetLock=EP_NEW epl::NoLock();
		break;
	default:
		m_packetLock=NULL;
		break;
	}

	LockObj lock(b.m_packetLock);
	m_packet=NULL;
	if(b.m_isAllocated)
	{
		if(b.m_packetSize>0)
		{
			m_packet=EP_NEW char[b.m_packetSize];
			epl::System::Memcpy(m_packet,b.m_packet,b.m_packetSize);
		}
		m_packetSize=b.m_packetSize;
	}
	else
	{
		m_packet=b.m_packet;
		m_packetSize=b.m_packetSize;
	}
	m_isAllocated=b.m_isAllocated;
	
}
Packet & Packet::operator=(const Packet&b)
{
	if(this!=&b)
	{
		resetPacket();

		SmartObject::operator =(b);

		m_lockPolicy=b.m_lockPolicy;
		switch(m_lockPolicy)
		{
		case epl::LOCK_POLICY_CRITICALSECTION:
			m_packetLock=EP_NEW epl::CriticalSectionEx();
			break;
		case epl::LOCK_POLICY_MUTEX:
			m_packetLock=EP_NEW epl::Mutex();
			break;
		case epl::LOCK_POLICY_NONE:
			m_packetLock=EP_NEW epl::NoLock();
			break;
		default:
			m_packetLock=NULL;
			break;
		}

		LockObj lock(b.m_packetLock);
		m_packet=NULL;
		if(b.m_isAllocated)
		{
			if(b.m_packetSize>0)
			{
				m_packet=EP_NEW char[b.m_packetSize];
				epl::System::Memcpy(m_packet,b.m_packet,b.m_packetSize);
			}
			m_packetSize=b.m_packetSize;
		}
		else
		{
			m_packet=b.m_packet;
			m_packetSize=b.m_packetSize;
		}
		m_isAllocated=b.m_isAllocated;

	}
	return *this;
}
void Packet::resetPacket()
{
	m_packetLock->Lock();
	if(m_isAllocated && m_packet)
	{
		EP_DELETE[] m_packet;
	}
	m_packet=NULL;
	m_packetLock->Unlock();
	if(m_packetLock)
		EP_DELETE m_packetLock;
	m_packetLock=NULL;
}

Packet::~Packet()
{
	resetPacket();
}

unsigned int Packet::GetPacketByteSize() const
{
	return m_packetSize;
}

const char *Packet::GetPacket() const
{
	return m_packet;	
}

void Packet::SetPacket(const void* packet, unsigned int packetByteSize)
{
	epl::LockObj lock(m_packetLock);
	if(m_isAllocated)
	{
		if(m_packet)
			EP_DELETE[] m_packet;
		m_packet=NULL;
		if(packetByteSize>0)
		{
			m_packet=EP_NEW char[packetByteSize];
			EP_ASSERT(m_packet);
		}
		if(packet)
			epl::System::Memcpy(m_packet,packet,packetByteSize);
		else
			epl::System::Memset(m_packet,0,packetByteSize);
		m_packetSize=packetByteSize;

	}
	else
	{
		m_packet=reinterpret_cast<char*>(const_cast<void*>(packet));
		m_packetSize=packetByteSize;
	}
}