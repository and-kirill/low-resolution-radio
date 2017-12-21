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

#ifndef LRR_SEQ_CACHE_H_
#define LRR_SEQ_CACHE_H_

#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include <map>

namespace ns3
{
namespace lrr
{
/**
 * \ingroup lrr
 * 
 * \brief Unique packets identification cache used for simple duplicate detection.
 */
class SeqCache
{
public:
  /// c-tor
  SeqCache (Time lifetime) : m_lifetime (lifetime) {}
  /// Check that entry (addr, id) exists in cache. Add entry, if it doesn't exist.
  bool IsDuplicate (Ipv4Address addr, uint32_t id);
  /// Set lifetime for future added entries.
  void SetLifetime (Time lifetime) { m_lifetime = lifetime; }
  /// Return lifetime for existing entries in cache
  Time GetLifeTime () const { return m_lifetime; }
private:
  /// Unique packet ID
  struct UniqueId
  {
    /// The id
    uint32_t m_id;
    /// When record will expire
    Time m_expire;
    UniqueId (uint32_t id, Time t) :
      m_id (id),
      m_expire (t)
    {};
    bool
    IsExpired ()
    {
      return (m_expire < Simulator::Now ());
    };
  };

  /// Already seen IDs
  std::map<Ipv4Address, UniqueId> m_seqnoCache;
  /// Default lifetime for ID records
  Time m_lifetime;
};

} // namepsace lrr
} // namespace ns3
#endif /* LRR_SEQ_CACHE_H_ */
