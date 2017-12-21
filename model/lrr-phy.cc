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
 * Author:  Kirill Andreev <k.andreev@skoltech.ru>
 */

#include "ns3/packet-burst.h"
#include "ns3/spectrum-converter.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"

#include "lrr-phy.h"
#include "lrr-device-impl.h"

namespace ns3 {
namespace lrr {

NS_LOG_COMPONENT_DEFINE ("ns3::lrr::Phy");
NS_OBJECT_ENSURE_REGISTERED (Phy);

Phy::Phy () :
  HalfDuplexIdealPhy (),
  m_channel (0),
  m_txPsd (0),
  m_errorModel (0),
  m_rxFilter (0),
  m_noisePsd (0),
  m_neighborDetectionThresholdDbm (0),
  m_edThresholdDbm (-99),
  m_linkQualityAddtionDb (0),
  m_communicationSuccessProbability (0.99),
  m_communicationTestPacketLength (100)
{
}

void
Phy::DoInitialize ()
{
  GetInterferenceNeighbors ();
}

Phy::~Phy ()
{
}

void
Phy::DoDispose ()
{
  m_channel = 0;
}

TypeId
Phy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::lrr::Phy")
    .SetParent<HalfDuplexIdealPhy> ()
    .AddConstructor<Phy> ()
    .AddAttribute ("EnergyDetectionThreshold",
                   "The same as the Energy detection threshold: all signals weaker than this threshold are not captured",
                   DoubleValue (-99), /// Default for WiFi modes with Range error model
                   MakeDoubleAccessor (&Phy::m_edThresholdDbm),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("LinkQualityAddtionDb",
                   "Add some gap to make links still relable even with fading. Set in according with fading model",
                   DoubleValue (0), /// Depends on fading conditions
                   MakeDoubleAccessor (&Phy::m_linkQualityAddtionDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CommunicationTestPacketLengh",
                   "Packet length to be tested with error model to estimate communication range",
                   UintegerValue (100),
                   MakeUintegerAccessor (&Phy::m_communicationTestPacketLength),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("CommunicationSuccessProbability",
                   "Minimal SINR required to communicate is achieved when packet of a test lenght is successfully received with CommunicationSuccessProbability",
                   DoubleValue (0.99),
                   MakeDoubleAccessor (&Phy::m_communicationSuccessProbability),
                   MakeDoubleChecker<double> (0,1))
  ;
  return tid;
}

void
Phy::SetChannel (Ptr<SpectrumChannel> c)
{
  m_channel = c->GetObject<NeighborAwareSpectrumChannel> ();
  NS_ASSERT (m_channel != 0);
  HalfDuplexIdealPhy::SetChannel (c);
}

void
Phy::StartRx (Ptr<SpectrumSignalParameters> params)
{
  NS_LOG_FUNCTION (this);
  params->psd = ApplyRxFilter (params->psd);
  double filteredPowerMw = GetSignalPowerMw (params->psd);
  if (filteredPowerMw == 0)
    {
      return;
    }
  if (MwToDbm (filteredPowerMw) > m_edThresholdDbm)
    {
      NS_LOG_LOGIC ("Signal power is enough to be received. RxPower = " << MwToDbm (filteredPowerMw) << " dBm");
      HalfDuplexIdealPhy::StartRx (params);
    }
  else
    {
      NS_LOG_LOGIC ("Signal is too weak to be received. RxPower = " << MwToDbm (filteredPowerMw) << " dBm");
      HalfDuplexIdealPhy::GetInterference ().AddSignal (params->psd, params->duration);
    }
}

void
Phy::SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
{
  m_txPsd = txPsd;
  InitiateNeighborDetectionthresholdDbm ();
  HalfDuplexIdealPhy::SetTxPowerSpectralDensity (txPsd);
}

void
Phy::SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd)
{
  m_noisePsd = noisePsd;
  InitiateNeighborDetectionthresholdDbm ();
  HalfDuplexIdealPhy::SetNoisePowerSpectralDensity (noisePsd);
}

void
Phy::SetErrorModel (Ptr<SpectrumErrorModel> m)
{
  HalfDuplexIdealPhy::SetErrorModel (m);
  m_errorModel = m;
  InitiateNeighborDetectionthresholdDbm ();
}

std::vector<Ptr<NetDevice> >
Phy::GetCommunicationNeighbors ()
{
  NeighborAwareSpectrumChannel::NeighborList neighbors = GetAllNeigbors ();
  std::vector<Ptr<NetDevice> > retval;
  for (NeighborAwareSpectrumChannel::NeighborList::iterator i = neighbors.begin (); i != neighbors.end (); i++)
    {
      NS_ASSERT (i->first->GetObject<NeighborAwareDeviceImpl> () != 0);
      Ptr<Phy> nbrPhy = i->first->GetObject<NeighborAwareDeviceImpl> ()->GetMac ()->GetPhy ()->GetObject<Phy> ();
      if (nbrPhy == 0)
        {
          continue;
        }
      double nbrRxPowerMw = GetSignalPowerMw (nbrPhy->ApplyRxFilter (i->second));
      if (MwToDbm (nbrRxPowerMw) <= m_neighborDetectionThresholdDbm)
        {
          continue;
        }
      if (NeighborRxFilterEquals (nbrPhy))
        {
          retval.push_back (i->first);
        }
    }
  return retval;
}

std::vector<Ptr<NetDevice> >
Phy::GetInterferenceNeighbors ()
{
  std::vector<Ptr<NetDevice> > retval;
  std::set<Ptr<Phy> > oneHopNeighbors = GetSensitivityNeighbors ();
  std::set<Ptr<Phy> > phySet;
  /**
   * \attention To avoid collisions, we must avoid the following simultaneous transmissions:
   * me        1-hop     2-hop
   * *-------->*<--------*
   * Receiver must not event start to receive a signal, so average RX-power must be less than energy detection.
   * Now we calculate interference neighbors as follows:
   * 1. Calculate all stations who has average RX power above ED and form 1-hop set
   * 2. For each one-hop neighbor (1-hop) we need to calculate all stations (2-hop), who, if starts to transmit(2-hop transmission), may deliver a signal to
   * a considered station (1-hop) with average RX-power above enerhy detection.
   *
   * BUT, 2-hop a calculated as follows:
   * For each one-hop neighbor (1-hop) we need to calculate all stations (2-hop), who receives a signal from (1-hop) with average RX-power above enerhy detection:
   * me        1-hop     2-hop
   * *-------->*-------->*
   *
   * This assumes the same ED and TX-PSD among all stations!
   */
  phySet.insert (oneHopNeighbors.begin (), oneHopNeighbors.end ());
  NS_LOG_DEBUG ("The size of single-hop neighbors is " << oneHopNeighbors.size ());
  for (std::set<Ptr<Phy> >::iterator i = oneHopNeighbors.begin (); i != oneHopNeighbors.end (); i++)
    {

      std::set<Ptr<Phy> > twoHopNeighbors = (*i)->GetSensitivityNeighbors ();
      phySet.insert (twoHopNeighbors.begin (), twoHopNeighbors.end ());
    }
  phySet.erase (this);
  for (std::set<Ptr<Phy> >::iterator i = phySet.begin (); i != phySet.end (); i++)
    {
      NS_ASSERT ((*i)->GetDevice ()->GetObject<NetDevice> () != 0);
      retval.push_back ((*i)->GetDevice ()->GetObject<NetDevice> ());
    }
  NS_LOG_DEBUG ("The size of interference neighbors is " << retval.size ());
  return retval;
}

std::set<Ptr<Phy> >
Phy::GetSensitivityNeighbors ()
{
  std::set<Ptr<Phy> > sensitivityNeighbors;
  NeighborAwareSpectrumChannel::NeighborList neighbors = GetAllNeigbors ();
  for (NeighborAwareSpectrumChannel::NeighborList::iterator i = neighbors.begin (); i != neighbors.end (); i++)
    {
      NS_ASSERT (i->first->GetObject<NeighborAwareDeviceImpl> () != 0);
      Ptr<Phy> nbrPhy = i->first->GetObject<NeighborAwareDeviceImpl> ()->GetMac ()->GetPhy ()->GetObject<Phy> ();
      if (nbrPhy == 0)
        {
          continue;
        }
      if (MayInterfere (nbrPhy, i->second))
        {
          sensitivityNeighbors.insert (nbrPhy);
        }
    }
  return sensitivityNeighbors;
}

void
Phy::SetRxFilter (Ptr<const SpectrumValue> rxFilter)
{
  m_rxFilter = rxFilter;
  InitiateNeighborDetectionthresholdDbm ();
}

Ptr<const SpectrumValue>
Phy::GetRxFilter () const
{
  return m_rxFilter;
}

Ptr<SpectrumValue>
Phy::ApplyRxFilter (Ptr<const SpectrumValue> received) const
{
  NS_LOG_FUNCTION (this);
  Ptr<const SpectrumValue> convertedSignal = received;
  if (received->GetSpectrumModelUid () != m_txPsd->GetSpectrumModelUid ())
    {
      Ptr<SpectrumConverter> converter = Create<SpectrumConverter> (received->GetSpectrumModel (), m_txPsd->GetSpectrumModel ());
      convertedSignal = converter->Convert (received);
      NS_LOG_LOGIC ("Signals have different UIDs");
    }
  return Create<SpectrumValue> ((*m_rxFilter) * (*convertedSignal));
}

void
Phy::InitiateNeighborDetectionthresholdDbm ()
{
  if ((m_rxFilter == 0) || (m_errorModel == 0) || (m_noisePsd == 0) || (m_txPsd == 0))
    {
      return;
    }
  Ptr<const SpectrumValue> txPsdFiltered = Create<SpectrumValue> ((*m_txPsd) * (*m_rxFilter));
  double startLossDb = MwToDbm (GetSignalPowerMw (txPsdFiltered)) - m_edThresholdDbm;
  if (GetChunkSuccessRate (*txPsdFiltered / DbToRatio (startLossDb), m_communicationTestPacketLength) > m_communicationSuccessProbability)
    {
      m_neighborDetectionThresholdDbm = m_edThresholdDbm;
      return;
    }
  double endLossDb = 0;
  NS_ASSERT (GetChunkSuccessRate (*txPsdFiltered, m_communicationTestPacketLength) > m_communicationSuccessProbability);
  while (startLossDb - endLossDb > 0.1)
    {
      double currentLossDb = (startLossDb + endLossDb) / 2;
      if (GetChunkSuccessRate (*txPsdFiltered / DbToRatio (currentLossDb), m_communicationTestPacketLength) > m_communicationSuccessProbability)
        {
          endLossDb = currentLossDb;
        }
      else
        {
          startLossDb = currentLossDb;
        }
    }
  NS_ASSERT (GetSignalPowerMw (txPsdFiltered));
  m_neighborDetectionThresholdDbm = MwToDbm (GetSignalPowerMw (txPsdFiltered)) - endLossDb;
  if (m_neighborDetectionThresholdDbm < m_edThresholdDbm)
    {
      m_neighborDetectionThresholdDbm = m_edThresholdDbm;
    }
  m_neighborDetectionThresholdDbm += m_linkQualityAddtionDb;
  NS_LOG_DEBUG ("Energy   detection threshold is:     " << m_edThresholdDbm << " dBm.");
  NS_LOG_DEBUG ("Neighbor detection threshold is:     " << m_neighborDetectionThresholdDbm << " dBm.");
}

NeighborAwareSpectrumChannel::NeighborList
Phy::GetAllNeigbors ()
{
  NS_ASSERT (m_errorModel != 0);
  NS_ASSERT_MSG (m_channel->GetObject<NeighborAwareSpectrumChannel> () != 0, "To detect neighbors, I need NeighborAwareSpectrumChannel");
  Ptr<NeighborAwareDeviceImpl> me = GetDevice ()->GetObject<NeighborAwareDeviceImpl> ();
  NS_ASSERT (me != 0);
  return m_channel->GetAllNeighbors (me, m_txPsd);
}

bool
Phy::NeighborRxFilterEquals (Ptr<const Phy> nbrPhy) const
{
  Ptr<const SpectrumValue> nbrFilter = nbrPhy->GetRxFilter ();
  // We can not compare doubles, so some tolerance is needed
  static double tolerance = 0.01;
  if (m_rxFilter->GetSpectrumModelUid () != nbrFilter->GetSpectrumModelUid ())
    {
      return false;
    }
  Values::const_iterator myIt = m_rxFilter->ConstValuesBegin ();
  Values::const_iterator nbrIt = nbrFilter->ConstValuesBegin ();
  for (; myIt != m_rxFilter->ConstValuesEnd (); myIt++, nbrIt++)
    {
      if ((*nbrIt == 0) && (abs (*myIt) > tolerance))
        {
          return false;
        }
      if (abs (*myIt / *nbrIt - 1) > tolerance)
        {
          return false;
        }
    }
  return true;
}

bool
Phy::MayInterfere (Ptr<const Phy> nbrPhy, Ptr<const SpectrumValue> avgNbrRxPsd) const
{
  // Average signal strenghts
  double rxPowerMw = GetSignalPowerMw (nbrPhy->ApplyRxFilter (avgNbrRxPsd));
  if (rxPowerMw == 0)
    {
      return false;
    }
  return (MwToDbm (rxPowerMw) > m_edThresholdDbm - m_linkQualityAddtionDb);
}

double
Phy::GetChunkSuccessRate (const SpectrumValue& signal, uint16_t size)
{
  static const unsigned int maxTry = 1000;
  unsigned int success = 0;
  // Is thousand packets enough?
  for (unsigned int i = 0; i < maxTry; i++)
    {
      m_errorModel->StartRx (Create<Packet> (size));
      m_errorModel->EvaluateChunk (signal / (*m_noisePsd), Seconds (GetRate ().CalculateBytesTxTime (size)));
      if (m_errorModel->IsRxCorrect ())
        {
          success++;
        }
    }
  return (double) success / (double) maxTry;
}

double
Phy::GetSignalPowerMw (Ptr<const SpectrumValue> signal) const
{
  return Integral (*signal) * 1000;
}

double
Phy::DbToRatio (double dB) const
{
  return pow (10, dB / 10);
}

double
Phy::MwToDbm (double mW) const
{
  return 10 * log10 (mW);
}

} // namespace lrr
} // namespace ns3
