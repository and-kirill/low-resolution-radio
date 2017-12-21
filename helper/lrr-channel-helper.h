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
 * Author:  Kirill Andreev <k.andreev@skoltech.ru>, written after SpectrumChannelHelper by Nicola Baldo <nbaldo@cttc.es>
 */

#ifndef LRR_CHANNEL_HELPER_H_
#define LRR_CHANNEL_HELPER_H_
/**
 * \ingroup lrr
 *
 */

#include <string>
#include <ns3/attribute.h>
#include <ns3/object-factory.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/spectrum-propagation-loss-model.h>

#include "ns3/lrr-channel.h"

namespace ns3 {

class LrrChannelHelper
{
public:
  static LrrChannelHelper Default ();

  /**
   * \param name the name of the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * Set deterministic propagation loss, which is a function of distance, is not frequency selective for each signal.
   * This model is used to estimate neighbor communication and interference ranges
   */
  void SetDeterministicPropagationLoss (std::string name,
                                        std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                                        std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                                        std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                                        std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                                        std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                                        std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                                        std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                                        std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());


  /**
   * Add any other propagation loss model
   */
  void SetDeterministicPropagationLoss (Ptr<PropagationLossModel> m);

  /**
   * \param name the name of the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * Add any other frequency-dependent propagation-loss model.
   */
  void AddSpectrumPropagationLoss (std::string name,
                                   std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                                   std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                                   std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                                   std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                                   std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                                   std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                                   std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                                   std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

  /**
   * Add a new frequency-dependent propagation loss model instance to this channel helper.
   *
   * \param m a pointer to the instance of the propagation loss model
   */
  void AddSpectrumPropagationLoss (Ptr<SpectrumPropagationLossModel> m);

  /**
   * \param name the name of the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * Configure a propagation delay for this channel.
   */
  void SetPropagationDelay (std::string name,
                            std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                            std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                            std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                            std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                            std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                            std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                            std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                            std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

  /**
   * \returns a new channel
   *
   * Create a channel based on the configuration parameters set previously.
   */
  Ptr<lrr::NeighborAwareSpectrumChannel> Create ();

private:
  Ptr<PropagationLossModel> m_deterministicPropagationLossModel;
  Ptr<SpectrumPropagationLossModel> m_spectrumPropagationLossModel;
  Ptr<PropagationLossModel> m_propagationLossModel;
  Ptr<PropagationDelayModel> m_propagationDelayModel;
};

} // namespace ns3
#endif // LRR_CHANNEL_HELPER_H_
