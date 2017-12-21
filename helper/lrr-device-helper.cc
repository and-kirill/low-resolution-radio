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
 * Author:  Kirill Andreev <k.andreev@skoltech.ru> written after AdhocAlohaNoackIdealPhyHelper by Nicola Baldo <nbaldo@cttc.es>
 */

#include "lrr-device-helper.h"

#include "ns3/mobility-model.h"
#include "ns3/mac48-address.h"
#include "ns3/string.h"
#include "ns3/names.h"
#include "ns3/queue.h"

#include "ns3/lrr-channel.h"
#include "ns3/lrr-range-error-model.h"
#include "ns3/lrr-phy.h"
namespace ns3
{

NeighborAwareDeviceHelper::NeighborAwareDeviceHelper () :
  m_txPsd (0),
  m_noisePsd (0),
  m_rxFilter (0)
{
  // Phy, device and MAC can not be changed now:
  m_phy.SetTypeId ("ns3::lrr::Phy");
  m_mac.SetTypeId ("ns3::lrr::CollisionFreeMacImpl");
  m_queue.SetTypeId ("ns3::DropTailQueue<Packet>");
  m_device.SetTypeId ("ns3::lrr::NeighborAwareDeviceImpl");
}

NeighborAwareDeviceHelper::~NeighborAwareDeviceHelper ()
{
  m_channel = 0;
  m_txPsd = 0;
  m_noisePsd = 0;
}

void
NeighborAwareDeviceHelper::SetChannel (Ptr<lrr::NeighborAwareSpectrumChannel> channel)
{
  m_channel = channel;
}

void
NeighborAwareDeviceHelper::SetType (std::string type,
                                    std::string n0, const AttributeValue &v0,
                                    std::string n1, const AttributeValue &v1,
                                    std::string n2, const AttributeValue &v2,
                                    std::string n3, const AttributeValue &v3,
                                    std::string n4, const AttributeValue &v4,
                                    std::string n5, const AttributeValue &v5,
                                    std::string n6, const AttributeValue &v6,
                                    std::string n7, const AttributeValue &v7)
{
  m_mac.SetTypeId (type);
  m_mac.Set (n0, v0);
  m_mac.Set (n1, v1);
  m_mac.Set (n2, v2);
  m_mac.Set (n3, v3);
  m_mac.Set (n4, v4);
  m_mac.Set (n5, v5);
  m_mac.Set (n6, v6);
  m_mac.Set (n7, v7);
}

Ptr<lrr::NeighborAwareDevice>
NeighborAwareDeviceHelper::CreateDevice (Ptr<Node> node, Address address)
{
  Ptr<lrr::Phy> phy = m_phy.Create<lrr::Phy> ();
  Ptr<lrr::Mac> mac = m_mac.Create<lrr::Mac> ();
  Ptr<Queue<Packet> > queue = m_queue.Create<Queue<Packet> > ();
  Ptr<lrr::NeighborAwareDeviceImpl> dev = m_device.Create<lrr::NeighborAwareDeviceImpl> ();

  NS_ASSERT (phy);
  NS_ASSERT (mac);
  NS_ASSERT (queue);
  NS_ASSERT (dev);
  NS_ASSERT_MSG (m_txPsd, "you forgot to call NeighborAwareDeviceHelper::SetTxPowerSpectralDensity ()");
  NS_ASSERT_MSG (m_noisePsd, "you forgot to call NeighborAwareDeviceHelper::SetNoisePowerSpectralDensity ()");
  NS_ASSERT_MSG (m_rxFilter, "you forgot to call NeighborAwareDeviceHelper::SetRxFilter ()");

  mac->SetPhy (phy);
  mac->SetQueue (queue);

  dev->SetMac (mac);
  dev->SetAddress (address);

  NS_ASSERT (node->GetObject<MobilityModel> () != 0);
  phy->SetMobility (node->GetObject<MobilityModel> ());
  phy->SetDevice (dev);
  phy->SetRxFilter (m_rxFilter);
  phy->SetTxPowerSpectralDensity (m_txPsd);
  phy->SetNoisePowerSpectralDensity (m_noisePsd);
  phy->SetGenericPhyRxEndOkCallback (MakeCallback (&lrr::Mac::Receive, mac));
  phy->SetErrorModel (CreateObject<lrr::RangeSpectrumErrorModel> ());

  m_channel->AddRx (phy);
  node->AddDevice (dev);
  return dev;
}

void
NeighborAwareDeviceHelper::SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
{
  m_txPsd = txPsd;
}

void
NeighborAwareDeviceHelper::SetNoisePowerSpectralDensity (Ptr<SpectrumValue> noisePsd)
{
  m_noisePsd = noisePsd;
}

void
NeighborAwareDeviceHelper::SetRxFilter (Ptr<SpectrumValue> rxFilter)
{
  m_rxFilter = rxFilter;
}

void
NeighborAwareDeviceHelper::SetPhyAttribute (std::string name, const AttributeValue &v)
{
  m_phy.Set (name, v);
}

void
NeighborAwareDeviceHelper::SetDeviceAttribute (std::string name, const AttributeValue &v)
{
  m_device.Set (name, v);
}

NetDeviceContainer
NeighborAwareDeviceHelper::Install (NodeContainer c)
{
  NetDeviceContainer retval;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      retval.Add (Install (*i));
    }
  return retval;
}

NetDeviceContainer
NeighborAwareDeviceHelper::Install (Ptr<Node> node)
{
  Ptr<lrr::NeighborAwareDevice> device = CreateDevice (node, Mac48Address::Allocate ());
  return NetDeviceContainer (device);
}

NetDeviceContainer
NeighborAwareDeviceHelper::Install (std::string nodeName)
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (node);
}

} // namespace ns3
