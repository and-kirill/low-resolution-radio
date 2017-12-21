/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Telum (www.telum.ru)
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
 * Author: Kirill Andreev <k.andreev@skoltech.ru>
 */
#ifndef LRR_GLOBAL_PEERING_H_
#define LRR_GLOBAL_PEERING_H_

#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include <map>

#include "ns3/lrr-device.h"

namespace ns3
{
namespace lrr
{
/**
 * \brief Implements Peering management function. Translates interface IP-addresses to main IP address of node
 * (main address is the address of rirst radio-interface). When constructing routes, it calculates next-hop interface
 * address from nest-hop main address, because global routing graph keeps only main interfaces
 */
class GlobalPeering : public Object
{
public:
  GlobalPeering ();
  static TypeId GetTypeId ();
  /**
   * \brief Every link is represented by:
   * 1. Source and destination main addresses
   * 2. Source and destination interface addresses (needed by forwarding mechanism)
   * 3. Metric of the link.
   */
  struct Link
  {
    Ipv4Address srcMainAddress;
    Ipv4Address dstMainAddress;
    Ipv4Address srcIfaceAddress;
    Ipv4Address dstIfaceAddress;
    uint16_t metric;
    Link (Ipv4Address LRR, Ipv4Address IS, Ipv4Address MD, Ipv4Address ID, uint16_t metric) :
      srcMainAddress (LRR),
      dstMainAddress (MD),
      srcIfaceAddress (IS),
      dstIfaceAddress (ID),
      metric (metric)
    {};
  };
  /// For each address we have outgoing links: destination interface and the best metric
  typedef std::map <Ipv4Address, std::map <Ipv4Address, uint16_t> > Topology;
public:
  /// Clear all internal structures including registered nodes
  void Clear ();
  /// Register a node:
  void AddVertex (Ptr<Node> node);
  /// Create all possible links using neighbors obtained from PHY layer
  void CreateLinks ();
  /// Get IP address of first radio interface (i.e. main address)
  static Ipv4Address GetMainAddress (Ptr<Node> node);
  /// Convert interface address to main address:
  Ipv4Address MainFromIfaceAddress (Ipv4Address interfaceAddress);
  /// Get all links:
  Topology GetTopology () const;
  /// Get links between two nodes, addressed by main address:
  std::vector<Link> GetLinks (Ipv4Address srcMainAddress, Ipv4Address dstMainAddress) const;
private:
  /// Link-set is kept in the following format: (key is src main address (first) and dst main address (second)) 
  typedef std::map <std::pair <Ipv4Address, Ipv4Address>, std::vector<Link> > LinkSet;
  /// Const iterator:
  typedef LinkSet::const_iterator LinkSetConstIterator;
  /// Iterator:
  typedef LinkSet::iterator LinkSetIterator;
  /// Description of interface: pointer to radio device and interface IP address.
  struct Interface
  {
    Ipv4Address address;
    Ptr<NeighborAwareDevice> device;
    Interface (Ipv4Address addr, Ptr<NeighborAwareDevice> dev) :
      address (addr),
      device (dev)
    {};
  };
private:
  /// Obtain list of radio interfaces from node:
  static std::vector<Interface> GetRadioInterfaces (Ptr<Node> node);
  /// Add a new link:
  void InsertLink (Link link);
  /// Choose the best link between a given pair of nodes:
  void UpdateTopology (Link link);
  /// Create all links, which start from a given node:
  void CreateOutgoingLinks (const Ptr<Node> node);
  ///\name Internal utilities:
  ///\{
  /// Obtain neighbors of a given interface:
  std::vector<Ipv4Address> GetIfaceNeighbors (Ptr<NetDevice> device);
  /// Obtain IP address from pointer to device:
  static Ipv4Address InterfaceAddrFromDevice (Ptr<NetDevice> device);
  ///\}
  void UpdateLinkStatus (const Topology& oldTopology) const;
private:
  /// IP address mapping:
  std::map <Ipv4Address, Ptr<Node> > m_addressToNodeMap;
  /// Nodes involved:
  std::vector <Ptr<Node> > m_registredNodes;
  /// Links:
  LinkSet m_globalLinkSet;
  /// Topology, represented in convenient form for GlobalTopology:
  Topology m_topology;
  /// New neighbor found. Arguments: interface addresses
  TracedCallback <Ipv4Address, Ipv4Address> m_linkOpen;
  /// Neighbor lost. Arguments: interface addresses
  TracedCallback <Ipv4Address, Ipv4Address> m_linkClosed;
};

} //lrr
} //ns3

#endif
