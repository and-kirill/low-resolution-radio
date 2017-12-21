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

#include "ns3/core-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
#include "ns3/mobility-module.h"
#include "ns3/low-resolution-radio-module.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/olsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/average.h"

#include "video-application.h"

using namespace ns3;

class VideoExample
{
public:
  enum RadioType
  {
    RADIO_WIFI,
    RADIO_LRR
  };
  enum RoutingType
  {
    ROUTING_OLSR,
    ROUTING_LRR
  };
  struct Parameters
  {
    ///\name Description of nodes and their mobility:
    ///\{
    std::string m_ns2MobilityTrace;
    uint32_t m_nodeCount;
    ///\}
    ///\name Description of a traffic: Several nodes send a video to surveillance center
    ///\{
    uint32_t m_surveillanceNode;
    std::vector<uint32_t> m_videoSources;
    ///\}
    ///\name Type of radio and routing used in this scenario:
    ///\{
    RadioType m_radioType;
    RoutingType m_routingType;
    ///\}
    double m_timeSeconds;
    double m_warmUpSeconds;
    Ptr<UniformRandomVariable> m_appStartTime;
    Parameters () :
      m_ns2MobilityTrace ("scenario.ns_movements"),
      m_nodeCount (2),
      m_surveillanceNode (0),
      m_videoSources (std::vector <uint32_t> ()),
      m_radioType (RADIO_LRR),
      m_routingType (ROUTING_LRR),
      m_timeSeconds (900),
      m_warmUpSeconds (300),
      m_appStartTime (CreateObject<UniformRandomVariable> ())
    {
    }
  };
  VideoExample (Parameters params);
  void Run ();
private:
  void CreateNodes ();
  void InstallWifiStack ();
  void InstallRadioStack ();
  void InstallInternetStack ();
  void InstallApplications ();
  void InstallFlowMonitor ();
  ///\name Trace sinks:
  ///\{
  void MacTransmissionError (std::string context, Address dst, Address src);
  void OlsrDropped (std::string, Ipv4Address, Ipv4Address);
  ///\}
private:
  Parameters m_parameters;
  double m_txPowerDbm;
  /// Internals:
  NodeContainer m_nodes;
  NetDeviceContainer m_devices;
  Ipv4InterfaceContainer m_interfaces;
  Ptr<PropagationLossModel> m_propagationLossModel;
  FlowMonitorHelper m_flowMonitorHelper;
  Ptr<FlowMonitor> m_flowMonitor;
};

VideoExample::VideoExample (Parameters params) :
  m_parameters (params),
  m_txPowerDbm (16),
  m_propagationLossModel (CreateObject<LogDistancePropagationLossModel> ())
{
  ///\name Settings:
  ///\{
  // Rate of LRR PHY is 6 MBs as in WiFi:
  Config::SetDefault ("ns3::HalfDuplexIdealPhy::Rate", StringValue ("6Mbps"));
  // Take into account preamble length:
  Config::SetDefault ("ns3::lrr::AccessManager::GuardInterval", TimeValue (MicroSeconds (34 + 20 + 16 + 44 /*DIFS + preamble + SIFS + ACK*/)));
  // Change OLSR defaults due to high mobility:
  Config::SetDefault ("ns3::olsr::RoutingProtocol::HelloInterval", TimeValue (Seconds (0.5)));
  Config::SetDefault ("ns3::olsr::RoutingProtocol::TcInterval", TimeValue (Seconds (1.25)));
  ///\}
  m_propagationLossModel->SetAttribute ("Exponent", DoubleValue (3));
  CreateNodes ();
  switch (m_parameters.m_radioType)
    {
    case RADIO_WIFI:
      std::cout << "Install WiFi PHY and MAC" << std::endl;
      InstallWifiStack ();
      break;
    case RADIO_LRR:
      std::cout << "Install RADIO PHY and MAC" << std::endl;
      InstallRadioStack ();
      break;
    default:
      NS_FATAL_ERROR ("Unknown type of radio stack");
      break;
    }
  InstallInternetStack ();
  InstallApplications ();
  InstallFlowMonitor ();
}

void
VideoExample::CreateNodes ()
{
  m_nodes.Create (m_parameters.m_nodeCount);
  Ns2MobilityHelper mobility (m_parameters.m_ns2MobilityTrace);
  mobility.Install ();
}

void
VideoExample::InstallRadioStack ()
{
  // Create channel:
  LrrChannelHelper channelHelper = LrrChannelHelper::Default ();
  channelHelper.SetDeterministicPropagationLoss (m_propagationLossModel);
  Ptr<lrr::NeighborAwareSpectrumChannel> channel = channelHelper.Create ();

  NeighborAwareDeviceHelper deviceHelper;
  deviceHelper.SetChannel (channel);

  // Crete TX power spectral density and noise power spectral densityL
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
  // Install devices on nodes:
  m_devices = deviceHelper.Install (m_nodes);
}

void
VideoExample::InstallWifiStack ()
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
VideoExample::InstallInternetStack ()
{
  InternetStackHelper internet;
  switch (m_parameters.m_routingType)
    {
    case ROUTING_OLSR:
      {
        OlsrHelper routing;
        internet.SetRoutingHelper (routing);
      }
      break;
    case ROUTING_LRR:
      {
        LrrRoutingHelper routing;
        internet.SetRoutingHelper (routing);
      }
      break;
    default:
      NS_FATAL_ERROR ("Specify correct routing type!");
      break;
    }
  internet.Install (m_nodes);
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("10.0.0.0", "255.255.255.0");
  m_interfaces = ipAddrs.Assign (m_devices);
}

void
VideoExample::InstallApplications ()
{
  uint16_t port = 6666;
  Ipv4Address dst = m_interfaces.GetAddress (m_parameters.m_surveillanceNode);
  for (unsigned int i = 0; i < m_parameters.m_videoSources.size (); i++)
    {
      Ptr<VideoApplication> video = CreateObject<VideoApplication> ();
      video->SetPeerAddress (InetSocketAddress (dst, port));
      video->SetStartTime (Seconds (m_parameters.m_warmUpSeconds + m_parameters.m_appStartTime->GetValue ()));
      video->SetStopTime (Seconds (m_parameters.m_timeSeconds));
      m_nodes.Get (m_parameters.m_videoSources[i])->AddApplication (video);
    }
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (m_nodes.Get (m_parameters.m_surveillanceNode));
}

void
VideoExample::Run ()
{
  // Connect traces:
  Config::Connect ("/NodeList/*/DeviceList/*/Mac/RxError", MakeCallback (&VideoExample::MacTransmissionError, this));
  Config::Connect ("/NodeList/*/$ns3::olsr::RoutingProtocol/NoRouteToHost", MakeCallback (&VideoExample::OlsrDropped, this));

  Simulator::Stop (Seconds (m_parameters.m_timeSeconds));
  if (m_parameters.m_routingType == ROUTING_LRR)
    {
      lrr::GlobalGraph::Instance ()->Start ();
    }
  Simulator::Run ();
  m_flowMonitor->CheckForLostPackets (Seconds (m_parameters.m_timeSeconds));
  m_flowMonitor->Start (Seconds (m_parameters.m_warmUpSeconds));
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (m_flowMonitorHelper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = m_flowMonitor->GetFlowStats ();
  std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i;
  Average<double> pdr;  // %-s.
  Average<double> delay;// seconds.
  for (i = stats.begin (); i != stats.end (); i++)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.destinationAddress == Ipv4Address ("10.0.0.1"))
        {
          pdr.Update ((double)i->second.rxPackets / (double) i->second.txPackets * 100.0);
          delay.Update (i->second.delaySum.GetSeconds () / (double)i->second.rxPackets);
        }
    }
  std::cout << "PDR:  " << pdr.Mean ()   << std::endl;
  std::cout << "Delay:" << delay.Mean () << std::endl;
  Simulator::Destroy ();
}

void
VideoExample::InstallFlowMonitor ()
{
  m_flowMonitor = m_flowMonitorHelper.InstallAll ();
}

void
VideoExample::MacTransmissionError (std::string context, Address dst, Address source)
{
  std::cout << "Transmission error happened! Source = " << Mac48Address::ConvertFrom (source) << ", destination = " << Mac48Address::ConvertFrom (dst) << std::endl;
}

void
VideoExample::OlsrDropped (std::string, Ipv4Address src, Ipv4Address dst)
{
  std::cout << "OLSR: no route to host! Source = " << src << ", destination = " << dst << std::endl;
}

int main (int argc, char *argv[])
{
  VideoExample::Parameters scenarioParameters;
  std::string routingType = "OLSR";
  std::string radioType = "LRR";
  CommandLine cmd;
  cmd.AddValue ("radio", "Type of radio access method. Possible values are: WiFi, LRR", radioType);
  cmd.AddValue ("routing", "Type of routing. Possible values are: OLSR, LRR", routingType);
  cmd.AddValue ("time", "Total simulation time [s]", scenarioParameters.m_timeSeconds);
  cmd.AddValue ("warming", "Time, which is not taken into consideration ('warming') [s]", scenarioParameters.m_warmUpSeconds);
  cmd.Parse (argc, argv);
  // Set approrpiate routing type:
  if (routingType == "OLSR")
    {
      scenarioParameters.m_routingType = VideoExample::ROUTING_OLSR;
    }
  else if (routingType == "LRR")
    {
      scenarioParameters.m_routingType = VideoExample::ROUTING_LRR;
    }
  else
    {
      NS_FATAL_ERROR ("Specify routing type correctly! Use --help option. " << routingType);
    }
  // Set appropriate device type:
  if (radioType == "WiFi")
    {
      scenarioParameters.m_radioType = VideoExample::RADIO_WIFI;
    }
  else if (radioType == "LRR")
    {
      scenarioParameters.m_radioType = VideoExample::RADIO_LRR;
    }
  else
    {
      NS_FATAL_ERROR ("Specify radio type correctly! Use --help option. " << radioType);
    }
  scenarioParameters.m_surveillanceNode = 0;
  scenarioParameters.m_videoSources.push_back (40);
  scenarioParameters.m_videoSources.push_back (41);
  scenarioParameters.m_videoSources.push_back (42);
  scenarioParameters.m_videoSources.push_back (43);
  scenarioParameters.m_videoSources.push_back (44);
  scenarioParameters.m_videoSources.push_back (45);
  scenarioParameters.m_videoSources.push_back (46);
  scenarioParameters.m_videoSources.push_back (47);
  scenarioParameters.m_videoSources.push_back (48);
  scenarioParameters.m_videoSources.push_back (49);
  scenarioParameters.m_videoSources.push_back (50);
  scenarioParameters.m_videoSources.push_back (51);
  scenarioParameters.m_videoSources.push_back (52);
  scenarioParameters.m_videoSources.push_back (53);
  scenarioParameters.m_videoSources.push_back (54);

  scenarioParameters.m_ns2MobilityTrace = "DA.ns_movements";
  scenarioParameters.m_nodeCount = 60;
  VideoExample example (scenarioParameters);
  example.Run ();
  return 0;
}
