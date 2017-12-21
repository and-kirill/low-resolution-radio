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
 * Author: Kirill Andreev <k.andreev@skoltech.ru>
 */

#include "ns3/uinteger.h"
#include "ns3/ipv4.h"
#include "lrr-routing-peering.h"
namespace ns3
{
namespace lrr
{
NS_OBJECT_ENSURE_REGISTERED (GlobalPeering);
GlobalPeering::GlobalPeering ()
{}

TypeId
GlobalPeering::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::GlobalPeering")
    .SetParent<Object> ()
    .AddConstructor<GlobalPeering>()
    .AddTraceSource ("LinkOpened", "New neighbor found.",
                     MakeTraceSourceAccessor (&GlobalPeering::m_linkOpen),
                     "ns3::Ipv4Address")
    .AddTraceSource ("LinkClosed", "Known neighbor lost.",
                     MakeTraceSourceAccessor (&GlobalPeering::m_linkClosed),
                     "ns3::Ipv4Address")
  ;
  return tid;
}

void
GlobalPeering::Clear ()
{
  m_globalLinkSet.clear ();
  m_addressToNodeMap.clear ();
  m_registredNodes.clear ();
  m_topology.clear ();
}

void
GlobalPeering::AddVertex (Ptr<Node> node)
{
  std::vector<Interface> radioInterfaces = GetRadioInterfaces (node);
  for (std::vector<Interface>::const_iterator i = radioInterfaces.begin (); i != radioInterfaces.end (); i++)
    {
      m_addressToNodeMap.insert (std::make_pair (i->address, node));
    }
  m_registredNodes.push_back (node);
  m_topology.insert (std::make_pair (MainFromIfaceAddress (radioInterfaces[0].address), std::map <Ipv4Address, uint16_t> ()));
}

void
GlobalPeering::CreateLinks ()
{
  m_globalLinkSet.clear ();
  Topology oldTopology = m_topology;
  m_topology.clear ();
  for (std::vector<Ptr<Node> >::const_iterator i = m_registredNodes.begin (); i != m_registredNodes.end (); ++i)
    {
      CreateOutgoingLinks (*i);
    }
  UpdateLinkStatus (oldTopology);
}

void
GlobalPeering::UpdateLinkStatus (const Topology& oldTopology) const
{
  NS_ASSERT (m_topology.size () == oldTopology.size ());
  Topology::const_iterator newTopoIt = m_topology.begin ();
  Topology::const_iterator oldTopoIt = oldTopology.begin ();
  for (newTopoIt = m_topology.begin (); newTopoIt != m_topology.end (); newTopoIt++, oldTopoIt++)
    {
      std::map <Ipv4Address, uint16_t>::const_iterator newLinksIt = newTopoIt->second.begin ();
      std::map <Ipv4Address, uint16_t>::const_iterator oldLinksIt = oldTopoIt->second.begin ();
      for (; newLinksIt != newTopoIt->second.end (); newLinksIt++)
        {
          if (oldTopoIt->second.find (newLinksIt->first) == oldTopoIt->second.end ())
            {
              m_linkOpen (newTopoIt->first, newLinksIt->first);
            }
        }
      for (; oldLinksIt != oldTopoIt->second.end (); oldLinksIt++)
        {
          if (newTopoIt->second.find (oldLinksIt->first) == newTopoIt->second.end ())
            {
              m_linkClosed (oldTopoIt->first, oldLinksIt->first);
            }
        }
    }
}

void
GlobalPeering::CreateOutgoingLinks (const Ptr<Node> srcNode)
{
  std::vector<Interface> nodeInterfaces = GetRadioInterfaces (srcNode);
  for (std::vector<Interface>::const_iterator i = nodeInterfaces.begin (); i != nodeInterfaces.end (); i++)
    {
      Ptr<NeighborAwareDevice> srcDev = i->device;
      std::vector<Ipv4Address> neighbors = GetIfaceNeighbors (srcDev);
      if (neighbors.size () == 0)
        {
          m_topology.insert (std::make_pair (MainFromIfaceAddress (i->address), std::map <Ipv4Address, uint16_t> ()));
        }
      for (std::vector<Ipv4Address>::const_iterator j = neighbors.begin (); j != neighbors.end (); j++)
        {
          /*Now, metric is a hop-count. But may be any other:)*/
          Link link (MainFromIfaceAddress (i->address), i->address, MainFromIfaceAddress (*j), *j, 1);
          InsertLink (link);
        }
    }
}

std::vector<Ipv4Address>
GlobalPeering::GetIfaceNeighbors (Ptr<NetDevice> device)
{
  std::vector<Ipv4Address> retval;
  NS_ASSERT (device->GetObject<NeighborAwareDevice> ());
  std::vector <Ptr<NetDevice> > neighborDevs = device->GetObject<NeighborAwareDevice> ()->GetCommunicationNeighbors ();
  for (std::vector <Ptr<NetDevice> >::const_iterator i = neighborDevs.begin (); i != neighborDevs.end (); i++)
    {
      retval.push_back (InterfaceAddrFromDevice (*i));
    }
  return retval;
}

void
GlobalPeering::InsertLink (Link link)
{
  std::pair<Ipv4Address, Ipv4Address> key = std::make_pair (link.srcMainAddress, link.dstMainAddress);
  LinkSetIterator linkSetIt = m_globalLinkSet.find (key);
  if (linkSetIt == m_globalLinkSet.end ())
    {
      NS_ASSERT (key.first != key.second);
      m_globalLinkSet.insert (std::make_pair (key, std::vector<Link> ()));
      linkSetIt = m_globalLinkSet.find (key);
    }
  linkSetIt->second.push_back (link);
  UpdateTopology (link);
}

void
GlobalPeering::UpdateTopology (Link link)
{
  Topology::iterator sourceIt = m_topology.find (link.srcMainAddress);
  if (sourceIt == m_topology.end ())
    {
      m_topology.insert (std::make_pair (link.srcMainAddress, std::map <Ipv4Address, uint16_t> ()));
      sourceIt = m_topology.find (link.srcMainAddress);
    }
  std::map <Ipv4Address, uint16_t>::iterator dstIt = sourceIt->second.find (link.dstMainAddress);
  if (
    (dstIt == sourceIt->second.end ()) ||                                    // No link exists
    ((dstIt != sourceIt->second.end ()) && (dstIt->second > link.metric))    // There is a link with worse metric
    )
    {
      sourceIt->second.insert (std::make_pair (link.dstMainAddress, link.metric));
    }
}

std::vector<GlobalPeering::Link>
GlobalPeering::GetLinks (Ipv4Address srcMainAddress, Ipv4Address dstMainAddress) const
{
  std::pair<Ipv4Address, Ipv4Address> key = std::make_pair (srcMainAddress, dstMainAddress);
  LinkSetConstIterator i = m_globalLinkSet.find (key);
  if (i != m_globalLinkSet.end ())
    {
      return i->second;
    }
  return std::vector<Link> ();
}

GlobalPeering::Topology
GlobalPeering::GetTopology () const
{
  return m_topology;
}
// Utilities:
Ipv4Address
GlobalPeering::MainFromIfaceAddress (Ipv4Address interfaceAddress)
{
  if (m_addressToNodeMap.find (interfaceAddress) == m_addressToNodeMap.end ())
    {
      NS_FATAL_ERROR ("Requested address " << interfaceAddress);
      return Ipv4Address ();
    }
  return GetMainAddress (m_addressToNodeMap[interfaceAddress]);
}

std::vector<GlobalPeering::Interface>
GlobalPeering::GetRadioInterfaces (Ptr<Node> node)
{
  std::vector<Interface> retval;
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  if (ipv4 == 0)
    {
      return retval;
    }

  NS_ASSERT (ipv4);
  for (uint32_t i = 0; i < ipv4->GetNInterfaces (); i++)
    {
      // Address of interface: TODO: we do not support multiple addresses on a single interface
      Ipv4Address address = ipv4->GetAddress (i, 0).GetLocal ();
      // Number of network device:
      int32_t interface = ipv4->GetInterfaceForAddress (address);
      // Pointer to network device:
      Ptr<NeighborAwareDevice> device = DynamicCast<NeighborAwareDevice> (ipv4->GetNetDevice (interface));
      if (device)
        {
          retval.push_back (Interface (address, device));
        }
    }
  return retval;
}

Ipv4Address
GlobalPeering::InterfaceAddrFromDevice (Ptr<NetDevice> device)
{
  std::vector <Interface> radioInterfaces = GetRadioInterfaces (device->GetNode ());
  for (std::vector <Interface>::const_iterator  i = radioInterfaces.begin (); i != radioInterfaces.end (); i++)
    {
      if (i->device == device)
        return i->address;
    }
  return Ipv4Address ();
}

Ipv4Address
GlobalPeering::GetMainAddress (Ptr<Node> node)
{
  std::vector<Interface> radioInterfaces = GetRadioInterfaces (node);
  if (radioInterfaces.size ())
    {
      return radioInterfaces[0].address;
    }
  return Ipv4Address ();
}

} //lrr
} //ns3
