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
 * Authors: Denis Fakhriev <fakhriev@telum.ru>
 *          Kirill Andreev <k.andreev@skoltech.ru>
 */

#include "lrr-mcast-table.h"
#include "lrr-mcast-group-mgt.h"
#include "lrr-routing-graph.h"

namespace ns3 {
namespace lrr {

using namespace std;

GlobalMcastTable::GlobalMcastTable ()
{
}

GlobalMcastTable::~GlobalMcastTable ()
{
}

void
GlobalMcastTable::Update ()
{
  GlobalGroupManagement::GroupMap groups = GlobalGroupManagement::GetInstance ()->GetGroupMap ();
  for (GlobalGroupManagement::GroupMap::iterator i = groups.begin (); i != groups.end (); i++)
    {
      UpdateMulticastGroup (i->first, i->second);
    }
}

void
GlobalMcastTable::UpdateMulticastGroup (const Ipv4Address & groupId, const set<Ipv4Address> & grMemberSet)
{
  for (set<Ipv4Address>::iterator i = grMemberSet.begin (); i != grMemberSet.end (); ++i)
    {
      UpdateMulticastTree (groupId, *i, grMemberSet);
    }
}

void
GlobalMcastTable::UpdateMulticastTree (const Ipv4Address & groupId, const Ipv4Address & srcAddress, const std::set<Ipv4Address> & groupMembers)
{
  for (set<Ipv4Address>::iterator i = groupMembers.begin (); i != groupMembers.end (); ++i)
    {
      if (*i == srcAddress)
        {
          continue;
        }
      if (!GlobalGraph::Instance ()->HavePath (srcAddress, *i))
        {
          continue;
        }
      UpdateMulticastBrunch (groupId, srcAddress, *i);
    }
}

void
GlobalMcastTable::UpdateMulticastBrunch (const Ipv4Address & groupId, const Ipv4Address & srcAddress,
                                         const Ipv4Address & dstAddress)
{
  Ipv4Address retr; // = GlobalGraph::Instance ()->GetNextHopMain (srcAddress, dstAddress);
  ForwardingTableTreeId treeId = std::make_pair (groupId, srcAddress);
  Ipv4Address current = srcAddress;
  do
    {
      NS_ASSERT (GlobalGraph::Instance ()->HavePath (current, dstAddress));
      retr = GlobalGraph::Instance ()->GetNextHopMain (current, dstAddress);
      // Update forwarding table (outgoing interfaces)
      ForwardingEntryId forwId = std::make_pair (current, treeId);
      MulticastForwardingTable::iterator forw_it = m_multicastForwardingMap.find (forwId);
      if (forw_it == m_multicastForwardingMap.end ())
        {
          m_multicastForwardingMap.insert (std::make_pair (forwId, std::set<Ipv4Address> ()));
          forw_it = m_multicastForwardingMap.find (forwId);
        }
      Ipv4Address outInterface = GlobalGraph::Instance ()->GetUnicastRoute (current, retr).first;
      forw_it->second.insert (outInterface);

      current = retr;
    }
  while (retr != dstAddress);
}

std::set<Ipv4Address>
GlobalMcastTable::GetMulticastRoute (const Ipv4Address& from, const Ipv4Address& mcastTo, const Ipv4Address& local) const
{
  ForwardingTableTreeId treeId = std::make_pair (mcastTo, from);
  ForwardingEntryId forwId = std::make_pair (local, treeId);
  MulticastForwardingTable::const_iterator forw_it = m_multicastForwardingMap.find (forwId);
  if (forw_it == m_multicastForwardingMap.end ())
    {
      return std::set<Ipv4Address> ();
    }
  return forw_it->second;
}

bool
GlobalMcastTable::IsMcastRetranslator (const Ipv4Address & local, const Ipv4Address & from, const Ipv4Address & mcastTo) const
{
  if (local == from)
    {
      // Can not be retranslator bacause a root of tree
      return false;
    }
  return (GetMulticastRoute (from, mcastTo, local).size () != 0);
}

}
}  // namespace ns3::lrr
