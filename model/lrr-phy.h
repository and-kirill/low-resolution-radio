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

#ifndef LRR_PHY_H
#define LRR_PHY_H

#include "ns3/half-duplex-ideal-phy.h"
#include "ns3/spectrum-error-model.h"
#include "ns3/lrr-channel.h"
#include "ns3/lrr-mac.h"
#include <set>

namespace ns3 {

namespace lrr {

/**
 * \ingroup lrr
 * \brief Implements PHY services: reuses HalfduplexIdealPhy +
 * implements receiver frequency response and energe detection
 * thresold. Both features are needed for LRR-MAC operation
 */
class Phy : public HalfDuplexIdealPhy
{

public:
  Phy ();
  virtual ~Phy ();
  static TypeId GetTypeId (void);

  ///\name Overwritten methods of HalfduplexIdealPhy
  ///\{
  void SetChannel (Ptr<SpectrumChannel> c);
  void StartRx (Ptr<SpectrumSignalParameters> params);
  void SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd);
  void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd);
  void SetErrorModel (Ptr<SpectrumErrorModel> model);
  ///\}
  ///\name Core feature of LRR PHY: it can calculate neighbors for communication and interference-resolving
  ///\{
  std::vector<Ptr<NetDevice> > GetCommunicationNeighbors ();
  /// Interference neihbors are one hop + two hop neighbors obtained by sensitivity threshold
  std::vector<Ptr<NetDevice> > GetInterferenceNeighbors ();
  ///\}
  /// Receiver frequency response characteristic (to take into account inter-channel interference)
  void SetRxFilter (Ptr<const SpectrumValue> rxFilter);
  /// Needed by neighbor PHYs to determine wheter my transmission causes interference or not to this device
  Ptr<const SpectrumValue> GetRxFilter () const;
private:
  void DoInitialize ();
  /// Real destructor
  virtual void DoDispose (void);
  /// Apply RX-filter to received signal. Convert received signal if needed:
  Ptr<SpectrumValue> ApplyRxFilter (Ptr<const SpectrumValue> received) const;
  /// Initiate neighbor detection threshold: packet size and it's one try success probability are intputs
  void InitiateNeighborDetectionthresholdDbm ();
  ///\name Neighbor detection methods:
  ///\{
  /// Get neighbors by a threshold from a channel. \return device and rx-power pairs.
  NeighborAwareSpectrumChannel::NeighborList GetAllNeigbors ();
  /// Check that neighbor operates at the same frequency with the same spectrum model UID
  bool NeighborRxFilterEquals (Ptr<const Phy> nbrPhy) const;
  /**
   * \brief Check that neighbor PHY may interfere with us. Checks inter-channel interference too.
   * \param nbrPhy is pointer to neighbor's PHY
   * \param avgNbrRxPsd is average received power spectral density received by a neighbor due to our transmission
   */
  bool MayInterfere (Ptr<const Phy> nbrPhy, Ptr<const SpectrumValue> avgNbrRxPsd) const;
  ///\}
  /// Calculate packet error probability using error model and sinr:
  double GetChunkSuccessRate (const SpectrumValue&  signal, uint16_t size);
  ///\name Calculate a power of a signal
  ///\{
  double GetSignalPowerMw (Ptr<const SpectrumValue> signal) const;
  double DbToRatio (double dB) const;
  double MwToDbm (double mW) const;
  ///\}
  /// Sensitivity neighbors: average rx-power more that a sensitivity threshold. One and Two-hop sensitivity neighbors are supposed to form interference neighbors.
  std::set<Ptr<Phy> > GetSensitivityNeighbors ();
private:
  Ptr<NeighborAwareSpectrumChannel> m_channel;
  ///\name Parameters needed to estimate communication neighbors:
  ///\{
  Ptr<SpectrumValue> m_txPsd;
  Ptr<SpectrumErrorModel> m_errorModel;
  Ptr<const SpectrumValue> m_rxFilter;
  Ptr<const SpectrumValue> m_noisePsd;
  ///\}
  ///\name Neighbor detection are calculated before Simulator::Run has been invoked using error rate model.
  double m_neighborDetectionThresholdDbm;
  ///\name Attributes:
  ///\{
  /// Sensitivity threshold: each signal weaker than energe detection is ignored
  double m_edThresholdDbm;
  /// Addition to the neighbor detection. Introduced to make links more robust in case of stockhastic propagation. Depens on fading model.
  double m_linkQualityAddtionDb;
  ///\name Communication neighbors: to calculate minimal SINR required for successful reception of a packet with given length and given success probability
  /// Packet success rate required
  double m_communicationSuccessProbability;
  /// Packet length to be tested with error model:
  uint16_t m_communicationTestPacketLength;
  ///\}
};

} // namespace lrr
} // namespace ns3
#endif
