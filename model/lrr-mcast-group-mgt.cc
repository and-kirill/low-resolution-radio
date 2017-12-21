/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Telum (www.telum.ru)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  Kirill Andreev <k.andreev@skoltech.ru>
 */

#include "lrr-mcast-group-mgt.h"
namespace ns3 {
namespace lrr {

static GlobalGroupManagement * g_instance = 0;

GlobalGroupManagement *
GlobalGroupManagement::GetInstance ()
{
  if (g_instance == 0)
    {
      g_instance = new GlobalGroupManagement ();
    }
  return g_instance;
}

Ipv4Address
GlobalGroupManagement::AllocateMulticastAddress ()
{
  static uint8_t lastOctet = 1;
  std::ostringstream os;
  NS_ASSERT (lastOctet < 0xff);
  os << "227.0.0." << (uint16_t) lastOctet;
  lastOctet++;
  return Ipv4Address (os.str ().c_str ());
}

void
GlobalGroupManagement::Destroy ()
{
  delete g_instance;
  g_instance = 0;
}

GlobalGroupManagement::GlobalGroupManagement ()
{
}

GlobalGroupManagement::~GlobalGroupManagement ()
{
}

void
GlobalGroupManagement::AddMcastGroup (Ipv4Address mcastGrAddr, MemberSet memberList)
{
  GroupMap::iterator i = m_groups.find (mcastGrAddr);
  if (i == m_groups.end ())
    {
      m_groups.insert (std::make_pair (mcastGrAddr, memberList));
      return;
    }
  i->second = memberList;
}

void
GlobalGroupManagement::RemoveMulticastgroup (Ipv4Address mcastGrAddr)
{
  GroupMap::iterator i = m_groups.find (mcastGrAddr);
  if (i != m_groups.end ())
    {
      m_groups.erase (i);
    }
}

bool
GlobalGroupManagement::IsInMcastGroup (Ipv4Address address, Ipv4Address mcastGrAddr) const
{
  GroupMap::const_iterator i = m_groups.find (mcastGrAddr);
  if (i != m_groups.end ())
    {
      return (i->second.find (address) != i->second.end ());
    }
  return false;
}

GlobalGroupManagement::MemberSet
GlobalGroupManagement::GetGroupMembers (Ipv4Address mcastGrAddr) const
{
  GroupMap::const_iterator i = m_groups.find (mcastGrAddr);
  if (i != m_groups.end ())
    {
      return i->second;
    }
  return MemberSet ();
}

GlobalGroupManagement::GroupMap
GlobalGroupManagement::GetGroupMap () const
{
  return m_groups;
}

}
}  // namespace ns3::lrr
