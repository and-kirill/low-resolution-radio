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
 * Author:  Denis Fakhriev <fakhriev@telum.ru>
 *          Kirill Andreev <k.andreev@skoltech.ru>
 */

#include "ns3/mac48-address.h"
#include "ns3/queue.h"
#include "lrr-mac-impl.h"

#include "lrr-mac-access-manager.h"
#include "lrr-mac-header.h"
#include "lrr-device-impl.h"

namespace ns3 {
namespace lrr {

NS_OBJECT_ENSURE_REGISTERED (CollisionFreeMacImpl);

TypeId 
CollisionFreeMacImpl::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::CollisionFreeMacImpl")
    .SetParent<Mac> ()
    .AddConstructor<CollisionFreeMacImpl> ()
    .AddTraceSource ("RxError", "Error when receiving a packet",
                     MakeTraceSourceAccessor (&CollisionFreeMacImpl::m_rxErrorTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

CollisionFreeMacImpl::CollisionFreeMacImpl () :
  Mac (),
  m_address (Address ()),
  m_accessManager (CreateObject<AccessManager> ())
{
  m_accessManager->SetMac (this);
}

void
CollisionFreeMacImpl::DoInitialize ()
{
  /// Works with lrr::Phy
  GetPhy ()->TraceConnectWithoutContext ("RxEndError", MakeCallback (&CollisionFreeMacImpl::ReceiveError, this));
}

CollisionFreeMacImpl::~CollisionFreeMacImpl ()
{}

void
CollisionFreeMacImpl::DoDispose ()
{
  m_accessManager->SetMac (0);
  m_accessManager = 0;
  Mac::DoDispose ();
}

void
CollisionFreeMacImpl::SetAddress (Address address)
{
  m_address = address;
}

Address
CollisionFreeMacImpl::GetAddress () const
{
  return m_address;
}
// Ideal MAC layer functionality: obtain reservations from neighbors
Ptr<AccessManager>
CollisionFreeMacImpl::GetAccessManager () const
{
  return m_accessManager;
}

void
CollisionFreeMacImpl::Enqueue (Ptr<Packet> packet, Address to, Address from, uint16_t type)
{
  MacHeader header;
  header.SetType (type);
  header.SetSource (from);
  header.SetDestination (to);
  packet->AddHeader (header);
  m_accessManager->Enqueue (packet);
}

void
CollisionFreeMacImpl::Receive (Ptr<Packet> packet)
{
  MacHeader header;
  packet->RemoveHeader (header);
  Address from = header.GetSource ();
  Address to = header.GetDestination ();
  uint16_t protocolNumber = header.GetType ();

  if (IsGroupAddress (to) || IsMyAddress (to))
    {
      ForwardUp (packet, from, to, protocolNumber);
    }
}

void
CollisionFreeMacImpl::SetQueue (Ptr<Queue<Packet> > queue)
{
  m_accessManager->SetQueue (queue);
}

void
CollisionFreeMacImpl::ReceiveError (Ptr<const Packet> errorPacket)
{
  MacHeader header;
  if (!errorPacket->PeekHeader (header))
    {
      return;
    }
  Address destination = header.GetDestination ();
  Address source = header.GetSource ();
  if (IsMyAddress (destination))
    {
      m_rxErrorTrace (destination, source);
    }
}

Address
CollisionFreeMacImpl::GetBroadcast () const
{
  return Mac48Address::GetBroadcast ();
}

Address
CollisionFreeMacImpl::GetMulticast (Ipv4Address group) const
{
  return Mac48Address::GetMulticast (group);
}

Address
CollisionFreeMacImpl::GetMulticast (Ipv6Address group) const
{
  return Mac48Address::GetMulticast (group);
}

bool
CollisionFreeMacImpl::IsMyAddress (const Address & a) const
{
  return Mac48Address::ConvertFrom (a) == Mac48Address::ConvertFrom (m_address);
}

bool
CollisionFreeMacImpl::IsGroupAddress (const Address & a) const
{
  return Mac48Address::ConvertFrom (a).IsGroup ();
}
} // namespace lrr
} // namespace ns3
