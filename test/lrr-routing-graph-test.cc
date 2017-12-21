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

#include "ns3/test.h"

#include "ns3/lrr-routing-topology.h"
#include "ns3/lrr-routing-graph.h"

#include "ns3/lrr-channel-helper.h"
#include "ns3/lrr-device-helper.h"
#include "ns3/lrr-routing-helper.h"
#include "ns3/wifi-spectrum-value-helper.h"

#include "ns3/constant-position-mobility-model.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/boolean.h"

using namespace ns3;
using namespace lrr;

struct LrrTopologyTest : public ns3::TestCase
{
  LrrTopologyTest () : ns3::TestCase ("lrr-routing-graph test") {}
  void DoRun ()
  {
    Ptr<GlobalTopology> graph = Create<GlobalTopology> ();

    /**
     * Created graph
     *
     * 1------2------3
     * |      |      |
     * |      |      |
     * 4------5------6             10
     *        |      |
     *        |      |
     *        7------8------9
     */

    graph->AddVertex ("10.0.0.1");
    graph->AddVertex ("10.0.0.2");
    graph->AddVertex ("10.0.0.3");
    graph->AddVertex ("10.0.0.4");
    graph->AddVertex ("10.0.0.5");
    graph->AddVertex ("10.0.0.6");
    graph->AddVertex ("10.0.0.7");
    graph->AddVertex ("10.0.0.8");
    graph->AddVertex ("10.0.0.9");
    graph->AddVertex ("10.0.0.10");

    std::map<Ipv4Address, uint16_t> edges_1;
    edges_1.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.2", 1));
    edges_1.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.4", 1));
    graph->SetEdges ("10.0.0.1", edges_1);

    std::map<Ipv4Address, uint16_t> edges_2;
    edges_2.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.1", 1));
    edges_2.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.5", 1));
    edges_2.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.3", 1));
    graph->SetEdges ("10.0.0.2", edges_2);

    std::map<Ipv4Address, uint16_t> edges_3;
    edges_3.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.2", 1));
    edges_3.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.6", 1));
    graph->SetEdges ("10.0.0.3", edges_3);

    std::map<Ipv4Address, uint16_t> edges_4;
    edges_4.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.1", 1));
    edges_4.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.5", 1));
    graph->SetEdges ("10.0.0.4", edges_4);

    std::map<Ipv4Address, uint16_t> edges_5;
    edges_5.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.2", 1));
    edges_5.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.4", 1));
    edges_5.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.6", 1));
    edges_5.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.7", 1));
    graph->SetEdges ("10.0.0.5", edges_5);

    std::map<Ipv4Address, uint16_t> edges_6;
    edges_6.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.3", 1));
    edges_6.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.5", 1));
    edges_6.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.8", 1));
    graph->SetEdges ("10.0.0.6", edges_6);

    std::map<Ipv4Address, uint16_t> edges_7;
    edges_7.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.5", 1));
    edges_7.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.8", 1));
    graph->SetEdges ("10.0.0.7", edges_7);

    std::map<Ipv4Address, uint16_t> edges_8;
    edges_8.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.6", 1));
    edges_8.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.7", 1));
    edges_8.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.9", 1));
    graph->SetEdges ("10.0.0.8", edges_8);

    std::map<Ipv4Address, uint16_t> edges_9;
    edges_9.insert (std::pair<Ipv4Address, uint16_t> ("10.0.0.8", 1));
    graph->SetEdges ("10.0.0.9", edges_9);

    std::map<Ipv4Address, uint16_t> edges_10;
    graph->SetEdges ("10.0.0.10", edges_10);

    const char * correct =
      "# Network topology as seen by vertex 10.0.0.1\n"
      "digraph G {\n"
      "\t\"10.0.0.1\" [label=\"10.0.0.1\", shape=box];\n"
      "\t\"10.0.0.1\" -> \"10.0.0.2\" [label=\"1\"];\n"
      "\t\"10.0.0.1\" -> \"10.0.0.4\" [label=\"1\"];\n"
      "\t\"10.0.0.2\" [label=\"10.0.0.2\"];\n"
      "\t\"10.0.0.2\" -> \"10.0.0.1\" [label=\"1\"];\n"
      "\t\"10.0.0.2\" -> \"10.0.0.3\" [label=\"1\"];\n"
      "\t\"10.0.0.2\" -> \"10.0.0.5\" [label=\"1\"];\n"
      "\t\"10.0.0.3\" [label=\"10.0.0.3\"];\n"
      "\t\"10.0.0.3\" -> \"10.0.0.2\" [label=\"1\"];\n"
      "\t\"10.0.0.3\" -> \"10.0.0.6\" [label=\"1\"];\n"
      "\t\"10.0.0.4\" [label=\"10.0.0.4\"];\n"
      "\t\"10.0.0.4\" -> \"10.0.0.1\" [label=\"1\"];\n"
      "\t\"10.0.0.4\" -> \"10.0.0.5\" [label=\"1\"];\n"
      "\t\"10.0.0.5\" [label=\"10.0.0.5\"];\n"
      "\t\"10.0.0.5\" -> \"10.0.0.2\" [label=\"1\"];\n"
      "\t\"10.0.0.5\" -> \"10.0.0.4\" [label=\"1\"];\n"
      "\t\"10.0.0.5\" -> \"10.0.0.6\" [label=\"1\"];\n"
      "\t\"10.0.0.5\" -> \"10.0.0.7\" [label=\"1\"];\n"
      "\t\"10.0.0.6\" [label=\"10.0.0.6\"];\n"
      "\t\"10.0.0.6\" -> \"10.0.0.3\" [label=\"1\"];\n"
      "\t\"10.0.0.6\" -> \"10.0.0.5\" [label=\"1\"];\n"
      "\t\"10.0.0.6\" -> \"10.0.0.8\" [label=\"1\"];\n"
      "\t\"10.0.0.7\" [label=\"10.0.0.7\"];\n"
      "\t\"10.0.0.7\" -> \"10.0.0.5\" [label=\"1\"];\n"
      "\t\"10.0.0.7\" -> \"10.0.0.8\" [label=\"1\"];\n"
      "\t\"10.0.0.8\" [label=\"10.0.0.8\"];\n"
      "\t\"10.0.0.8\" -> \"10.0.0.6\" [label=\"1\"];\n"
      "\t\"10.0.0.8\" -> \"10.0.0.7\" [label=\"1\"];\n"
      "\t\"10.0.0.8\" -> \"10.0.0.9\" [label=\"1\"];\n"
      "\t\"10.0.0.9\" [label=\"10.0.0.9\"];\n"
      "\t\"10.0.0.9\" -> \"10.0.0.8\" [label=\"1\"];\n"
      "\t\"10.0.0.10\" [label=\"10.0.0.10\"];\n"
      "};\n"
    ;
    std::ostringstream os;
    graph->Print (os);
    NS_TEST_ASSERT_MSG_EQ (os.str (), correct, "AddVertex() and AddEdges() works as expected");

    graph->FloydWarshal ();

    graph->Print (std::cout);
    // ---------- HavePath() test ----------

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.1", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.2", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.3", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.4", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.5", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.6", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.7", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.8", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.1"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.2"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.3"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.4"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.5"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.6"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.7"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.8"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.9"), true, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.9", "10.0.0.10"), false, "HavePath() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.1"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.2"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.3"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.4"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.5"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.6"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.7"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.8"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.9"), false, "HavePath() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->HavePath ("10.0.0.10", "10.0.0.10"), true, "HavePath() works as expected");

    // ---------- PathDistance() test ----------

    uint32_t Infty = (uint32_t)(-1);

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.1"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.2"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.3"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.4"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.5"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.6"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.7"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.8"), 4, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.9"), 5, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.1"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.2"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.3"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.4"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.5"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.6"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.7"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.8"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.9"), 4, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.2", "10.0.0.10"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.1", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.1"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.2"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.3"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.4"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.5"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.6"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.7"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.8"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.9"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.3", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.1"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.2"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.3"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.4"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.5"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.6"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.7"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.8"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.9"), 4, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.4", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.1"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.2"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.3"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.4"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.5"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.6"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.7"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.8"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.9"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.5", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.1"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.2"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.3"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.4"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.5"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.6"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.7"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.8"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.9"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.6", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.1"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.2"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.3"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.4"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.5"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.6"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.7"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.8"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.9"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.7", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.1"), 4, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.2"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.3"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.4"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.5"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.6"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.7"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.8"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.9"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.8", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.1"), 5, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.2"), 4, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.3"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.4"), 4, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.5"), 3, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.6"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.7"), 2, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.8"), 1, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.9"), 0, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.9", "10.0.0.10"), Infty, "PathDistance() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.1"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.2"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.3"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.4"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.5"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.6"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.7"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.8"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.9"), Infty, "PathDistance() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->PathDistance ("10.0.0.10", "10.0.0.10"), 0, "PathDistance() works as expected");

    // ---------- GetNextHop() test ----------

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.1"), "10.0.0.1", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.2"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.3"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.4"), "10.0.0.4", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.5"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.6"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.7"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.8"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.9"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.1", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.1"), "10.0.0.1", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.2"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.3"), "10.0.0.3", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.4"), "10.0.0.1", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.5"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.6"), "10.0.0.3", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.7"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.8"), "10.0.0.3", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.9"), "10.0.0.3", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.2", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.1"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.2"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.3"), "10.0.0.3", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.4"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.5"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.6"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.7"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.8"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.9"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.3", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.1"), "10.0.0.1", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.2"), "10.0.0.1", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.3"), "10.0.0.1", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.4"), "10.0.0.4", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.5"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.6"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.7"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.8"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.9"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.4", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.1"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.2"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.3"), "10.0.0.2", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.4"), "10.0.0.4", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.5"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.6"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.7"), "10.0.0.7", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.8"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.9"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.5", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.1"), "10.0.0.3", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.2"), "10.0.0.3", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.3"), "10.0.0.3", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.4"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.5"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.6"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.7"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.8"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.9"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.6", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.1"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.2"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.3"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.4"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.5"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.6"), "10.0.0.5", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.7"), "10.0.0.7", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.8"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.9"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.7", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.1"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.2"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.3"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.4"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.5"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.6"), "10.0.0.6", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.7"), "10.0.0.7", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.8"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.9"), "10.0.0.9", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.8", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.1"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.2"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.3"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.4"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.5"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.6"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.7"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.8"), "10.0.0.8", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.9"), "10.0.0.9", "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.9", "10.0.0.10"), Ipv4Address (), "GetNextHop() works as expected");

    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.1"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.2"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.3"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.4"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.5"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.6"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.7"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.8"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.9"), Ipv4Address (), "GetNextHop() works as expected");
    NS_TEST_ASSERT_MSG_EQ (graph->GetNextHop ("10.0.0.10", "10.0.0.10"), "10.0.0.10", "GetNextHop() works as expected");
  }
};
class LrrGraphTest : public ns3::TestCase
{
public:
  LrrGraphTest () : ns3::TestCase ("LRR-Global graph test") {}
  void DoRun ();
};
void
LrrGraphTest::DoRun (void)
{
  double totalTime = 20; // in seconds

  // Create nodes (nCount nodes are sources and one is receiver)
  uint32_t nNodes = 5;
  NodeContainer nodes;
  nodes.Create (nNodes);

  // Setup channel

  for (uint32_t i = 0; i < nNodes; ++i)
    {
      Ptr<Node> node = nodes.Get (i);
      Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();
      mobility->SetPosition (Vector (0.0, 0.0, 0.0));
      node->AggregateObject (mobility);
    }
  LrrChannelHelper channelHelper = LrrChannelHelper::Default ();
  Ptr<MatrixPropagationLossModel> matrix = CreateObject<MatrixPropagationLossModel> ();
  double disabled = 220; // -200 dBm
  double enabled = 60; // -40 dBm
  matrix->SetDefaultLoss (disabled);
  matrix->SetLoss (nodes.Get (0)->GetObject <MobilityModel> (), nodes.Get (1)->GetObject <MobilityModel> (), enabled);
  matrix->SetLoss (nodes.Get (1)->GetObject <MobilityModel> (), nodes.Get (2)->GetObject <MobilityModel> (), enabled);
  matrix->SetLoss (nodes.Get (2)->GetObject <MobilityModel> (), nodes.Get (3)->GetObject <MobilityModel> (), enabled);
  matrix->SetLoss (nodes.Get (3)->GetObject <MobilityModel> (), nodes.Get (4)->GetObject <MobilityModel> (), enabled);
  channelHelper.SetDeterministicPropagationLoss (matrix);
  //channelHelper.SetPropagationDelay (CreateObject<ConstantSpeedPropagationDelayModel> ());
  // Configure node devices
  NeighborAwareDeviceHelper deviceHelper;
  deviceHelper.SetChannel (channelHelper.Create ());
  // Set modulation and noise:
  WifiSpectrumValue5MhzFactory sf;
  deviceHelper.SetTxPowerSpectralDensity (sf.CreateTxPowerSpectralDensity (0.1 /*Watts*/, 1 /*channel number*/));
  deviceHelper.SetNoisePowerSpectralDensity (sf.CreateConstant (1.381e-23 * 290 /*kT*/));
  deviceHelper.SetRxFilter (sf.CreateRfFilter (5));
  NetDeviceContainer nodeDevices = deviceHelper.Install (nodes);

  LrrRoutingHelper routingHelper;
  // Install Internet stack
  InternetStackHelper internet;
  internet.SetRoutingHelper (routingHelper);
  internet.Install (nodes);
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipAddrs.Assign (nodeDevices);

  lrr::GlobalGraph * graph = GlobalGraph::Instance ();
  graph->Start ();
  graph->PrintGraph (std::cout);
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  graph->Stop ();
  GlobalGraph::Destroy ();
  NS_TEST_EXPECT_MSG_EQ_TOL (1.0 / 1.0, 1.0, 0.01, "Known throughput for nCount stations");
}

class LrrGraphTestSuite : public ns3::TestSuite
{
public:
  LrrGraphTestSuite () : ns3::TestSuite ("lrr-routing-graph", UNIT)
  {
    AddTestCase (new LrrGraphTest, TestCase::QUICK);
    AddTestCase (new LrrTopologyTest, TestCase::QUICK);
  }
} g_lrrGraphTestSuite;

