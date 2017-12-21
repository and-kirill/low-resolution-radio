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
#include "ns3/v4ping-helper.h"
#include "ns3/application-container.h"
#include "ns3/boolean.h"

#include "ns3/simulator.h"

using namespace ns3;

int main ()
{
  /*
   * This example creates three nodes @step meters apart from each other, installs two radio-interface
   * on each node and tests ping between first and last nodes.
   *
   * Channel interference is shown: access manager resolves channel interference and communication neighbors
   * are those ones, who operate at the same frequency
   *
   * Internet stack with global routing are installed above radio stack.
   */
  ///\name Model Parameters:
  ///\{
  double step = 50;
  uint32_t nodeCount = 6;
  double totalTime = 15;
  ///\}
  // 1. Create and place nodes:
  NodeContainer nodes;
  nodes.Create (nodeCount);
  /// Set position for each node: chain topology
  for (unsigned int i = 0; i < nodeCount; i++)
    {
      Ptr<Node> node = nodes.Get (i);
      Ptr<ConstantPositionMobilityModel> model = CreateObject<ConstantPositionMobilityModel> ();
      node->AggregateObject (model);
      model->SetPosition (Vector (0, step * i, 0));
    }
  // 2. Create channel:
  Ptr<lrr::NeighborAwareSpectrumChannel> channel = LrrChannelHelper::Default ().Create ();



  //3. Crete TX power spectral density and noise power spectral densityL
  WifiSpectrumValue5MhzFactory sf;

  double txPower = 0.1; // Watts
  const double k = 1.381e-23; //Boltzmann's constant
  const double T = 290; // temperature in Kelvin
  double noisePsdValue = k * T; // watts per hertz
  Ptr<SpectrumValue> noisePsd = sf.CreateConstant (noisePsdValue);
  NetDeviceContainer devices;
  for (unsigned int i = 0; i < (nodeCount - 1); i++)
    {
      NeighborAwareDeviceHelper deviceHelper;
      deviceHelper.SetChannel (channel);
      // *---1---*---2---*---3---*
      // Each intermediate node is 2-interface
      deviceHelper.SetTxPowerSpectralDensity (sf.CreateTxPowerSpectralDensity (txPower, i+1));
      deviceHelper.SetNoisePowerSpectralDensity (noisePsd);
      deviceHelper.SetRxFilter (sf.CreateRfFilter (i + 5));
      //4. Install devices on nodes:
      devices.Add (deviceHelper.Install (nodes.Get (i)));
      devices.Add (deviceHelper.Install (nodes.Get (i+1)));
    }
  //5. Install internet stack and routing:
  InternetStackHelper internet;
  LrrRoutingHelper lrrRouting;
  internet.SetRoutingHelper (lrrRouting);
  internet.Install (nodes);
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipAddrs.Assign (devices);
  //6. Install applications (ping)
  V4PingHelper ping (interfaces.GetAddress (nodeCount - 1));
  ping.SetAttribute ("Verbose", BooleanValue (true));
  ApplicationContainer p = ping.Install (nodes.Get (0));
  p.Start (Seconds (0));
  p.Stop (Seconds (totalTime) - Seconds (0.001));
  //7. GO!
  lrr::GlobalGraph::Instance ()->Start ();
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  lrr::GlobalGraph::Instance ()->Stop ();
  Simulator::Destroy ();

  return 0;
}
