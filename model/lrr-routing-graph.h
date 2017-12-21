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
 */

#ifndef GLOBALGRAPH_H_
#define GLOBALGRAPH_H_

#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include <map>
#include <set>

#include "ns3/lrr-routing-peering.h"

namespace ns3
{
namespace lrr
{
class GlobalTopology;
class GlobalMcastTable;
/**
 * \ingroup lrr
 * \brief Implements all graph functionality: keeps global topology
 * information, keeps multicast routes, keeps all links.
 * 
 * This graph supports multiple IP wireless interfaces. Node is
 * identified by main address (like in OLSR)
 * 
 * Multicast tree is implemented as a concatenation of shortest paths
 * between a source and a group of destinations
 *
 * Implemented as a singleton + some functional sub-blocks
 *
 * To support mobility, all graph information is refreshed
 * periodically all over the world.
 */
class GlobalGraph
{
public:
  /// Access method to instance:
  static GlobalGraph* Instance ();
  /// Destroy all topology structures
  static void Destroy ();
  /// How often periodical update is triggered
  void SetUpdatePeriod (Time updatePeriod);
  ~GlobalGraph ();

protected:
  /// Constructor is protected
  GlobalGraph ();

public:
  /**
   * \brief Create graph vertices and start the updateEdges scheduler (which
   * will update graph one time per m_updatePeriod)
   * \attention must be called before Simulator::Run()
   */
  void Start ();
  /**
   * \brief Clear graph
   * \attention must be called after Simulator::Destroy()
   */
  void Stop ();
  // Debug method (for visualizing tests)
  void PrintGraph (std::ostream & os);

  /**
   * \return true if path from Interface with \param from address
   * to Interface with \param to address exists
   */
  bool HavePath (Ipv4Address from, Ipv4Address to);
  /**
   * \return distance (measured by link metrics) from Interface
   * with \param from address to Interfacewith \param to address
   */
  uint32_t PathDistance (Ipv4Address from, Ipv4Address to);
  /**
   * \return address of next Interface in path from Interface
   * with \param from address to Interface with \param to address,
   * return Ipv4Address() if path doesn't exist (!HavePath())
   */
  Ipv4Address GetNextHopMain (Ipv4Address from, Ipv4Address to);
  /**
   * \brief Get source and next hop interface addresses
   * \param from is the source address of the packet
   * \param to is a destination of a packet
   * \return a pair of <source, next hop> interface addresses
   */
  std::pair <Ipv4Address, Ipv4Address> GetUnicastRoute (Ipv4Address from, Ipv4Address to);
  /// Check if destination one of local interfaces
  bool IsLocalAddress (Ipv4Address dst, Ipv4Address localIfaceAddr) const;
  /**
   * \brief Get outgoing interfaces for a tree specifiied by:
   * \param from source address
   * \param mcastTo group, which this source belongs to
   * \param local is a local retranslator
   * \return a set of outgoing interfaces' IP addresses
   */
  std::set<Ipv4Address> GetMulticastRoute (Ipv4Address from, Ipv4Address mcastTo, Ipv4Address local) const;
  /**
   * \brief Have Src with address \param from path to any Dst
   * from group with multicast address \param mcastTo.
   */
  bool HaveMcastPath (Ipv4Address from, Ipv4Address mcastTo);
  /**
   * \brief Test if station with address \param local is retranslator for multicast
   * traffic from address \param from to multicast address \param mcastTo.
   */
  bool IsMcastRetranslator (Ipv4Address local, Ipv4Address from, Ipv4Address mcastTo);
  /// Get McastGroupIpv4Address for group with group ID \param grId
  Ipv4Address GetMcastIpAddr (uint16_t grId);
  /// Needed for topology monitoring:
  Ptr<GlobalPeering> GetGlobalPeering () const;
private:
  /// Create graph vertices (of all nodes in simulator)
  void CreateVertices ();
  /// Recreate graph edges
  void UpdateEdges ();
  /// Create all existing edges (between all nodes in simulator)
  void CreateEdges ();
  /// isStarted means that topology is constructed and periodical
  //update is started
  bool m_isStarted;
  /// Period for periodicall update
  Time m_updatePeriod;
  /// Update event:
  EventId m_updateEvent;
private:
  /// Instance:
  static GlobalGraph* _instance;
  /// Subcomponents:
  Ptr<GlobalTopology> m_graph;
  Ptr<GlobalPeering> m_peering;
  Ptr<GlobalMcastTable> m_mcastTable;
};

} // namespace lrr
} // namespace ns3

#endif // GLOBALGRAPH_H_
