/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Telum (www.telum.ru)
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
#include "lrr-routing-seq-cache.h"
#include <algorithm>

namespace ns3
{
namespace lrr
{
bool
SeqCache::IsDuplicate (Ipv4Address addr, uint32_t id)
{
  std::map<Ipv4Address, UniqueId>::iterator i = m_seqnoCache.find (addr);
  if (i == m_seqnoCache.end ())
    {
      m_seqnoCache.insert (std::make_pair (addr, UniqueId (id, m_lifetime + Simulator::Now ())));
      return false;
    }
  if ((!i->second.IsExpired ()) && (i->second.m_id == id))
    {
      i->second = UniqueId (id, m_lifetime + Simulator::Now ());
      return true;
    }
  i->second = UniqueId (id, m_lifetime + Simulator::Now ());
  return false;
}
}
}
