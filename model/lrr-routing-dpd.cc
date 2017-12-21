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
 *
 * Authors: Pavel Boyko <boyko@telum.ru>
 */

#include "lrr-routing-dpd.h"

namespace ns3
{
namespace lrr
{

bool
DuplicatePacketDetection::IsDuplicate  (Ptr<const Packet> p, const Ipv4Header & header)
{
  SeqTag tag;
  if (p->PeekPacketTag (tag))
    {
      return m_seqnoCache.IsDuplicate (header.GetSource (), tag.seqno);
    }
  return true;
}

void
DuplicatePacketDetection::SetLifetime (Time lifetime)
{
  m_seqnoCache.SetLifetime (lifetime);
}

Time
DuplicatePacketDetection::GetLifetime () const
{
  return m_seqnoCache.GetLifeTime ();
}

void
DuplicatePacketDetection::PrepareTx (Ptr<Packet> p)
{
  static uint32_t lastSequence = 0;
  SeqTag tag;
  if (p->PeekPacketTag (tag))
    {
      return;
    }
  lastSequence++;
  p->AddPacketTag (SeqTag (lastSequence));
}

TypeId
DuplicatePacketDetection::SeqTag::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::DuplicatePacketDetection::SeqTag")
    .SetParent<Tag> ()
    .AddConstructor<SeqTag> ();
  return tid;
}

TypeId
DuplicatePacketDetection::SeqTag::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
DuplicatePacketDetection::SeqTag::GetSerializedSize () const
{
  return (sizeof (uint32_t));
}

void
DuplicatePacketDetection::SeqTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (seqno);
}

void
DuplicatePacketDetection::SeqTag::Deserialize (TagBuffer i)
{
  seqno = i.ReadU32 ();
}

void
DuplicatePacketDetection::SeqTag::Print (std::ostream &os) const
{
  os << "Seqno = " << seqno;
}

} //namespace lrr
} //namespace ns3

