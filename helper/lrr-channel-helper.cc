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
 * Author:  Kirill Andreev <k.andreev@skoltech.ru>, written after LrrChannelHelper by Nicola Baldo <nbaldo@cttc.es>
 */

#include "lrr-channel-helper.h"


namespace ns3 {


LrrChannelHelper
LrrChannelHelper::Default (void)
{
  LrrChannelHelper h;
  h.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  h.SetDeterministicPropagationLoss ("ns3::FriisPropagationLossModel");
  return h;
}

void
LrrChannelHelper::SetDeterministicPropagationLoss (std::string type,
                                                   std::string n0, const AttributeValue &v0,
                                                   std::string n1, const AttributeValue &v1,
                                                   std::string n2, const AttributeValue &v2,
                                                   std::string n3, const AttributeValue &v3,
                                                   std::string n4, const AttributeValue &v4,
                                                   std::string n5, const AttributeValue &v5,
                                                   std::string n6, const AttributeValue &v6,
                                                   std::string n7, const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_deterministicPropagationLossModel = factory.Create<PropagationLossModel> ();
}

void
LrrChannelHelper::SetDeterministicPropagationLoss (Ptr<PropagationLossModel> m)
{
  m_deterministicPropagationLossModel = m;
}

void
LrrChannelHelper::AddSpectrumPropagationLoss (std::string type,
                                              std::string n0, const AttributeValue &v0,
                                              std::string n1, const AttributeValue &v1,
                                              std::string n2, const AttributeValue &v2,
                                              std::string n3, const AttributeValue &v3,
                                              std::string n4, const AttributeValue &v4,
                                              std::string n5, const AttributeValue &v5,
                                              std::string n6, const AttributeValue &v6,
                                              std::string n7, const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  Ptr<SpectrumPropagationLossModel> m = factory.Create<SpectrumPropagationLossModel> ();
  AddSpectrumPropagationLoss (m);
}

void
LrrChannelHelper::AddSpectrumPropagationLoss (Ptr<SpectrumPropagationLossModel> m)
{
  m->SetNext (m_spectrumPropagationLossModel);
  m_spectrumPropagationLossModel = m;
}

void
LrrChannelHelper::SetPropagationDelay (std::string type,
                                       std::string n0, const AttributeValue &v0,
                                       std::string n1, const AttributeValue &v1,
                                       std::string n2, const AttributeValue &v2,
                                       std::string n3, const AttributeValue &v3,
                                       std::string n4, const AttributeValue &v4,
                                       std::string n5, const AttributeValue &v5,
                                       std::string n6, const AttributeValue &v6,
                                       std::string n7, const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_propagationDelayModel = factory.Create<PropagationDelayModel>();
}

Ptr<lrr::NeighborAwareSpectrumChannel>
LrrChannelHelper::Create ()
{
  Ptr<lrr::NeighborAwareSpectrumChannel> channel = CreateObject<lrr::NeighborAwareSpectrumChannel> ();
  channel->SetDeterministicPropagationLossModel (m_deterministicPropagationLossModel);
  channel->SetPropagationDelayModel (m_propagationDelayModel);
  channel->SetStochasticPropagationLossModel (m_propagationLossModel);
  return channel;
}
} //namespace ns3
