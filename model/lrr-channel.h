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
 * Author:  Denis Fakhriev <fakhriev@telum.ru>
 *          Kirill Andreev <k.andreev@skoltech.ru>
 */

#ifndef LRR_CHANNEL_H
#define LRR_CHANNEL_H

#include "ns3/spectrum-value.h"
#include "ns3/spectrum-phy.h"
#include "ns3/spectrum-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/spectrum-propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"

namespace ns3 {
namespace lrr {
class NeighborAwareDevice;
/**
 * \ingroup lrr
 * \brief Is a wrapper for channel. Calculates communication and interference neighbors, needed
 * for collision free MAC and global MANET routing protocols.
 *
 * To construct neighbor set (communication or interference), this class keeps deterministic propagation-loss model
 * and uses it as a reference to estimate average loss. This propagation-loss model is a function of a distance and is
 * not frequency selective (i.e. signal attenuation remains the same through the received signal bandwidth).
 *
 * Neighbor sets are calculated in accordance with average RX-power provided by upper layers, which is sufficient to
 * communicate or to interfere
 *
 * This channel does not apply any spectrum converters. PHY converts received signal if needed.
 * This channel may operate with any type of spectrum PHY regardless of Communication/Interference neighbors support.
 */
class NeighborAwareSpectrumChannel : public SpectrumChannel
{
public:
  /**
   * \brief Neighbor list is associated with a sender device and represents a pointer to a device and average
   * RX-power spectral density received by a neighbor at Simulator::Now ().
   * This neighbor list is requested by PHY and needed to calculate.
   */
  typedef std::vector<std::pair<Ptr<NeighborAwareDevice>, Ptr<const SpectrumValue> > > NeighborList;
public:
  static TypeId GetTypeId ();

  NeighborAwareSpectrumChannel ();
  virtual ~NeighborAwareSpectrumChannel ();
  ///\name Inherited from Spectrum channel:
  ///\{
  void AddPropagationLossModel (Ptr<PropagationLossModel> loss);
  void AddSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> loss);
  void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);
  void StartTx (Ptr<SpectrumSignalParameters> params);
  void AddRx (Ptr<SpectrumPhy> phy);
  ///\}
  ///\name Inherited from Channel:
  ///\{
  uint32_t GetNDevices () const;
  Ptr<NetDevice> GetDevice (uint32_t i) const;
  ///\}
  ///\name Deterministic propagation loss model must depend only on the distance between nodes. Supposed to be frequency-flat
  ///\{
  void SetDeterministicPropagationLossModel (Ptr<PropagationLossModel> loss);
  void SetDeterministicSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> loss);
  ///\}
  ///\name Stochastic propagation loss model depends only on the time
  ///\{
  void SetStochasticPropagationLossModel (Ptr<PropagationLossModel> loss);
  void SetStochasticSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> loss);
  ///\}
  /// Get all pairs of devices in this channel and average RX-PSD for each device.
  NeighborList GetAllNeighbors (Ptr<NeighborAwareDevice> sender, Ptr<const SpectrumValue> txPsd) const;
private:
  typedef std::vector<Ptr<SpectrumPhy> > PhyList;
private:
  void DoDispose ();
  ///\name Helper method to calculate loss between the receiver and the transmitter
  ///\{
  Ptr<SpectrumValue> CalcLoss (Ptr<const SpectrumValue> txPsd, Ptr<MobilityModel> sender, Ptr<MobilityModel> receiver, bool deterministicOnly) const;
  Ptr<SpectrumValue> DoCalcLoss (Ptr<const SpectrumValue> txPsd, Ptr<MobilityModel> sender, Ptr<MobilityModel> receiver, Ptr<PropagationLossModel> loss) const;
  Ptr<SpectrumValue> DoCalcSpectrumLoss (Ptr<const SpectrumValue> txPsd, Ptr<MobilityModel> sender, Ptr<MobilityModel> receiver, Ptr<SpectrumPropagationLossModel> loss) const;
  ///\}
  /// For devices without node
  void StartRx (Ptr<SpectrumSignalParameters> params, Ptr<SpectrumPhy> receiver);
private:
  /**
   * \name Deterministic models:
   * \brief During any update period (of interference or communication neighbors) the value of received power
   * between any pair of nodes must not change significantly.
   * \attention Only one of two is possible: simple or spectrum model
   * \{
   */
  /// Simple propagation model:
  Ptr<PropagationLossModel> m_deterministicLoss;
  /// Spectrum propagation model
  Ptr<SpectrumPropagationLossModel> m_deterministicSpectrumLoss;
  ///\}
  ///\name Stochastic loss model: only one of two at the same moment
  ///\{
  /// Stochastic loss model:
  Ptr<PropagationLossModel> m_stochasticLoss;
  /// Stochastic spectrum loss model:
  Ptr<SpectrumPropagationLossModel> m_stochasticSpectrumLoss;
  ///\}
  /// Propagation-delay model:
  Ptr<PropagationDelayModel> m_delayModel;
  /// Attached devices:
  PhyList m_phyList;
};
} // namespace lrr
} // namespace ns3

#endif /* LRR_CHANNEL_H */
