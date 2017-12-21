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
 * Author: Kirill Andreev <k.andreev@skoltech.ru>
 */

#include "ns3/low-resolution-radio-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/core-module.h"
#include "ns3/string.h"

#include "ns3/mac48-address.h"
#include "ns3/wifi-spectrum-value-helper.h"
/// Mobility:
#include "ns3/constant-position-mobility-model.h"
/// Propagation:
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/double.h"
/// Headers needed to install internet:
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
/// Applications:
#include "ns3/bulk-send-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/v4ping-helper.h"
#include "ns3/application-container.h"
#include "ns3/boolean.h"

#include "ns3/simulator.h"

using namespace ns3;

class LrrWifiComparisonExample
{
public:
  LrrWifiComparisonExample (bool enableWifi, double stepMeters);
  void Run ();
private:
  void CreateNodes ();
  void InstallWifiStack ();
  void InstallRadioStack ();
  void InstallInternetStack ();
  void InstallApplications ();
  void InstallFlowMonitor ();
private:
  ///Parameters:
  uint32_t m_nodeCount;
  double m_step;
  double m_totalTime;
  double m_txPowerDbm;
  double m_startTime;
  /// Internals:
  NodeContainer m_nodes;
  NetDeviceContainer m_devices;
  Ipv4InterfaceContainer m_interfaces;
  Ptr<PropagationLossModel> m_propagationLossModel;
  FlowMonitorHelper m_flowMonitorHelper;
  Ptr<FlowMonitor> m_flowMonitor;
};

LrrWifiComparisonExample::LrrWifiComparisonExample (bool enableWifi, double stepMeters) :
  m_nodeCount (5),
  m_step (stepMeters),
  m_totalTime (900),
  m_txPowerDbm (16),
  m_startTime (50),
  m_propagationLossModel (CreateObject<TwoRayGroundPropagationLossModel> ())
{
  CreateNodes ();
  if (enableWifi)
    {
      std::cout << "Install WiFi PHY and MAC" << std::endl;
      InstallWifiStack ();
    }
  else
    {
      std::cout << "Install RADIO PHY and MAC" << std::endl;
      InstallRadioStack ();
    }
  InstallInternetStack ();
  InstallApplications ();
  InstallFlowMonitor ();
}

void
LrrWifiComparisonExample::CreateNodes ()
{
  m_nodes.Create (m_nodeCount);
  /// Set position for each node: chain topology
  for (unsigned int i = 0; i < m_nodeCount; i++)
    {
      Ptr<Node> node = m_nodes.Get (i);
      Ptr<ConstantPositionMobilityModel> model = CreateObject<ConstantPositionMobilityModel> ();
      node->AggregateObject (model);
      model->SetPosition (Vector (0, m_step * i, 2 /*Antenna height*/));
    }
}

void
LrrWifiComparisonExample::InstallRadioStack ()
{
  // 2. Create channel:
  LrrChannelHelper channelHelper = LrrChannelHelper::Default ();
  channelHelper.SetDeterministicPropagationLoss (m_propagationLossModel);
  Ptr<lrr::NeighborAwareSpectrumChannel> channel = channelHelper.Create ();
  Config::SetDefault ("ns3::HalfDuplexIdealPhy::Rate", StringValue ("6Mbps"));
  /// Preamble length:
  Config::SetDefault ("ns3::lrr::AccessManager::GuardInterval", TimeValue (MicroSeconds (34 + 20 + 16 + 44 /*DIFS + preamble + SIFS + ACK*/)));

  NeighborAwareDeviceHelper deviceHelper;
  deviceHelper.SetChannel (channel);

  //3. Crete TX power spectral density and noise power spectral densityL
  WifiSpectrumValue5MhzFactory sf;

  double txPowerW = pow (10, m_txPowerDbm / 10) / 1000;
  uint32_t channelNumber = 1;
  Ptr<SpectrumValue> txPsd = sf.CreateTxPowerSpectralDensity (txPowerW, channelNumber);

  const double k = 1.381e-23; //Boltzmann's constant
  const double T = 290; // temperature in Kelvin
  double noisePsdValue = k * T; // watts per hertz
  Ptr<SpectrumValue> noisePsd = sf.CreateConstant (noisePsdValue);
  Ptr<SpectrumValue> rxfilter = sf.CreateRfFilter (channelNumber + 4);

  deviceHelper.SetTxPowerSpectralDensity (txPsd);
  deviceHelper.SetNoisePowerSpectralDensity (noisePsd);
  deviceHelper.SetRxFilter (rxfilter);

  //4. Install devices on nodes:
  m_devices = deviceHelper.Install (m_nodes);
}

void
LrrWifiComparisonExample::InstallWifiStack ()
{
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (0));
  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel> ();
  channel->SetPropagationLossModel (m_propagationLossModel);
  channel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());
  wifiPhy.SetChannel (channel);
  wifiPhy.Set ("TxPowerStart",DoubleValue (m_txPowerDbm));
  wifiPhy.Set ("TxPowerEnd",DoubleValue (m_txPowerDbm));
  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("OfdmRate6Mbps"),
                                "ControlMode", StringValue ("OfdmRate6Mbps"));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  m_devices = wifi.Install (wifiPhy, wifiMac, m_nodes);
}

void
LrrWifiComparisonExample::InstallInternetStack ()
{
  InternetStackHelper internet;
  OlsrHelper routing;
  internet.SetRoutingHelper (routing);
  internet.Install (m_nodes);
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("10.0.0.0", "255.255.255.0");
  m_interfaces = ipAddrs.Assign (m_devices);
}

void
LrrWifiComparisonExample::InstallApplications ()
{
  // Ping:
  V4PingHelper ping (m_interfaces.GetAddress (m_nodeCount - 1));
  ping.SetAttribute ("Verbose", BooleanValue (true));
  ApplicationContainer p = ping.Install (m_nodes.Get (0));
  p.Start (Seconds (0));
  p.Stop (Seconds (m_totalTime) - Seconds (0.001));
  // TCP:
  uint16_t port = 9;
  BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (m_interfaces.GetAddress (m_nodeCount - 1), port));
  std::cout << "Source:" << m_interfaces.GetAddress (1) << std::endl;
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (m_nodes.Get (0));
  sourceApps.Start (Seconds (m_startTime));
  sourceApps.Stop (Seconds (m_totalTime));

  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (m_nodes.Get (m_nodeCount -1));
  sinkApps.Start (Seconds (10.0));
  sinkApps.Stop (Seconds (m_totalTime));
}

void
LrrWifiComparisonExample::Run ()
{
  Simulator::Stop ( Seconds (m_totalTime));
  Simulator::Run ();
  m_flowMonitor->CheckForLostPackets (Seconds (m_totalTime));
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (m_flowMonitorHelper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = m_flowMonitor->GetFlowStats ();
  std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i;
  for (i = stats.begin (); i != stats.end (); i++)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flow " << i->first << " ( " << t.sourceAddress << " --> " << t.destinationAddress << " ) " << std::endl;
      std::cout << "Throughput     : " << i->second.rxBytes / (m_totalTime - m_startTime) * 8 / 1e6 << " Mbs."<< std::endl;
    }
  Simulator::Destroy ();
}

void
LrrWifiComparisonExample::InstallFlowMonitor ()
{
  m_flowMonitor = m_flowMonitorHelper.InstallAll ();
}

int main (int argc, char *argv[])
{
  double distance = 30;
  bool isWifi = false;
  CommandLine cmd;
  cmd.AddValue ("distance", "distance between nodes", distance);
  cmd.AddValue ("isWifi", "Wifi device is installed if true, radio device otherwise", isWifi);
  cmd.Parse (argc, argv);
  LrrWifiComparisonExample example (isWifi, distance);
  example.Run ();
  return 0;
}
