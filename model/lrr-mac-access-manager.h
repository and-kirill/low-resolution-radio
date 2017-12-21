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

#ifndef MANET_MAC_ACCESS_MANAGER_H
#define MANET_MAC_ACCESS_MANAGER_H

#include "ns3/event-id.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"

namespace ns3 {

template <typename Item> class Queue;

namespace lrr {

class Mac;
/**
 * \ingroup lrr
 * \brief Core block of Collision-less MAC layer. Operates as follows:
 * 1. Each station takes into account all transmissions by all stations inside the interference area
 * 2. After medium is occupied, a special TimeoutEnd value is set (which is txStart + txDuration).
 * 3. When a new packet arrives, station may start to transmit at moment equals to maximum timeout
 *    end among all neighbors. Timeout end is updated when sending a packet to LOW.
 *
 */
class AccessManager : public Object
{
public:
  static TypeId GetTypeId ();
  AccessManager ();
  ~AccessManager ();
  /// Packet from net devices goes to queue:
  void Enqueue (Ptr<const Packet> packet);
  /// Callback to pass packet to PHY for transmission:
  void SetMac (Ptr<Mac> mac);
  /// Timeout end is needed for neighbors:
  Time GetTimeoutEnd ();
  ///\name Queue:
  void SetQueue (Ptr<Queue<Packet> > queue);
  Ptr<Queue<Packet> > GetQueue () const;
private:
  /// Packet list consists of pairs (duration, packet), needed for aggregation
  typedef std::list<std::pair<Time, Ptr<const Packet> > > PacketList;
private:
  /// Start access if not started and queue is not empty
  void StartAccessIfNeeded ();
  /**
   * \brief Core function: start access to the medium:
   * 1. Calculate TX-start time (among neighbors)
   * 2. Construct a packet burst (timed out packets are dropped automatically by queue)
   * 3. Update timeout end
   * 4. Schedule MAC send event for packet list (aggregation).
   */
  void StartAccess ();
  /// Get timeout and timeout end from all neighbors and return a max timeout end value.
  Time CalculateTxStartTime ();
  /// Update interference neighbors cache if it is too old
  void UpdateInterferenceNeighbors ();
  /// Object's destructor:
  void DoDispose ();
private:
  /// Queue for packets: non-qos simple queue
  Ptr<Queue<Packet> > m_queue;
  /// Absolute time when others may start to TX
  Time m_timeoutEnd;
  /// Event scheduled for StartAccess method
  EventId m_accessTimeout;
  /// Tx-start event:
  EventId m_txStartEvent;
  /// MAC pointer:
  Ptr<Mac> m_mac;
private:
  ///\name Attributes
  ///\{
  /// take propagation delay+preamble into account
  Time m_guardInterval;
  ///\}
  ///\name Interference Neighbors cache:
  ///\{
  /// Stored neighbors:
  std::vector<Ptr<NetDevice> > m_interferenceNeighbors;
  /// last update:
  Time m_lastInterferenceNeighborsUpdate;
  /// update period:
  Time m_interferenceNeighborUpdatePeriod;
  ///\}
};
} // namespace lrr
} // namespace ns3

#endif /* ACCESS_MANAGER_H */
