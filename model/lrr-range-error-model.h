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
 * Authors: Kirill Andreev <k.andreev@skoltech.ru>
 */
#ifndef LRR_RANGE_ERROR_MODEL_H_
#define LRR_RANGE_ERROR_MODEL_H_

#include "ns3/spectrum-error-model.h"

namespace ns3
{
namespace lrr
{
/**
 * \ingroup lrr
 * \brief This error model uses minimal SINR value needed for successful reception.
 * If there is a moment of time, when average value of SINR (in dB) was less than a threshold,
 * packet is supposed to be corrupted.
 */
class RangeSpectrumErrorModel : public SpectrumErrorModel
{
public:
  RangeSpectrumErrorModel ();
  static TypeId GetTypeId ();
  ///\name Inherited from SpectrumErrorModel
  ///\{
  void StartRx (Ptr<const Packet> p);
  void EvaluateChunk (const SpectrumValue& sinr, Time duration);
  bool IsRxCorrect ();
  ///\}
private:
  ///\name Used by TypeId: set a threshold in dB needed for successful reception
  ///\{
  void SetMinSinrDb (double valDb);
  double GetMinSinrDb () const;
private:
  double m_minSinrRatio;
  ///\}
  bool m_isRxing;
  bool m_isLastRxCorrect;
};
} // namespace lrr
} // namespace ns3
#endif
