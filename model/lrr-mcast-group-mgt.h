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

#ifndef LRR_GROUP_MANAGEMENT_H_
#define LRR_GROUP_MANAGEMENT_H_

#include "ns3/ipv4-address.h"
#include <map>
#include <vector>
#include <set>

namespace ns3 {
namespace lrr {
/// Singleton operating with global group member list
class GlobalGroupManagement : public SimpleRefCount<GlobalGroupManagement>
{
public:
  typedef std::set<Ipv4Address> MemberSet;
  typedef std::map <Ipv4Address, MemberSet> GroupMap;
public:
  static GlobalGroupManagement * GetInstance ();
  /// Allocate next IP address. Number of addresses is less than 255
  static Ipv4Address AllocateMulticastAddress ();
  static void Destroy ();
  ~GlobalGroupManagement ();
  /// Add a new multicast group. If exists: update (OVERWRITE!) member list
  void AddMcastGroup (Ipv4Address mcastGrAddr, MemberSet memberList);
  /// Delete multicast group by multicast address:
  void RemoveMulticastgroup (Ipv4Address mcastAddress);
  /// Check that address is in multicast group
  bool IsInMcastGroup (Ipv4Address address, Ipv4Address mcastGroupAddr) const;
  /// Get group members by multicast adddress:
  MemberSet GetGroupMembers (Ipv4Address mcastAddress) const;
  /// Get the whole group set (for global topology)
  GroupMap GetGroupMap () const;
private:
  /// Private singleton constructor:
  GlobalGroupManagement ();
private:
  GroupMap m_groups;
};

}
}  // namespace ns3::lrr

#endif // GLOBAL_GROUP_MANAGEMENT_H_
