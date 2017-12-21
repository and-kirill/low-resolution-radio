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

#include "lrr-range-error-model.h"
#include "ns3/double.h"

namespace ns3
{
namespace lrr
{

NS_OBJECT_ENSURE_REGISTERED (RangeSpectrumErrorModel);

RangeSpectrumErrorModel::RangeSpectrumErrorModel () :
  m_minSinrRatio (pow (10, 8.58/10)),
  m_isRxing (false),
  m_isLastRxCorrect (true)
{
}

TypeId
RangeSpectrumErrorModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::RangeSpectrumErrorModel")
    .AddConstructor<RangeSpectrumErrorModel> ()
    .SetParent<SpectrumErrorModel> ()
    .AddAttribute ("MinSinrDb", "Minimum average sinr sufficienf for successful reception",
                   DoubleValue (8.58),
                   MakeDoubleAccessor (&RangeSpectrumErrorModel::SetMinSinrDb,
                                       &RangeSpectrumErrorModel::GetMinSinrDb),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

// inherited from SpectrumErrorModel
void
RangeSpectrumErrorModel::StartRx (Ptr<const Packet> p)
{
  m_isRxing = true;
  m_isLastRxCorrect = true;
}

void
RangeSpectrumErrorModel::EvaluateChunk (const SpectrumValue& sinr, Time duration)
{
  NS_ASSERT (m_isRxing);
  double totSinr = 0.0;
  uint32_t size = 0;
  for (Values::const_iterator vit = sinr.ConstValuesBegin (); vit != sinr.ConstValuesEnd (); vit++)
    {
      if (*vit == 0)
        {
          continue;
        }
      size++;
      totSinr += *vit;
    }
  // Compare average SINR passed through RX filter with threshold
  if (totSinr / (double)size < m_minSinrRatio)
    {
      m_isLastRxCorrect = false;
    }
}

bool
RangeSpectrumErrorModel::IsRxCorrect ()
{
  NS_ASSERT (m_isRxing);
  m_isRxing = false;
  return m_isLastRxCorrect;
}

void
RangeSpectrumErrorModel::SetMinSinrDb (double valDb)
{
  m_minSinrRatio = pow (10, valDb/10);
}

double
RangeSpectrumErrorModel::GetMinSinrDb () const
{
  return 10 * log10 (m_minSinrRatio);
}
} // namespace lrr
} // namespace ns3
