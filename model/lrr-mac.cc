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
#include "lrr-mac.h"
#include "lrr-phy.h"
#include "ns3/uinteger.h"

namespace ns3 {
namespace lrr {
NS_OBJECT_ENSURE_REGISTERED (Mac);

TypeId
Mac::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::Mac")
    .SetParent<Object> ()
    .AddAttribute ("MaxMsduSize", "The maximum size of an MSDU accepted by the MAC layer.",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&Mac::m_maxMsduSize),
                   MakeUintegerChecker<uint16_t> (1,1500))
  ;
  return tid;
}

Mac::Mac () :
  m_phy (0)
{}

Mac::~Mac()
{
  m_phy = 0;
  m_upCallback = MakeNullCallback<void,Ptr<Packet>, Address, Address, uint16_t> ();
}

void
Mac::DoInitialize ()
{
}

void
Mac::DoDispose ()
{
  m_phy = 0;
  m_upCallback = MakeNullCallback<void,Ptr<Packet>, Address, Address, uint16_t> ();
}

void
Mac::SetMaxMsduSize (uint32_t maxMsduSize)
{
  m_maxMsduSize = maxMsduSize;
}

uint32_t
Mac::GetMaxMsduSize () const
{
  return m_maxMsduSize;
}

void
Mac::SetForwardUpCallback (ForwardUpCallback upCallback)
{
  m_upCallback = upCallback;
}

void
Mac::SetPhy (Ptr<SpectrumPhy> phy)
{
  NS_ASSERT (phy->GetObject<Phy> ());
  m_phy = phy->GetObject<Phy> ();
}

Ptr<SpectrumPhy>
Mac::GetPhy () const
{
  return m_phy;
}

std::vector <Ptr<NetDevice> >
Mac::GetCommunicationNeighbors () const
{
  NS_ASSERT (m_phy != 0);
  return m_phy->GetCommunicationNeighbors ();
}

void
Mac::StartTransmission (Ptr<const Packet> packet)
{
  m_phy->StartTx (packet->Copy ());
}

void
Mac::ForwardUp (Ptr<Packet> p, Address from, Address to, uint16_t protocol)
{
  m_upCallback (p, from, to, protocol);
}


} // namespace lrr
} // namespace ns3
