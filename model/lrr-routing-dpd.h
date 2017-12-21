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
 * Authors: Pavel Boyko <boyko@telum.ru>
 */

#ifndef LRR_DUPLICATEPACKETDETECTION_H_
#define LRR_DUPLICATEPACKETDETECTION_H_

#include "ns3/lrr-routing-seq-cache.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"

namespace ns3
{
namespace lrr
{
/**
 * \ingroup lrr
 * 
 * \brief Helper class used to remember already seen packets and detect duplicates.
 *
 * Duplicate detection is based on uinique packet ID given by sequence tag added
 */
class DuplicatePacketDetection
{
public:
  /// C-tor
  DuplicatePacketDetection (Time lifetime) : m_seqnoCache (lifetime) {}
  /// Check that the packet is duplicated. If not, save information about this packet.
  bool IsDuplicate (Ptr<const Packet> p, const Ipv4Header & header);
  /// Set duplicate records lifetimes
  void SetLifetime (Time lifetime);
  /// Get duplicate records lifetimes
  Time GetLifetime () const;
  /// Add sequence tag:
  void PrepareTx (Ptr<Packet> p);
private:
  class SeqTag : public Tag
  {
public:
    /// Sequence number
    uint32_t seqno;

    SeqTag (uint32_t s = 0) :
      Tag (), seqno (s)
    {}

    ///\name Inherited from Tag
    ///\{
    static TypeId GetTypeId ();
    TypeId GetInstanceTypeId () const;
    uint32_t GetSerializedSize () const;
    void Serialize (TagBuffer i) const;
    void Deserialize (TagBuffer i);
    void Print (std::ostream &os) const;
    ///\}
  };
  /// Impl
  SeqCache m_seqnoCache;
};

} // namespace lrr
} // namespace lrr

#endif
