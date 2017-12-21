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
 *          Kirill Andreev <k.andreev@skoltech.ru>
 */

#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/constant-position-mobility-model.h"

#include "ns3/internet-stack-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-socket-factory.h"

#include "ns3/lrr-mcast-group-mgt.h"

#include "ns3/lrr-channel-helper.h"
#include "ns3/lrr-device-helper.h"
#include "ns3/lrr-routing-helper.h"
#include "ns3/wifi-spectrum-value-helper.h"

#include "ns3/config.h"
#include "ns3/double.h"
#include "lrr-routing-mcast-test.h"

#include <algorithm>

namespace ns3
{

using namespace lrr;
// ===================================== Base class: =====================================
LrrMcastTestCase::LrrMcastTestCase (uint32_t nodeCount, Time totalTime) :

  TestCase ("LRR multicast test"),
  m_nodes (),
  m_graph (0),
  m_nodeCount (nodeCount),
  m_totalTime (totalTime),
  m_receiveCount (std::vector<uint32_t> (nodeCount, 0)),
  m_port (12345),
  m_lastAssignedGid (1),
  m_lastAssignedMcast (Ipv4Address ("227.0.0.1").Get ())
{};

LrrMcastTestCase::~LrrMcastTestCase ()
{
  m_receiveCount.clear ();
}

void
LrrMcastTestCase::CreateNodes ()
{
  m_nodes.Create (m_nodeCount);
  NS_ASSERT (NodeContainer::GetGlobal ().GetN () == m_nodeCount);
  for (uint32_t i = 0; i < m_nodes.GetN (); ++i)
    {
      Ptr<Node> node = m_nodes.Get (i);
      Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();
      mobility->SetPosition (Vector (0, 0, 0));
      node->AggregateObject (mobility);
    }
}

void
LrrMcastTestCase::InstallDevices ()
{
  NeighborAwareDeviceHelper deviceHelper;
  deviceHelper.SetChannel (m_channel);
  // Set modulation and noise:
  WifiSpectrumValue5MhzFactory sf;
  deviceHelper.SetTxPowerSpectralDensity (sf.CreateTxPowerSpectralDensity (0.1 /*Watts*/, 1 /*channel number*/));
  deviceHelper.SetNoisePowerSpectralDensity (sf.CreateConstant (1.381e-23 * 290 /*kT*/));
  deviceHelper.SetRxFilter (sf.CreateRfFilter (5));
  m_nodeDevices = deviceHelper.Install (m_nodes);
}

void
LrrMcastTestCase::InstallInternet ()
{
  LrrRoutingHelper routingHelper;
  // Install Internet stack
  InternetStackHelper internet;
  internet.SetRoutingHelper (routingHelper);
  internet.Install (m_nodes);
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipAddrs.Assign (m_nodeDevices);
  m_graph = GlobalGraph::Instance ();
}

void
LrrMcastTestCase::AddGroup (std::set <uint32_t> nodeIndexes)
{
  NodeContainer group;
  std::set<Ipv4Address> addresses;
  for (std::set<uint32_t>::const_iterator i = nodeIndexes.begin (); i != nodeIndexes.end (); i++)
    {
      group.Add (m_nodes.Get (*i));
      addresses.insert (GlobalPeering::GetMainAddress (m_nodes.Get (*i)));
    }
  GlobalGroupManagement::GetInstance ()->AddMcastGroup (Ipv4Address (m_lastAssignedMcast), addresses);
  m_lastAssignedGid++;
  m_lastAssignedMcast++;
}

void
LrrMcastTestCase::CreateSockets ()
{
  for (uint32_t i = 0; i < m_nodes.GetN (); i++)
    {
      Ptr<Socket> socket = Socket::CreateSocket (m_nodes.Get (i), UdpSocketFactory::GetTypeId ());
      socket->SetRecvCallback (MakeCallback (&LrrMcastXTest::Receive, this));
      if (socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_port)))
        {
          NS_FATAL_ERROR ("Failed to bind() to port " << m_port);
        }
      socket->SetAllowBroadcast (true);
      m_sockets.push_back (socket);
    }
}

void
LrrMcastTestCase::Send (uint32_t senderIndex, Ipv4Address multicastDst)
{
  Ptr<Packet> packet = Create<Packet> (100);
  std::cout << "Node number " << senderIndex << "has sent a packet. DST = " << multicastDst << "Now = " << Simulator::Now () << std::endl;
  m_sockets [senderIndex]->SendTo (packet, 0, InetSocketAddress (multicastDst, m_port));
}

void
LrrMcastTestCase::Receive (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (0xffffffff, 0, from);
  std::vector<Ptr<Socket> > sockets;
  std::vector<Ptr<Socket> >::const_iterator in = std::find (m_sockets.begin (), m_sockets.end (), socket);
  NS_ASSERT (in != m_sockets.end ());
  uint32_t node = in - m_sockets.begin ();
  std::cout << "Node number " << node << "has received a packet. Now = " << Simulator::Now () << std::endl;
  m_receiveCount [node]++;
}

void
LrrMcastTestCase::CheckReceive (std::vector<uint32_t> expected)
{
  NS_TEST_EXPECT_MSG_EQ (expected.size (), m_receiveCount.size (), "Receive count test");
  std::cout << "Now == " << Simulator::Now () << std::endl;
  for (unsigned int i = 0; i < expected.size (); i++)
    {
      std::cout << "expected[" << i << "] = " << expected[i] << ", got[" << i << "] = " << m_receiveCount[i] << std::endl;
      NS_TEST_EXPECT_MSG_EQ (expected[i], m_receiveCount[i], "Receive count test");
    }
  m_receiveCount = std::vector<uint32_t> (m_nodes.GetN (), 0);
}

void
LrrMcastTestCase::RunSimulator ()
{
  Simulator::Stop (m_totalTime);
  Simulator::Run ();
  Simulator::Destroy ();
}

void
LrrMcastTestCase::DestroyTest ()
{
  m_graph->Stop ();
  GlobalGraph::Destroy ();
  GlobalGroupManagement::Destroy ();
  m_graph = 0;
}

// ===================================== Chain test: =====================================
const std::string LrrMcastColumnTest::m_correctGraph = std::string (
    "# Network topology as seen by vertex 10.0.0.1\n"
    "digraph G {\n"
    "\t\"10.0.0.1\" [label=\"10.0.0.1\", shape=box];\n"
    "\t\"10.0.0.1\" -> \"10.0.0.2\" [label=\"1\"];\n"
    "\t\"10.0.0.2\" [label=\"10.0.0.2\"];\n"
    "\t\"10.0.0.2\" -> \"10.0.0.1\" [label=\"1\"];\n"
    "\t\"10.0.0.2\" -> \"10.0.0.3\" [label=\"1\"];\n"
    "\t\"10.0.0.3\" [label=\"10.0.0.3\"];\n"
    "\t\"10.0.0.3\" -> \"10.0.0.2\" [label=\"1\"];\n"
    "\t\"10.0.0.3\" -> \"10.0.0.4\" [label=\"1\"];\n"
    "\t\"10.0.0.4\" [label=\"10.0.0.4\"];\n"
    "\t\"10.0.0.4\" -> \"10.0.0.3\" [label=\"1\"];\n"
    "\t\"10.0.0.4\" -> \"10.0.0.5\" [label=\"1\"];\n"
    "\t\"10.0.0.5\" [label=\"10.0.0.5\"];\n"
    "\t\"10.0.0.5\" -> \"10.0.0.4\" [label=\"1\"];\n"
    "\t\"10.0.0.5\" -> \"10.0.0.6\" [label=\"1\"];\n"
    "\t\"10.0.0.6\" [label=\"10.0.0.6\"];\n"
    "\t\"10.0.0.6\" -> \"10.0.0.5\" [label=\"1\"];\n"
    "\t\"10.0.0.7\" [label=\"10.0.0.7\"];\n"
    "};\n"
    );

LrrMcastColumnTest::LrrMcastColumnTest () :
  LrrMcastTestCase (7, Seconds (100))
{}

void
LrrMcastColumnTest::InstallChannel ()
{
  LrrChannelHelper channelHelper  = LrrChannelHelper::Default ();
  Ptr<MatrixPropagationLossModel> matrix = CreateObject<MatrixPropagationLossModel> ();
  double disabled = 216; // -200 dBm
  double enabled = 56; // -40 dBm
  matrix->SetDefaultLoss (disabled);
  matrix->SetLoss (m_nodes.Get (0)->GetObject <MobilityModel> (), m_nodes.Get (1)->GetObject <MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (1)->GetObject <MobilityModel> (), m_nodes.Get (2)->GetObject <MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (2)->GetObject <MobilityModel> (), m_nodes.Get (3)->GetObject <MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (3)->GetObject <MobilityModel> (), m_nodes.Get (4)->GetObject <MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (4)->GetObject <MobilityModel> (), m_nodes.Get (5)->GetObject <MobilityModel> (), enabled);
  //matrix->SetLoss (nodes.Get (5), nodes.Get (6), disabled);
  channelHelper.SetDeterministicPropagationLoss (matrix);
  m_channel = channelHelper.Create ();
}

void
LrrMcastColumnTest::CheckGraph ()
{
  std::ostringstream os;
  m_graph->PrintGraph (os);
  NS_TEST_ASSERT_MSG_EQ (os.str (), m_correctGraph, "Graph was created as expected");
}

void
LrrMcastColumnTest::CheckMulticastPath ()
{
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.2", "227.0.0.1"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.4", "227.0.0.1"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.5", "227.0.0.1"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.7", "227.0.0.1"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.2", "227.0.0.2"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.1", "227.0.0.1"), false, "HaveMcastPath() works as expected");
}

void
LrrMcastColumnTest::CheckMembers ()
{
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.1", "227.0.0.1"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.2", "227.0.0.1"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.3", "227.0.0.1"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.4", "227.0.0.1"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.5", "227.0.0.1"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.6", "227.0.0.1"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.7", "227.0.0.1"), true, "IsInMulticastGroup () works as expected");
}

void
LrrMcastColumnTest::CheckRetranslators ()
{
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.2", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.2", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.4", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.5", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.5", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.5", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
}

void
LrrMcastColumnTest::DoRun ()
{
  CreateNodes ();
  InstallChannel ();
  InstallDevices ();
  InstallInternet ();
  std::set <uint32_t> group;
  group.insert (1);
  group.insert (3);
  group.insert (4);
  group.insert (6);
  AddGroup (group);
  m_graph->Start ();
  //Check stack:
  CheckGraph ();
  CheckMulticastPath ();
  CheckMembers ();
  CheckRetranslators ();
  // Start simulation part of test:
  CreateSockets ();
  for (unsigned int i = 0; i < m_nodes.GetN (); i++)
    {
      Time at = Seconds (10.0 + 0.5 * (double)i);
      // Members are: 0, 3, 6, 7
      Simulator::Schedule (at, &LrrMcastColumnTest::Send, this, i, Ipv4Address ("227.0.0.1"));
      at = Seconds (20.0 + 0.5 * (double)i);
      // No members:
      Simulator::Schedule (at, &LrrMcastColumnTest::Send, this, i, Ipv4Address ("227.0.0.2"));
    }
  // Check first group:
  std::vector <uint32_t> reference;
  reference.push_back (0);
  reference.push_back (2);
  reference.push_back (0);
  reference.push_back (2);
  reference.push_back (2);
  reference.push_back (0);
  reference.push_back (0);
  Simulator::Schedule (Seconds (19), &LrrMcastColumnTest::CheckReceive, this, reference);
  // If group does not exist, nothing must be received:
  reference = std::vector <uint32_t> (7,0);
  Simulator::Schedule (Seconds (29), &LrrMcastColumnTest::CheckReceive, this, reference);
  RunSimulator ();
  DestroyTest ();
}

// ===================================== T-topoTest: =====================================
const std::string LrrMcastXTest::m_correctGraph = std::string (
    "# Network topology as seen by vertex 10.0.0.1\n"
    "digraph G {\n"
    "\t\"10.0.0.1\" [label=\"10.0.0.1\", shape=box];\n"
    "\t\"10.0.0.1\" -> \"10.0.0.2\" [label=\"1\"];\n"
    "\t\"10.0.0.1\" -> \"10.0.0.5\" [label=\"1\"];\n"
    "\t\"10.0.0.2\" [label=\"10.0.0.2\"];\n"
    "\t\"10.0.0.2\" -> \"10.0.0.1\" [label=\"1\"];\n"
    "\t\"10.0.0.2\" -> \"10.0.0.3\" [label=\"1\"];\n"
    "\t\"10.0.0.2\" -> \"10.0.0.5\" [label=\"1\"];\n"
    "\t\"10.0.0.3\" [label=\"10.0.0.3\"];\n"
    "\t\"10.0.0.3\" -> \"10.0.0.2\" [label=\"1\"];\n"
    "\t\"10.0.0.3\" -> \"10.0.0.4\" [label=\"1\"];\n"
    "\t\"10.0.0.4\" [label=\"10.0.0.4\"];\n"
    "\t\"10.0.0.4\" -> \"10.0.0.3\" [label=\"1\"];\n"
    "\t\"10.0.0.5\" [label=\"10.0.0.5\"];\n"
    "\t\"10.0.0.5\" -> \"10.0.0.1\" [label=\"1\"];\n"
    "\t\"10.0.0.5\" -> \"10.0.0.2\" [label=\"1\"];\n"
    "\t\"10.0.0.5\" -> \"10.0.0.6\" [label=\"1\"];\n"
    "\t\"10.0.0.5\" -> \"10.0.0.7\" [label=\"1\"];\n"
    "\t\"10.0.0.6\" [label=\"10.0.0.6\"];\n"
    "\t\"10.0.0.6\" -> \"10.0.0.5\" [label=\"1\"];\n"
    "\t\"10.0.0.6\" -> \"10.0.0.8\" [label=\"1\"];\n"
    "\t\"10.0.0.7\" [label=\"10.0.0.7\"];\n"
    "\t\"10.0.0.7\" -> \"10.0.0.5\" [label=\"1\"];\n"
    "\t\"10.0.0.7\" -> \"10.0.0.9\" [label=\"1\"];\n"
    "\t\"10.0.0.8\" [label=\"10.0.0.8\"];\n"
    "\t\"10.0.0.8\" -> \"10.0.0.6\" [label=\"1\"];\n"
    "\t\"10.0.0.9\" [label=\"10.0.0.9\"];\n"
    "\t\"10.0.0.9\" -> \"10.0.0.7\" [label=\"1\"];\n"
    "};\n"
    );

LrrMcastXTest::LrrMcastXTest() :
  LrrMcastTestCase (9, Seconds (100))
{};

void
LrrMcastXTest::InstallChannel ()
{
  LrrChannelHelper channelHelper = LrrChannelHelper::Default ();
  Ptr<MatrixPropagationLossModel> matrix = CreateObject<MatrixPropagationLossModel> ();
  double enabled = 50; // -40 dBm
  double disabled = 250;
  matrix->SetDefaultLoss (disabled);
  matrix->SetLoss (m_nodes.Get (0)->GetObject<MobilityModel> (), m_nodes.Get (1)->GetObject<MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (0)->GetObject<MobilityModel> (), m_nodes.Get (4)->GetObject<MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (1)->GetObject<MobilityModel> (), m_nodes.Get (2)->GetObject<MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (1)->GetObject<MobilityModel> (), m_nodes.Get (4)->GetObject<MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (2)->GetObject<MobilityModel> (), m_nodes.Get (3)->GetObject<MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (4)->GetObject<MobilityModel> (), m_nodes.Get (5)->GetObject<MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (4)->GetObject<MobilityModel> (), m_nodes.Get (6)->GetObject<MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (5)->GetObject<MobilityModel> (), m_nodes.Get (7)->GetObject<MobilityModel> (), enabled);
  matrix->SetLoss (m_nodes.Get (6)->GetObject<MobilityModel> (), m_nodes.Get (8)->GetObject<MobilityModel> (), enabled);
  channelHelper.SetDeterministicPropagationLoss (matrix);
  m_channel = channelHelper.Create ();
}

void
LrrMcastXTest::CheckGraph ()
{
  std::ostringstream os;
  m_graph->PrintGraph (os);
  NS_TEST_ASSERT_MSG_EQ (os.str (), m_correctGraph, "Graph was created as expected");
}

void
LrrMcastXTest::CheckMulticastPath ()
{
  //Multicast group {0,3,6,7}, {2,4,5,8}
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.1", "227.0.0.1"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.2", "227.0.0.1"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.3", "227.0.0.1"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.4", "227.0.0.1"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.5", "227.0.0.1"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.6", "227.0.0.1"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.7", "227.0.0.1"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.8", "227.0.0.1"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.9", "227.0.0.1"), false, "HaveMcastPath() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.1", "227.0.0.2"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.2", "227.0.0.2"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.3", "227.0.0.2"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.4", "227.0.0.2"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.5", "227.0.0.2"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.6", "227.0.0.2"), true, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.7", "227.0.0.2"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.8", "227.0.0.2"), false, "HaveMcastPath() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->HaveMcastPath ("10.0.0.9", "227.0.0.2"), true, "HaveMcastPath() works as expected");
}

void
LrrMcastXTest::CheckMembers ()
{
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.1", "227.0.0.1"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.2", "227.0.0.1"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.3", "227.0.0.1"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.4", "227.0.0.1"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.5", "227.0.0.1"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.6", "227.0.0.1"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.7", "227.0.0.1"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.8", "227.0.0.1"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.9", "227.0.0.1"), false, "IsInMulticastGroup () works as expected");

  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.1", "227.0.0.2"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.2", "227.0.0.2"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.3", "227.0.0.2"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.4", "227.0.0.2"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.5", "227.0.0.2"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.6", "227.0.0.2"), true, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.7", "227.0.0.2"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.8", "227.0.0.2"), false, "IsInMulticastGroup () works as expected");
  NS_TEST_ASSERT_MSG_EQ (GlobalGroupManagement::GetInstance ()->IsInMcastGroup ("10.0.0.9", "227.0.0.2"), true, "IsInMulticastGroup () works as expected");
}

void
LrrMcastXTest::CheckRetranslators ()
{
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.1", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.1", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.1", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.1", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.1", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.2", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.3", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.4", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.4", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.4", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.4", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.4", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.5", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.6", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.7", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.7", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.7", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.7", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.7", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.8", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.8", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.8", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.8", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.8", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.8", "227.0.0.1"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.8", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.8", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.8", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.9", "227.0.0.1"), false, "IsMcastRetranslator() works as expected");
  // Check second group retranslators (full check:)
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.1", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.2", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.3", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.3", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.3", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.3", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.3", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.3", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.3", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.3", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.3", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.4", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.5", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.5", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.5", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.5", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.5", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.5", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.5", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.5", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.5", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.6", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.6", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.6", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.6", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.6", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.6", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.6", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.6", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.6", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.7", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.8", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");

  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.1", "10.0.0.9", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.2", "10.0.0.9", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.3", "10.0.0.9", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.4", "10.0.0.9", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.5", "10.0.0.9", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.6", "10.0.0.9", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.7", "10.0.0.9", "227.0.0.2"), true, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.8", "10.0.0.9", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
  NS_TEST_ASSERT_MSG_EQ (m_graph->IsMcastRetranslator ("10.0.0.9", "10.0.0.9", "227.0.0.2"), false, "IsMcastRetranslator() works as expected");
}

void
LrrMcastXTest::DoRun ()
{
  //Config::SetGlobal ("RngRun", IntegerValue (123));
  CreateNodes ();
  // Prepare stack:
  InstallChannel ();
  InstallDevices ();
  InstallInternet ();
  std::set <uint32_t> group;
  group.insert (0);
  group.insert (3);
  group.insert (6);
  group.insert (7);
  AddGroup (group);
  group.clear ();
  group.insert (2);
  group.insert (4);
  group.insert (5);
  group.insert (8);
  AddGroup (group);
  m_graph->Start ();
  //Check stack:
  CheckGraph ();
  CheckMulticastPath ();
  CheckMembers ();
  CheckRetranslators ();
  // Start simulation part of test:
  CreateSockets ();
  for (unsigned int i = 0; i < m_nodes.GetN (); i++)
    {
      Time at = Seconds (10.0 + 0.5 * (double)i);
      // Members are: 0, 3, 6, 7
      Simulator::Schedule (at, &LrrMcastXTest::Send, this, i, Ipv4Address ("227.0.0.1"));
      at = Seconds (20.0 + 0.5 * (double)i);
      // Members are: 2, 4, 5, 8
      Simulator::Schedule (at, &LrrMcastXTest::Send, this, i, Ipv4Address ("227.0.0.2"));
    }
  // Check first group:
  std::vector <uint32_t> reference;
  reference.push_back (3);
  reference.push_back (0);
  reference.push_back (0);
  reference.push_back (3);
  reference.push_back (0);
  reference.push_back (0);
  reference.push_back (3);
  reference.push_back (3);
  reference.push_back (0);
  Simulator::Schedule (Seconds (19), &LrrMcastXTest::CheckReceive, this, reference);
  reference.clear ();
  // Check second group:
  reference.push_back (0);
  reference.push_back (0);
  reference.push_back (3);
  reference.push_back (0);
  reference.push_back (3);
  reference.push_back (3);
  reference.push_back (0);
  reference.push_back (0);
  reference.push_back (3);
  Simulator::Schedule (Seconds (29), &LrrMcastXTest::CheckReceive, this, reference);
  RunSimulator ();
  DestroyTest ();
}

class LrrMcastTestSuite : public ns3::TestSuite
{
public:
  LrrMcastTestSuite () : ns3::TestSuite ("lrr-mcast-test", UNIT)
  {
    AddTestCase (new LrrMcastColumnTest, TestCase::EXTENSIVE);
    AddTestCase (new LrrMcastXTest, TestCase::EXTENSIVE);
  }
} g_lrrMcastTestSuite;

}
