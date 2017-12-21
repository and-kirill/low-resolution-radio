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

#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "lrr-channel.h"
#include "lrr-device-impl.h"
#include "lrr-mac.h"

NS_LOG_COMPONENT_DEFINE ("LrrNeighborAwareSpectrumChannel");

namespace ns3 {
namespace lrr {

NS_OBJECT_ENSURE_REGISTERED (NeighborAwareSpectrumChannel);

TypeId
NeighborAwareSpectrumChannel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::NeighborAwareSpectrumChannel")
    .SetParent<Channel> ()
    .AddConstructor<NeighborAwareSpectrumChannel> ()
  ;
  return tid;
}

NeighborAwareSpectrumChannel::NeighborAwareSpectrumChannel () :
  m_deterministicLoss (0),
  m_deterministicSpectrumLoss (0),
  m_stochasticLoss (0),
  m_stochasticSpectrumLoss (0),
  m_delayModel (0),
  m_phyList (PhyList ())
{
}

NeighborAwareSpectrumChannel::~NeighborAwareSpectrumChannel ()
{
  m_deterministicLoss = 0;
  m_deterministicSpectrumLoss = 0;
  m_stochasticLoss = 0;
  m_stochasticSpectrumLoss = 0;
  m_delayModel = 0;
  m_phyList.clear ();
}

void
NeighborAwareSpectrumChannel::DoDispose ()
{
  m_deterministicLoss = 0;
  m_deterministicSpectrumLoss = 0;
  m_stochasticLoss = 0;
  m_stochasticSpectrumLoss = 0;
  m_delayModel = 0;
  m_phyList.clear ();
}

void
NeighborAwareSpectrumChannel::SetDeterministicPropagationLossModel (Ptr<PropagationLossModel> loss)
{
  if (m_deterministicSpectrumLoss != 0)
    {
      NS_FATAL_ERROR ("You have already set determinisitc loss model!");
    }
  m_deterministicLoss = loss;
}

void
NeighborAwareSpectrumChannel::SetDeterministicSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> loss)
{
  if (m_deterministicLoss != 0)
    {
      NS_FATAL_ERROR ("You have already set determinisitc loss model!");
    }
  m_deterministicSpectrumLoss = loss;
}

void
NeighborAwareSpectrumChannel::SetStochasticPropagationLossModel (Ptr<PropagationLossModel> loss)
{
  if (m_stochasticSpectrumLoss != 0)
    {
      NS_FATAL_ERROR ("You have already set stochastic loss model!");
    }
  m_stochasticLoss = loss;
}

void
NeighborAwareSpectrumChannel::SetStochasticSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> loss)
{
  if (m_stochasticLoss != 0)
    {
      NS_FATAL_ERROR ("You have already set stochastic loss model!");
    }
  m_stochasticSpectrumLoss = loss;
}

void
NeighborAwareSpectrumChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay)
{
  m_delayModel = delay;
}

void
NeighborAwareSpectrumChannel::AddRx (Ptr<SpectrumPhy> phy)
{
  m_phyList.push_back (phy);
  phy->SetChannel (this);
}

NeighborAwareSpectrumChannel::NeighborList
NeighborAwareSpectrumChannel::GetAllNeighbors (Ptr<NeighborAwareDevice> sender, Ptr<const SpectrumValue> txPsd) const
{
  NS_ASSERT (sender->GetObject<NeighborAwareDeviceImpl>() != 0);
  Ptr<MobilityModel> senderMobility = sender->GetObject<NeighborAwareDeviceImpl> ()->GetMac ()->GetPhy ()->GetMobility ();
  NS_ASSERT (senderMobility != 0);
  NeighborList retval;
  for (PhyList::const_iterator rxPhyIterator = m_phyList.begin (); rxPhyIterator != m_phyList.end (); ++rxPhyIterator)
    {
      Ptr<NeighborAwareDevice> device = (*rxPhyIterator)->GetDevice ()->GetObject<NeighborAwareDevice> ();
      // We may deal with self or with some other device type
      if ((device == 0) || (sender == device))
        {
          continue;
        }
      Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility ();
      NS_ASSERT (receiverMobility != 0);

      Ptr<SpectrumValue> rxPsd = CalcLoss (txPsd, senderMobility, receiverMobility, true /*Deterministic only*/);
      retval.push_back (std::make_pair (device, rxPsd));
    }
  return retval;
}

void
NeighborAwareSpectrumChannel::AddPropagationLossModel (Ptr<PropagationLossModel> loss)
{
  NS_FATAL_ERROR ("This type of channel needs two types of propagation loss models: Deterministic and Stochastic!");
}

void
NeighborAwareSpectrumChannel::AddSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> loss)
{
  NS_FATAL_ERROR ("This type of channel needs two types of propagation loss models: Deterministic and Stochastic!");
}

Ptr<NetDevice>
NeighborAwareSpectrumChannel::GetDevice (uint32_t i) const
{
  return m_phyList.at (i)->GetDevice ()->GetObject<NetDevice> ();
}

uint32_t
NeighborAwareSpectrumChannel::GetNDevices () const
{
  return m_phyList.size ();
}

Ptr<SpectrumValue>
NeighborAwareSpectrumChannel::CalcLoss (Ptr<const SpectrumValue> txPsd, Ptr<MobilityModel> sender, Ptr<MobilityModel> receiver, bool deterministicOnly) const
{
  Ptr<SpectrumValue> rxPsd = txPsd->Copy ();
  if (m_deterministicLoss != 0)
    {
      rxPsd = DoCalcLoss (rxPsd, sender, receiver, m_deterministicLoss);
      NS_ASSERT (m_deterministicSpectrumLoss == 0);
    }
  else if (m_deterministicSpectrumLoss != 0)
    {
      rxPsd = DoCalcSpectrumLoss (rxPsd, sender, receiver, m_deterministicSpectrumLoss);
    }
  // Return if stochastic loss is not taken into account
  if (deterministicOnly)
    {
      return rxPsd;
    }
  if (m_stochasticLoss != 0)
    {
      rxPsd = DoCalcLoss (rxPsd, sender, receiver, m_stochasticLoss);
    }
  else if (m_stochasticSpectrumLoss != 0)
    {
      rxPsd = DoCalcSpectrumLoss (rxPsd, sender, receiver, m_stochasticSpectrumLoss);
    }
  return rxPsd;
}

Ptr<SpectrumValue>
NeighborAwareSpectrumChannel::DoCalcLoss (Ptr <const SpectrumValue> txPsd, Ptr<MobilityModel> sender, Ptr<MobilityModel> receiver, Ptr<PropagationLossModel> loss) const
{
  Ptr<SpectrumValue> rxPsd = txPsd->Copy ();
  Bands::const_iterator fit = rxPsd->ConstBandsBegin ();
  Values::iterator vit = rxPsd->ValuesBegin ();
  // We suppose non-frequency selective propagation for better performance
  double lossRatio = pow (10, loss->CalcRxPower (0, sender, receiver) / 10.0);
  while (vit != rxPsd->ValuesEnd ())
    {
      NS_ASSERT (fit != rxPsd->ConstBandsEnd ());
      *vit *= lossRatio;
      ++vit;
      ++fit;
    }
  return rxPsd;
}

Ptr<SpectrumValue>
NeighborAwareSpectrumChannel::DoCalcSpectrumLoss (Ptr <const SpectrumValue> txPsd, Ptr<MobilityModel> sender, Ptr<MobilityModel> receiver, Ptr<SpectrumPropagationLossModel> loss) const
{
  return loss->CalcRxPowerSpectralDensity (txPsd, sender, receiver);
}

void
NeighborAwareSpectrumChannel::StartTx (Ptr<SpectrumSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this << txParams->psd << txParams->duration << txParams->txPhy);
  NS_ASSERT_MSG (txParams->psd, "NULL txPsd");
  NS_ASSERT_MSG (txParams->txPhy, "NULL txPhy");

  Ptr<MobilityModel> senderMobility = txParams->txPhy->GetMobility ();

  for (PhyList::const_iterator rxPhyIterator = m_phyList.begin (); rxPhyIterator != m_phyList.end (); ++rxPhyIterator)
    {
      if ((*rxPhyIterator) == txParams->txPhy)
        {
          continue;
        }
      Time delay = MicroSeconds (0);

      Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility ();
      NS_ASSERT (senderMobility != 0);
      NS_ASSERT (receiverMobility != 0);
      NS_ASSERT (m_deterministicLoss != 0);
      Ptr<SpectrumSignalParameters> rxParams = txParams->Copy ();
      rxParams->psd = CalcLoss (txParams->psd, senderMobility, receiverMobility, false /*Not determinisic only*/);
      if (m_delayModel)
        {
          delay = m_delayModel->GetDelay (senderMobility, receiverMobility);
        }
      Ptr<Object> netDevObj = (*rxPhyIterator)->GetDevice ();
      if (netDevObj)
        {
          // the receiver has a NetDevice, so we expect that it is attached to a Node
          uint32_t dstNode = netDevObj->GetObject<NetDevice> ()->GetNode ()->GetId ();
          Simulator::ScheduleWithContext (dstNode, delay, &NeighborAwareSpectrumChannel::StartRx, this, rxParams,
                                          *rxPhyIterator);
        }
      else
        {
          // the receiver is not attached to a NetDevice, so we cannot assume that it is attached to a node
          Simulator::Schedule (delay, &NeighborAwareSpectrumChannel::StartRx, this, rxParams, *rxPhyIterator);
        }
    }
}

void
NeighborAwareSpectrumChannel::StartRx (Ptr<SpectrumSignalParameters> params, Ptr<SpectrumPhy> receiver)
{
  NS_LOG_FUNCTION (this << params);
  receiver->StartRx (params);
}
} // namespace lrr
} // namespace ns3
