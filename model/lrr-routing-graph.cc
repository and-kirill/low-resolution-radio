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

#include "lrr-routing-graph.h"
#include "lrr-mcast-group-mgt.h"
#include "lrr-routing-topology.h"
#include "lrr-mcast-table.h"

#include "ns3/node-container.h"
#include "ns3/ipv4.h"
#include "ns3/simulator.h"
#include <fstream>
namespace ns3 {
namespace lrr {

GlobalGraph* GlobalGraph::_instance = 0;

GlobalGraph*
GlobalGraph::Instance ()
{
  if (_instance == 0)
    {
      _instance = new GlobalGraph;
    }
  return _instance;
}

void
GlobalGraph::Destroy ()
{
  delete _instance;
  _instance = 0;
}

void
GlobalGraph::SetUpdatePeriod (Time updatePeriod)
{
  m_updatePeriod = updatePeriod;
}

GlobalGraph::GlobalGraph () :
  m_isStarted (false),
  m_updatePeriod (Seconds (1)),
  m_graph (Create<GlobalTopology> ()),
  m_peering (CreateObject <GlobalPeering> ()),
  m_mcastTable (Create<GlobalMcastTable> ())
{
}

GlobalGraph::~GlobalGraph ()
{
  Stop ();
  m_graph = 0;
  m_peering = 0;
  m_mcastTable = 0;
}

void
GlobalGraph::Start (void)
{
  NS_ASSERT (!m_isStarted);
  m_isStarted = true;
  CreateVertices ();
  UpdateEdges ();
}

void
GlobalGraph::Stop (void)
{
  m_graph->Clear ();
  m_peering->Clear ();
  m_updateEvent.Cancel ();
  m_isStarted = false;
}

void
GlobalGraph::UpdateEdges (void)
{
  CreateEdges ();
  m_updateEvent = Simulator::Schedule (m_updatePeriod, &GlobalGraph::UpdateEdges, this);
}

void
GlobalGraph::CreateVertices (void)
{
  NodeContainer allNodes = NodeContainer::GetGlobal ();
  for (NodeContainer::Iterator i = allNodes.Begin (); i != allNodes.End (); ++i)
    {
      Ipv4Address mainAddress = GlobalPeering::GetMainAddress (*i);
      m_graph->AddVertex (mainAddress);
      m_peering->AddVertex (*i);
    }
}

void
GlobalGraph::CreateEdges (void)
{
  bool updated (false);
  m_peering->CreateLinks ();
  GlobalPeering::Topology topology = m_peering->GetTopology ();
  for (GlobalPeering::Topology::iterator topology_it = topology.begin (); topology_it != topology.end (); topology_it++)
    {
      updated |= m_graph->SetEdges (topology_it->first, topology_it->second);
    }

  if (!updated)
    {
      return;
    }

  m_graph->FloydWarshal ();
  if (m_mcastTable != 0)
    {
      m_mcastTable->Update ();
    }
}

void
GlobalGraph::PrintGraph (std::ostream & os)
{
  m_graph->Print (os);
}

bool
GlobalGraph::HavePath (Ipv4Address from, Ipv4Address to)
{
  NS_ASSERT (!(from.IsMulticast () || to.IsMulticast ()));
  if (m_peering->MainFromIfaceAddress (from) == m_peering->MainFromIfaceAddress (to))
    {
      return false;
    }
  return m_graph->HavePath (m_peering->MainFromIfaceAddress (from),m_peering->MainFromIfaceAddress (to));
}

uint32_t
GlobalGraph::PathDistance (Ipv4Address from, Ipv4Address to)
{
  return m_graph->PathDistance (m_peering->MainFromIfaceAddress (from),m_peering->MainFromIfaceAddress (to));
}

std::pair <Ipv4Address, Ipv4Address>
GlobalGraph::GetUnicastRoute (Ipv4Address from, Ipv4Address to)
{
  Ipv4Address nextHopMain = GetNextHopMain (from, to);
  // From is local adderess rather than source, so we check link between from and nextHopMain address:
  std::vector<GlobalPeering::Link> links = m_peering->GetLinks (m_peering->MainFromIfaceAddress (from), nextHopMain);
  std::vector<GlobalPeering::Link>::const_iterator bestLink = links.end ();
  uint32_t bestMetric = 0xffffffff;
  for (std::vector<GlobalPeering::Link>::const_iterator i = links.begin (); i != links.end (); i++)
    {
      if (i->metric < bestMetric)
        {
          bestMetric = i->metric;
          bestLink = i;
        }
    }
  if (bestLink != links.end ())
    {
      return std::make_pair (bestLink->srcIfaceAddress, bestLink->dstIfaceAddress);
    }
  NS_FATAL_ERROR ("No link! from " << from << "(, main = " << m_peering->MainFromIfaceAddress (from)<<"), to = " << to << "(, main = " << m_peering->MainFromIfaceAddress (to) << "), next hop main = " << nextHopMain);
  return std::make_pair (Ipv4Address (), Ipv4Address ());
}

bool
GlobalGraph::IsLocalAddress (Ipv4Address dst, Ipv4Address localIfaceAddr) const
{
  return (m_peering->MainFromIfaceAddress (dst) == m_peering->MainFromIfaceAddress (localIfaceAddr));
}

std::set<Ipv4Address>
GlobalGraph::GetMulticastRoute (Ipv4Address from, Ipv4Address mcastTo, Ipv4Address local) const
{
  return m_mcastTable->GetMulticastRoute (m_peering->MainFromIfaceAddress (from), mcastTo, m_peering->MainFromIfaceAddress (local));
}

Ipv4Address
GlobalGraph::GetNextHopMain (Ipv4Address from, Ipv4Address to)
{
  return m_graph->GetNextHop (m_peering->MainFromIfaceAddress (from),m_peering->MainFromIfaceAddress (to));
}

bool
GlobalGraph::HaveMcastPath (Ipv4Address local, Ipv4Address mcastTo)
{
  std::set <Ipv4Address>  members = GlobalGroupManagement::GetInstance ()->GetGroupMembers (mcastTo);
  Ipv4Address localMain = m_peering->MainFromIfaceAddress (local);
  /// Not a member->have no path
  if (members.find (localMain) == members.end ())
    {
      return false;
    }
  /// I am a member of group: check path to at least one other member:
  for (std::set<Ipv4Address>::const_iterator i = members.begin (); i != members.end (); ++i)
    {
      if (*i == localMain)
        {
          continue;
        }
      if (GlobalGraph::Instance ()->HavePath (localMain, *i))
        {
          return true;
        }
    }
  return false;
}

bool
GlobalGraph::IsMcastRetranslator (Ipv4Address local, Ipv4Address from, Ipv4Address mcastTo)
{
  return m_mcastTable->IsMcastRetranslator (m_peering->MainFromIfaceAddress (local), m_peering->MainFromIfaceAddress (from), mcastTo);
}

Ptr<GlobalPeering>
GlobalGraph::GetGlobalPeering () const
{
  return m_peering;
}
} // namespace lrr
} // namespace ns3

