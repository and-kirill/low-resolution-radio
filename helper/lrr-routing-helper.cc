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
 * Authors: Pavel Boyko <boyko@telum.ru>, written after OlsrHelper by Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "lrr-routing-helper.h"
#include "ns3/lrr-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{

LrrRoutingHelper::LrrRoutingHelper() : 
  Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::lrr::RoutingProtocol");
}

LrrRoutingHelper* 
LrrRoutingHelper::Copy (void) const 
{
  return new LrrRoutingHelper (*this); 
}

Ptr<Ipv4RoutingProtocol> 
LrrRoutingHelper::Create (Ptr<Node> node) const
{
  Ptr<lrr::RoutingProtocol> agent = m_agentFactory.Create<lrr::RoutingProtocol> ();
  node->AggregateObject (agent);
  return agent;
}

void 
LrrRoutingHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

}
