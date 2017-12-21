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
#include "lrr-routing-protocol.h"
#include "lrr-routing-peering.h"
#include "lrr-mcast-group-mgt.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/ipv4-route.h"
#include <algorithm>
#include <limits>

NS_LOG_COMPONENT_DEFINE ("LrrRoutingProtocol");

namespace ns3 {
namespace lrr {
NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

RoutingProtocol::RoutingProtocol () :
  m_dpd (DuplicatePacketDetection (Seconds (10)))
{
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::lrr::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .AddConstructor<RoutingProtocol> ()
  ;
  return tid;
}

RoutingProtocol::~RoutingProtocol ()
{
  m_ipv4 = 0;
}

void
RoutingProtocol::DoDispose ()
{
  m_ipv4 = 0;
}

void
RoutingProtocol::Start ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

Ptr<Ipv4Route>
RoutingProtocol::HandleMulticast (Ptr<const Packet> p, const Ipv4Header &header, Ipv4Address src) const
{
  NS_LOG_FUNCTION (this << src << header);
  Ipv4Address dst = header.GetDestination ();
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetDestination (dst);
  route->SetGateway (m_bcast);
  route->SetSource (src);
  // Outgoing interface is not needed for multicast. Multicast is sent to all interfaces by IP
  route->SetOutputDevice (m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (m_local)));
  return route;
}

Ptr<Ipv4Route>
RoutingProtocol::HandleUnicast (Ptr<const Packet> p, const Ipv4Header &header) const
{
  NS_LOG_FUNCTION (this << header);
  Ipv4Address dst = header.GetDestination ();
  std::pair<Ipv4Address, Ipv4Address> ifacePair = GlobalGraph::Instance ()->GetUnicastRoute (m_local, dst);
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetDestination (dst);
  route->SetGateway (ifacePair.second);
  route->SetSource (ifacePair.first);
  route->SetOutputDevice (m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (ifacePair.first)));
  NS_LOG_DEBUG ("ROUTING: Sending unicast packet " << p->GetUid () << " from " << m_local << ", destination " << dst << ", gateway is " << route->GetGateway ());
  return route;
}

Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  Ipv4Address dst = header.GetDestination ();
  NS_LOG_FUNCTION (this << dst);
  if (dst.IsMulticast () && GlobalGraph::Instance ()->HaveMcastPath (m_local, dst))
    {
      m_dpd.PrepareTx (p);
      return HandleMulticast (p, header, m_local);
    }
  if ((!dst.IsMulticast ()) && GlobalGraph::Instance ()->HavePath (m_local, dst))
    {
      return HandleUnicast (p, header);
    }
  sockerr = Socket::ERROR_NOROUTETOHOST;
  NS_LOG_DEBUG ("ROUTING:" << this << "No route to host " << dst);
  return 0;
}

bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header,
                             Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                             MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p->GetUid () << header.GetDestination () << idev->GetAddress ());
  NS_ASSERT (m_ipv4 != 0);
  NS_ASSERT (p != 0);
  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice (idev);

  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();
  if (m_local == origin)
    {
      NS_LOG_DEBUG ("ROUTING: " << m_local << " drop my own packet");
      return true;
    }
  // Multicast local delivery/forwarding
  if (dst.IsMulticast ())
    {
      if (m_dpd.IsDuplicate (p, header))
        {
          NS_LOG_DEBUG ("ROUTING: Duplicated packet " << p->GetUid () << " from " << origin << ". Drop.");
          return true;
        }
      Ptr<Packet> packet = p->Copy ();
      if (GlobalGroupManagement::GetInstance ()->IsInMcastGroup (m_local, dst))
        {
          NS_LOG_LOGIC ("Multicast local delivery to " << m_local);
          lcb (p, header, iif);
        }
      if (header.GetTtl () > 1)
        {
          if (GlobalGraph::Instance ()->IsMcastRetranslator (m_local, origin, dst))
            {
              // Find outgoing interfaces:
              std::set<Ipv4Address> interfaces = GlobalGraph::Instance ()->GetMulticastRoute (origin, header.GetDestination (), m_local);
              for (std::set<Ipv4Address>::iterator i =  interfaces.begin (); i != interfaces.end (); i++)
                {
                  Ipv4Address dst = header.GetDestination ();
                  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
                  route->SetDestination (dst);
                  route->SetGateway (m_bcast);
                  route->SetSource (origin);
                  route->SetOutputDevice (m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (*i)));
                  NS_LOG_DEBUG ("ROUTING: Forward multicast packet " << p->GetUid () << " from " << origin << ", destination " << dst << ", outgoing interface is " << *i);
                  ucb (route, packet->Copy (), header);
                }
            }
        }
      else
        {
          NS_LOG_DEBUG ("ROUTING: TTL exceeded. Drop packet " << p->GetUid ());
        }
      return true;
    }

  // Unicast local delivery
  if (GlobalGraph::Instance ()->IsLocalAddress (dst, m_local))
    {
      NS_LOG_DEBUG ("ROUTING: Unicast local delivery to " << dst << " packet " << p->GetUid ());
      lcb (p, header, iif);
      return true;
    }
  NS_ASSERT_MSG (m_ipv4->GetInterfaceForAddress (dst) == -1, "packet addresses to local node was not delivered!");
  // Forwarding
  if (GlobalGraph::Instance ()->HavePath (m_local, dst))
    {
      Ptr<Ipv4Route> route = HandleUnicast (p, header);
      NS_LOG_DEBUG ("ROUTING: Forward unicast packet " << p->GetUid () << " from " << origin << ", destination " << dst << ", gateway is " << route->GetGateway ());
      ucb (route, p, header);
      return true;
    }
  return false;
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_ASSERT (ipv4 != 0);
  NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4;
  Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

void
RoutingProtocol::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());
  Ipv4Address loopback ("127.0.0.1");
  if ( m_ipv4->GetAddress (i, 0).GetLocal () != loopback)
    {
      m_local = m_ipv4->GetAddress (i, 0).GetLocal ();
      m_bcast = m_ipv4->GetAddress (i, 0).GetBroadcast ();
    }
}

void
RoutingProtocol::NotifyInterfaceDown (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());
}

void
RoutingProtocol::NotifyAddAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << " interface " << i << " address " << address);
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this);
}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
}

}
}
