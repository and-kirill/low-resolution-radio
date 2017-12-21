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
#ifndef MANET_STACK_MAC_H
#define MANET_STACK_MAC_H

#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"

#include "ns3/lrr-mac.h"

namespace ns3 {
template <typename Item> class Queue;
namespace lrr {

class AccessManager;

/**
 * \ingroup lrr
 *
 * \brief MAC layer of MANET stack. Keeps AccessManager inside and grants a collision-free access to the medium.
 * Collisions may occur only due to fading
 *
 * Forward packets down from net device to access manger;
 * forward packets up to net device.
 */
class CollisionFreeMacImpl : public Mac
{
public:
  static TypeId GetTypeId ();

  CollisionFreeMacImpl ();
  ~CollisionFreeMacImpl ();
  ///\name Virtual methods from Mac base class:
  ///\{
  /// Address:
  void SetAddress (Address address);
  Address GetAddress () const;
  /// Send method:
  void Enqueue (Ptr<Packet> packet, Address to, Address from, uint16_t type);
  /// Addresses: all methods are virtual=> address type may be changed
  virtual Address GetBroadcast () const;
  virtual Address GetMulticast (Ipv4Address group) const;
  virtual Address GetMulticast (Ipv6Address group) const;
  virtual bool IsMyAddress (const Address & a) const;
  virtual bool IsGroupAddress (const Address & a) const;
  ///\}
  /// Get access manager to determine max occupied time among neighbors:
  Ptr<AccessManager> GetAccessManager () const;
  /// Set MAC queue:
  void SetQueue (Ptr<Queue<Packet> > queue);
protected:
  /// Receive packet from PHY and forward to net-device. Made as protected for further features implementation.
  void Receive (Ptr<Packet> packet);
  /// Object's destructor
  void DoDispose ();
  void DoInitialize ();
private:
  /// Sniffer for erroneous packets:
  void ReceiveError (Ptr<const Packet> errorPacket);
protected:
  /// Own address:
  Address m_address;
private:
  ///\name Internals:
  ///\{
  /// Access manager (ideal-TDM functionality)
  Ptr<AccessManager> m_accessManager;
  ///\}
  ///RX error trace: destination, source addresses are passed
  TracedCallback<Address, Address> m_rxErrorTrace;
};
} // manet stack
} // namespace ns3

#endif /* MANET_STACK_MAC_H */
