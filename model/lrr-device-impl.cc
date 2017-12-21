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

#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "lrr-device-impl.h"

NS_LOG_COMPONENT_DEFINE ("LrrNeighborAwareDeviceImpl");

namespace ns3 {
namespace lrr {

NS_OBJECT_ENSURE_REGISTERED (NeighborAwareDeviceImpl);

TypeId
NeighborAwareDeviceImpl::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::NeighborAwareDeviceImpl")
    .SetParent<NeighborAwareDevice> ()
    .AddConstructor<NeighborAwareDeviceImpl> ()
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&NeighborAwareDeviceImpl::GetMac,
                                        &NeighborAwareDeviceImpl::SetMac),
                   MakePointerChecker<Mac> ())
  ;
  return tid;
}

NeighborAwareDeviceImpl::NeighborAwareDeviceImpl () :
  m_mac (0), m_node (0), m_ifIndex (0), m_mtu (0)
{
}

void
NeighborAwareDeviceImpl::DoInitialize ()
{
  m_mac->Initialize ();
  NetDevice::DoInitialize ();
}

NeighborAwareDeviceImpl::~NeighborAwareDeviceImpl ()
{
  NS_LOG_FUNCTION (this);
}

void
NeighborAwareDeviceImpl::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_mac = 0;
  m_forwardUp = MakeNullCallback <bool,Ptr<NetDevice>,Ptr<const Packet>,uint16_t,const Address &> ();
  m_promiscRx = MakeNullCallback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address &, const Address &, enum PacketType> ();
  // chain up.
  NetDevice::DoDispose ();
}
// MAC {
void
NeighborAwareDeviceImpl::SetMac (Ptr<Mac> mac)
{
  m_mac = mac;
  m_mac->SetForwardUpCallback (MakeCallback (&NeighborAwareDeviceImpl::ForwardUp, this));
}

Ptr<Mac>
NeighborAwareDeviceImpl::GetMac () const
{
  return m_mac;
}
// }
// Inherited from NetDevice base class {
void
NeighborAwareDeviceImpl::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}

uint32_t
NeighborAwareDeviceImpl::GetIfIndex () const
{
  return m_ifIndex;
}

Ptr<Channel>
NeighborAwareDeviceImpl::GetChannel () const
{
  return 0;
}

void
NeighborAwareDeviceImpl::SetAddress (Address address)
{
  m_mac->SetAddress (address);
}

Address
NeighborAwareDeviceImpl::GetAddress () const
{
  NS_ASSERT (m_mac != 0);
  return m_mac->GetAddress ();
}

bool
NeighborAwareDeviceImpl::SetMtu (const uint16_t mtu)
{
  UintegerValue maxMsduSize;
  m_mac->GetAttribute ("MaxMsduSize", maxMsduSize);
  if (mtu > maxMsduSize.Get () || mtu == 0)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}

uint16_t
NeighborAwareDeviceImpl::GetMtu () const
{
  if (m_mtu == 0)
    {
      UintegerValue maxMsduSize;
      m_mac->GetAttribute ("MaxMsduSize", maxMsduSize);
      m_mtu = maxMsduSize.Get ();
    }
  return m_mtu;
}

bool
NeighborAwareDeviceImpl::IsLinkUp () const
{
  return true;
}

void
NeighborAwareDeviceImpl::AddLinkChangeCallback (Callback<void> callback)
{
}

bool
NeighborAwareDeviceImpl::IsBroadcast () const
{
  return true;
}

Address
NeighborAwareDeviceImpl::GetBroadcast () const
{
  return m_mac->GetBroadcast ();
}

bool
NeighborAwareDeviceImpl::IsMulticast () const
{
  return true;
}

Address
NeighborAwareDeviceImpl::GetMulticast (Ipv4Address multicastGroup) const
{
  return m_mac->GetMulticast (multicastGroup);
}

Address
NeighborAwareDeviceImpl::GetMulticast (Ipv6Address multicastGroup) const
{
  return m_mac->GetMulticast (multicastGroup);
}

bool
NeighborAwareDeviceImpl::IsPointToPoint () const
{
  return false;
}

bool
NeighborAwareDeviceImpl::IsBridge () const
{
  return false;
}

bool
NeighborAwareDeviceImpl::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << dest);
  m_mac->Enqueue (packet, dest, m_mac->GetAddress (), protocolNumber);
  return true;
}

bool
NeighborAwareDeviceImpl::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_FATAL_ERROR ("Radio does not support SendFrom");
  return false;
}

Ptr<Node>
NeighborAwareDeviceImpl::GetNode () const
{
  return m_node;
}

void
NeighborAwareDeviceImpl::SetNode (Ptr<Node> node)
{
  m_node = node;
}

bool
NeighborAwareDeviceImpl::NeedsArp () const
{
  return true;
}

void
NeighborAwareDeviceImpl::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_forwardUp = cb;
}

void
NeighborAwareDeviceImpl::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscRx = cb;
}

bool
NeighborAwareDeviceImpl::SupportsSendFrom () const
{
  return false;
}
// }
std::vector<Ptr<NetDevice> >
NeighborAwareDeviceImpl::GetCommunicationNeighbors () const
{
  NS_ASSERT (m_mac != 0);
  return GetMac ()->GetCommunicationNeighbors ();
}

void
NeighborAwareDeviceImpl::ForwardUp (Ptr<Packet> packet, Address from, Address to, uint16_t protocolN)
{
  m_forwardUp (this, packet, protocolN, from);
}
}
}
