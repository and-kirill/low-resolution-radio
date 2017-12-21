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
 * Authors: Pavel Boyko <boyko@telum.ru>
 */
#ifndef LRR_ROUTINGPROTOCOL_H
#define LRR_ROUTINGPROTOCOL_H

#include "ns3/lrr-routing-dpd.h"
#include "ns3/lrr-routing-graph.h"
#include "ns3/node.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include <map>

namespace ns3
{
namespace lrr
{
/**
 * \ingroup lrr
 * 
 * \brief Global MANET routing protocol (per-node connection to global
 * graph)
 */
class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
  static TypeId GetTypeId (void);

  /// c-tor
  RoutingProtocol ();
  virtual ~RoutingProtocol();
  virtual void DoDispose ();

  ///\name Inherited from Ipv4RoutingProtocol
  ///\{
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const;
  ///\}
private:
  ///\name Handle output and input packets:
  ///\{
  Ptr<Ipv4Route> HandleUnicast (Ptr<const Packet> p, const Ipv4Header &header) const;
  Ptr<Ipv4Route> HandleMulticast (Ptr<const Packet> p, const Ipv4Header &header, const Ipv4Address srcAddress) const;
  ///\}
private:
  /// IP
  Ptr<Ipv4> m_ipv4;
  /// Local main address
  Ipv4Address m_local;
  /// Broadcast address
  Ipv4Address m_bcast;
  /// Handle duplicated broadcast/multicast packets
  DuplicatePacketDetection m_dpd;
private:
  /// Start protocol operation
  void Start ();
};

}
}
#endif /* LRR_ROUTINGPROTOCOL_H */
