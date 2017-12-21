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

#ifndef RADIO_DEVICE_STACK_HELPER_H_
#define RADIO_DEVICE_STACK_HELPER_H_

#include "ns3/object.h"
#include "ns3/object-factory.h"
#include "ns3/lrr-device-impl.h"
#include "ns3/lrr-mac.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
namespace ns3
{
class NeighborAwareDeviceHelper
{
public:
  NeighborAwareDeviceHelper ();
  ~NeighborAwareDeviceHelper ();
  void SetChannel (Ptr<lrr::NeighborAwareSpectrumChannel> channel);
  /// Set type of MAC and some attributes:
  void SetType (std::string type,
                std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());
  /// Install radio device to a given nofe with a given channel ID, standard and address:
  Ptr<lrr::NeighborAwareDevice> CreateDevice (Ptr<Node> node, Address address);
  ///\name TX-power and noise of a device:
  ///\{
  void SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd);
  void SetNoisePowerSpectralDensity (Ptr<SpectrumValue> noisePsd);
  void SetRxFilter (Ptr<SpectrumValue> rxFilter);
  ///\}
  ///\name Set attributes:
  ///\{
  void SetPhyAttribute (std::string name, const AttributeValue &v);
  void SetDeviceAttribute (std::string n1, const AttributeValue &v1);
  ///\}
  ///\name Install radio device with HalfDuplexIdealPhy on a node:
  ///\{
  NetDeviceContainer Install (NodeContainer c);
  NetDeviceContainer Install (Ptr<Node> node);
  NetDeviceContainer Install (std::string nodeName);
  ///\}
private:
  ObjectFactory m_phy;
  ObjectFactory m_mac;
  ObjectFactory m_queue;
  ObjectFactory m_device;
  Ptr<lrr::NeighborAwareSpectrumChannel> m_channel;
  Ptr<SpectrumValue> m_txPsd;
  Ptr<SpectrumValue> m_noisePsd;
  Ptr<SpectrumValue> m_rxFilter;
};
}
#endif // RADIO_DEVICE_HELPER_H_
