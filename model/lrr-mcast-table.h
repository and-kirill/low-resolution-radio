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

#ifndef GLOBALMCASTTABLE_H_
#define GLOBALMCASTTABLE_H_

#include "ns3/ipv4-address.h"
#include <map>
#include <set>

namespace ns3 {
namespace lrr {

/**
 * \brief Keeps a global set of multicast trees and derives receiver set and translator set.
 * Is a part of GlobalGraph singleton.
 */
class GlobalMcastTable : public SimpleRefCount<GlobalMcastTable>
{
public:
  GlobalMcastTable ();
  ~GlobalMcastTable ();

  /**
   *  \brief Add set of stations with addresses \param memberList to multicast
   *  group with multicast address \param mcastGrAddr.
   */
  void AddMcastGroup (const Ipv4Address & mcastGrAddr, const std::set<Ipv4Address>& memberList);
  /**
   * \brief Test if station with address \param local is retranslator for multicast
   * traffic from address \param from to multicast address \param mcastTo.
   *
   * \return false if:
   *       - there is no mcastGroup with address \param mcastTo;
   *       - address \param from is not in mcastGroup \param mcastTo;
   *       - address \param local is equal to address \param from.
   */
  bool IsMcastRetranslator (const Ipv4Address & local, const Ipv4Address & from, const Ipv4Address & mcastTo) const;
  /// Description see at the same method of GlobalGraph
  std::set<Ipv4Address> GetMulticastRoute (const Ipv4Address & from, const Ipv4Address & mcastTo, const Ipv4Address & local) const;
  /// Update multicast table. Called after graph update
  void Update ();
private:
  ///\name Multicast forwarding table: Each node must know about nexthop set of each multicast tree, identified by (source unicast, dts multicast pair)
  ///\{
  /// Forwarding table tree ID consists of group ID multicast address (first) and source address  (tree root)
  typedef std::pair<Ipv4Address, Ipv4Address> ForwardingTableTreeId;
  /// Tree id and retranslator address form a forwarding entry ID
  typedef std::pair <Ipv4Address, ForwardingTableTreeId> ForwardingEntryId;
  /// A set of source outgoing interfaces for each forwarding entry ID
  typedef std::map <ForwardingEntryId, std::set<Ipv4Address> > MulticastForwardingTable;
  ///\}
  /**
   * \brief Update multicast group. Iterate throug all group members and construct trees starting at
   * each group member with all the rest group members as leafs.
   * \param groupId is a multicast address identifying a group
   * \param members is a member set
   */
  void UpdateMulticastGroup (const Ipv4Address & groupId, const std::set<Ipv4Address> & members);
  /**
   * \brief Create multicast tree for specified root (group member)
   * \param groupId is a multicast address identifying a group
   * \param srcAddress is a root of the tree (group member)
   * \param groupMembers leafs of the tree (group members)
   * \return a set of retranslators
   */
  void UpdateMulticastTree (const Ipv4Address & groupId, const Ipv4Address & srcAddress, const std::set<Ipv4Address> & groupMembers);
  /**
   * \brief Create a tree brunch from one group member (root )to another (leaf)
   * \param groupId is a group, which a given tree belongs to
   * \param srcAddress is a root of the brunch
   * \param dstAddress is a leaf of the brunch
   */
  void UpdateMulticastBrunch (const Ipv4Address & groupId, const Ipv4Address & srcAddress, const Ipv4Address & dstAddress);
private:
  /// Multicast forwarding table gives a set of next-hop interfaces for each station inside a tree (tree ID is source and )
  MulticastForwardingTable m_multicastForwardingMap;
};

} // namespace lrr
} // namespace ns3

#endif // GLOBALMCASTTABLE_H_
