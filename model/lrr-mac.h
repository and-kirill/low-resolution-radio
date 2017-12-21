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

#ifndef RADIO_MAC_H_
#define RADIO_MAC_H_

#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/object.h"
#include "ns3/lrr-phy.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

#include <set>

namespace ns3 {
template <typename Item> class Queue;
namespace lrr {
class NeighborAwareDevice;
class Phy;

/**
 * \ingroup lrr
 * \brief Implements an interface to MAC-layer of manet-stack
 *  Key feature is GetCommunicationNeighbors method
 */
class Mac : public Object
{
public:
  typedef Callback<void,Ptr<Packet>, Address, Address, uint16_t> ForwardUpCallback;
public:
  static TypeId GetTypeId ();
  Mac ();
  virtual ~Mac ();
  ///\name Settings:
  ///\{
  /// Max LRRDU size:
  void SetMaxMsduSize (uint32_t maxMsduSize);
  uint32_t GetMaxMsduSize () const;
  /// Address
  virtual void SetAddress (Address address) = 0;
  virtual Address GetAddress () const = 0;
  ///\}
  ///\name Neighbor detection functionality:
  ///\{
  std::vector <Ptr<NetDevice> > GetCommunicationNeighbors () const;
  ///\}
  ///\name MAC is responsible for addresses: NetDevice may communicate with any types of addresses:
  ///\{
  virtual Address GetBroadcast () const = 0;
  virtual Address GetMulticast (Ipv4Address group) const = 0;
  virtual Address GetMulticast (Ipv6Address group) const = 0;
  ///\}
  /// Receive from PHY
  virtual void Receive (Ptr<Packet> packet) = 0;
  ///\name Interaction with higher layers:
  ///\{
  /// Send Method:
  virtual void Enqueue (Ptr<Packet> packet, Address to, Address from, uint16_t type) = 0;
  /// Receive method: by callback:
  void SetForwardUpCallback (ForwardUpCallback upCallback);
  ///\}
  virtual void SetQueue (Ptr<Queue<Packet> > queue) = 0;
  ///\name PHY: needed to obtain neighbors
  ///\{
  void SetPhy (Ptr<SpectrumPhy> phy);
  Ptr<SpectrumPhy> GetPhy () const;
  ///\}
  /// Pass a packet to PHY:
  void StartTransmission (Ptr<const Packet> packet);
protected:
  /// Forward packet up:
  void ForwardUp (Ptr<Packet> p, Address from, Address to, uint16_t protocol);
  /// Object's starter and destructor:
  //\{
  void DoInitialize ();
  void DoDispose ();
  //\}
private:
  /// ForwardUp callback, used by inherited classes directly
  ForwardUpCallback m_upCallback;
  /// PHY pointer
  Ptr<Phy> m_phy;
  /// Max LRRDU size, that MAC can send
  uint32_t m_maxMsduSize;
};
} // namespace lrr
} // namespace ns3
#endif //RADIO_MAC_H_
