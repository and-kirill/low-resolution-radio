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

#ifndef LRR_NET_DEVICE_IMPL_H_
#define LRR_NET_DEVICE_IMPL_H_


#include "ns3/lrr-device.h"
#include "ns3/node.h"
#include "ns3/lrr-mac.h"

namespace ns3
{
namespace lrr
{
/**
 * \defgroup lrr LRR: low-resolution radio stack
 */
class NeighborAwareSpectrumChannel;
/**
 * \ingroup lrr
 * \brief Represents a networking device, which  provides
 *  global topology information using GetNeigbors method
 */
class NeighborAwareDeviceImpl : public NeighborAwareDevice
{
public:
  static TypeId GetTypeId ();
  NeighborAwareDeviceImpl ();
  virtual ~NeighborAwareDeviceImpl ();
  ///\name MAC installations:
  ///\{
  void SetMac (Ptr<Mac> mac);
  Ptr<Mac> GetMac () const;
  ///\}
  ///\name Inherited from NetDevice base class:
  ///\{
  /// Interface index:
  void SetIfIndex (const uint32_t index);
  uint32_t GetIfIndex () const;
  /// Channel:
  Ptr<NeighborAwareSpectrumChannel> SetChannel ();
  Ptr<Channel> GetChannel () const;
  /// Address:
  void SetAddress (Address address);
  Address GetAddress () const;
  /// MTU:
  bool SetMtu (const uint16_t mtu);
  uint16_t GetMtu () const;
  /// Link UP:
  bool IsLinkUp () const;
  void AddLinkChangeCallback (Callback<void> callback);
  /// Broadcast/Multicast addresses:
  bool IsBroadcast () const;
  Address GetBroadcast () const;
  Address GetMulticast (Ipv4Address multicastGroup) const;
  Address GetMulticast (Ipv6Address addr) const;
  bool IsMulticast () const;
  /// Bridge or P2P:
  bool IsPointToPoint () const;
  bool IsBridge () const;
  /// Call MAC Enqueue method:
  bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  Ptr<Node> GetNode () const;
  void SetNode (Ptr<Node> node);
  bool NeedsArp () const;
  void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  void SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb);
  bool SupportsSendFrom () const;
  ///\}
  ///\name Global topology knowledge functionality:
  ///\{
  /// Obtain neighbors: depends on MAC type
  std::vector<Ptr<NetDevice> > GetCommunicationNeighbors () const;
  ///\}
private:
  void ForwardUp (Ptr<Packet> packet, Address from, Address to, uint16_t protocolN);
  /// Object's starter and disposer:
  void DoInitialize ();
  void DoDispose ();
private:
  /// Pointer to a MAC
  Ptr<Mac> m_mac;
  /// pointer to a Node
  Ptr<Node> m_node;
  /// NetDevice's interface index
  uint32_t m_ifIndex;

  mutable uint16_t m_mtu;

  NetDevice::ReceiveCallback m_forwardUp;
  NetDevice::PromiscReceiveCallback m_promiscRx;
};
}
}
#endif //RADIO_NET_DEVICE_IMPL_H_
