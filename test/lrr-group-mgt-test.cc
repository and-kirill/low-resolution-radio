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
 * Authors: Kirill Andreev <k.andreev@skoltech.ru>
 */

#include "ns3/test.h"
#include "ns3/lrr-mcast-group-mgt.h"

using namespace ns3;

class LrrGroupManagementTestCase : public ns3::TestCase
{
public:
  LrrGroupManagementTestCase () : ns3::TestCase ("Global group management test") {}
  void DoRun ();
};

void
LrrGroupManagementTestCase::DoRun ()
{
  /// Test add group:
  std::set<Ipv4Address> group;
  group.insert (Ipv4Address ("192.168.0.1"));
  group.insert (Ipv4Address ("192.168.0.2"));
  group.insert (Ipv4Address ("192.168.0.3"));
  group.insert (Ipv4Address ("192.168.0.4"));

  lrr::GlobalGroupManagement::GetInstance ()->AddMcastGroup (Ipv4Address ("227.0.0.1"), group);
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.4"), Ipv4Address ("227.0.0.1")), true, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.4"), Ipv4Address ("227.0.0.2")), false, "Check members");
  group.clear ();
  /// Add second group:
  group.insert (Ipv4Address ("192.168.0.7"));
  group.insert (Ipv4Address ("192.168.0.6"));
  group.insert (Ipv4Address ("192.168.0.3"));
  group.insert (Ipv4Address ("192.168.0.4"));
  lrr::GlobalGroupManagement::GetInstance ()->AddMcastGroup (Ipv4Address ("227.0.0.2"), group);
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.4"), Ipv4Address ("227.0.0.1")), true, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.4"), Ipv4Address ("227.0.0.2")), true, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.7"), Ipv4Address ("227.0.0.2")), true, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.7"), Ipv4Address ("227.0.0.1")), false, "Check members");
  /// Now remove group:
  lrr::GlobalGroupManagement::GetInstance ()->RemoveMulticastgroup (Ipv4Address ("227.0.0.2"));
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.4"), Ipv4Address ("227.0.0.2")), false, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.7"), Ipv4Address ("227.0.0.2")), false, "Check members");
  /// Now overwrite first group:
  group.clear ();
  group.insert (Ipv4Address ("192.168.0.11"));
  group.insert (Ipv4Address ("192.168.0.12"));
  group.insert (Ipv4Address ("192.168.0.13"));
  group.insert (Ipv4Address ("192.168.0.14"));
  lrr::GlobalGroupManagement::GetInstance ()->AddMcastGroup (Ipv4Address ("227.0.0.1"), group);
  /// Old group is erased:
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.1"), Ipv4Address ("227.0.0.1")), false, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.2"), Ipv4Address ("227.0.0.1")), false, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.3"), Ipv4Address ("227.0.0.1")), false, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.4"), Ipv4Address ("227.0.0.1")), false, "Check members");
  /// New group is present:
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.11"), Ipv4Address ("227.0.0.1")), true, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.12"), Ipv4Address ("227.0.0.1")), true, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.13"), Ipv4Address ("227.0.0.1")), true, "Check members");
  NS_TEST_ASSERT_MSG_EQ (lrr::GlobalGroupManagement::GetInstance ()->IsInMcastGroup (Ipv4Address ("192.168.0.14"), Ipv4Address ("227.0.0.1")), true, "Check members");
  lrr::GlobalGroupManagement::Destroy ();
}

class LrrGroupManagementTest : public ns3::TestSuite
{
public:
  LrrGroupManagementTest () : ns3::TestSuite ("lrr-group-management-test", UNIT)
  {
    AddTestCase (new LrrGroupManagementTestCase, TestCase::QUICK);
  }
} g_lrrGroupManagementTest;
