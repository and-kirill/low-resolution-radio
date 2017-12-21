/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Telum (www.telum.ru)
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
 * Author:  Kirill Andreev <k.andreev@skoltech.ru>
 */

#ifndef LRR_MCAST_TEST_H_
#define LRR_MCAST_TEST_H_

#include "ns3/test.h"
#include "ns3/socket.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"

#include "ns3/lrr-channel.h"
#include "ns3/lrr-routing-graph.h"
#include <set>
namespace ns3
{
class LrrMcastTestCase : public ns3::TestCase
{
public:
  /**
   * \brief Base class for testing multicast trees.
   * The order of function calls is the following for each subsclass:
   * 1. Run CreateNodes
   * 2. Run InstallChannel. Topology is created here by matrix loss
   * model
   * 3. Run InstallDevices
   * 4. Run InstallInternet
   * 5. Add multicast groups as needed
   * 6. Make all checks:
   *       CheckGraph: correctness of created graph
   *       CheckMulticastPath: correctness of multicast groups
   *       Check Members: correctness of group members
   *       CheckRetranslators: check tree
   * 7. Create sockets for each node
   * 8. Generate some packets
   * 9. Check results
   */
  LrrMcastTestCase (uint32_t nodeCount, Time totalTime);
  virtual ~LrrMcastTestCase ();

protected:
  ///Create test scenario's topology (matrix loss model) and channel:
  virtual void InstallChannel () = 0;
  ///\name Check constructed graph:
  ///\{
  virtual void CheckGraph () = 0;
  /// Check exsistance of multicast path:
  virtual void CheckMulticastPath () = 0;
  /// Check group members
  virtual void CheckMembers () = 0;
  /// Check multicast retranslators
  virtual void CheckRetranslators () = 0;
  /// Run test
  virtual void DoRun () = 0;
  ///\}
protected:
  /// Create nodes:
  void CreateNodes ();
  /// Install net devices:
  void InstallDevices ();
  /// Install internet stack:
  void InstallInternet ();
  /// Add group to a global graph:
  void AddGroup (std::set <uint32_t> nodeIndexes);
  /// Create socket for each node.
  void CreateSockets ();
  /// Send a packet to a socket to a given destination
  void Send (uint32_t senderIndex, Ipv4Address multicastDst);
  /// Receive callback for each socket
  void Receive (Ptr<Socket> socket);
  /// Check the number of received packets
  void CheckReceive (std::vector<uint32_t> expected);
  /// Run simulator:
  void RunSimulator ();
  /// Delete all data structures:
  void DestroyTest ();
protected:
  /// Node cpntainer:
  NodeContainer m_nodes;
  /// Pointer to a global graph:
  lrr::GlobalGraph* m_graph;
  /// Pointer to radio channel:
  Ptr<lrr::NeighborAwareSpectrumChannel> m_channel;
private:
  /// The number of nodes:
  uint32_t m_nodeCount;
  /// Total simulatin time:
  Time m_totalTime;
  // Devices for nodes: (one per node)
  NetDeviceContainer m_nodeDevices;
  /// Scokets: one socket per one node
  std::vector<Ptr<Socket> > m_sockets;
  /// How many packets were received by each node
  std::vector <uint32_t> m_receiveCount;
  /// Port for communication:
  uint16_t m_port;
  uint32_t m_lastAssignedGid;
  uint32_t m_lastAssignedMcast;
};

/**
  *                 Created graph
  *
  * 1------2------3------4------5------6-x-x-x-7
  *        ^             ^      ^              ^
  *           Multicast group {2,4,5,7}
  */
class LrrMcastColumnTest : public LrrMcastTestCase
{
public:
  LrrMcastColumnTest ();
private:
  static const std::string m_correctGraph;
  static const std::string m_correctMcastTable;
  ///\name Inherited from LrrMcastTestCase:
  ///\{
  void InstallChannel ();
  void CheckGraph ();
  void CheckMulticastPath ();
  void CheckMembers ();
  void CheckRetranslators ();
private:
  void DoRun ();
  ///\}
};

/**
  * Created graph
  *
  *  0----1-----2----3
  *   \   |
  *     \ |
  *     __4__
  *  __/     \__
  * 5           6
  * |           |
  * |           |
  * 7           8
  *
  * Multicast group {0,3,6,7}, {2,4,5,8}
  */
class LrrMcastXTest : public LrrMcastTestCase
{
public:
  LrrMcastXTest ();
private:
  static const std::string m_correctGraph;
  static const std::string m_correctMcastTable;
  ///\name Inherited from LrrMcastTestCase:
  ///\{
  void InstallChannel ();
  void CheckGraph ();
  void CheckMulticastPath ();
  void CheckMembers ();
  void CheckRetranslators ();
private:
  void DoRun ();
  ///\}
};
}
#endif
