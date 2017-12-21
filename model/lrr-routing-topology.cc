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
 * Authors: Denis Fakhriev <fakhriev@telum.ru>
 */

#include "lrr-routing-topology.h"

namespace ns3 {
namespace lrr {

const uint32_t GlobalTopology::Infty = (uint32_t)(-1);

GlobalTopology::GlobalTopology ()
{
}
GlobalTopology::~GlobalTopology ()
{
  Clear ();
}

void
GlobalTopology::AddVertex (Ipv4Address vertexAddr)
{
  NS_ASSERT (m_addressToIndexMap.find (vertexAddr) == m_addressToIndexMap.end ());
  NS_ASSERT (m_connectivityMatrix.size () == m_intexToAddressVector.size ());
  NS_ASSERT (m_connectivityMatrix.size () == m_addressToIndexMap.size ());

  m_addressToIndexMap.insert (std::pair<Ipv4Address, uint32_t> (vertexAddr, m_addressToIndexMap.size ()));
  m_intexToAddressVector.push_back (vertexAddr);
  InitConnectivityMatrix ();
}

void
GlobalTopology::InitConnectivityMatrix ()
{
  m_connectivityMatrix.clear ();
  for (uint32_t i = 0; i < m_intexToAddressVector.size (); ++i)
    {
      m_connectivityMatrix.push_back (std::vector<uint32_t> (m_intexToAddressVector.size (), Infty));
    }
  for (unsigned int i = 0; i < m_connectivityMatrix.size (); i++)
    {
      m_connectivityMatrix[i][i] = 0;
    }
  uint32_t nn = m_connectivityMatrix.size ();
  m_shortestPathVector.clear ();
  m_predecessorVector.clear ();
  m_shortestPathVector = std::vector<uint32_t> (nn * nn, Infty);
  m_predecessorVector = std::vector<uint32_t> (nn * nn, Infty);
}

bool
GlobalTopology::SetEdges (Ipv4Address srcAddr, std::map<Ipv4Address, uint16_t> & dstAddrVect)
{
  for (std::map<Ipv4Address, uint16_t>::const_iterator it = dstAddrVect.begin (); it != dstAddrVect.end (); ++it)
    {
      NS_ASSERT (m_addressToIndexMap.find (it->first) != m_addressToIndexMap.end ());
    }
  bool updated (false);

  // find srcNumber with srcAddr in addr_map
  uint32_t srcNumber = m_addressToIndexMap[srcAddr];
  NS_ASSERT (srcNumber < m_connectivityMatrix.size ());

  std::vector<uint32_t> newEdges (m_connectivityMatrix.size (), Infty);
  newEdges[srcNumber] = 0;
  for (std::map<Ipv4Address, uint16_t>::const_iterator it = dstAddrVect.begin (); it != dstAddrVect.end (); ++it)
    {
      newEdges[m_addressToIndexMap[it->first]] = it->second;
    }

  if (m_connectivityMatrix[srcNumber] != newEdges)
    {
      m_connectivityMatrix[srcNumber] = newEdges;
      updated = true;
    }

  return updated;
}

void
GlobalTopology::Clear (void)
{
  m_connectivityMatrix.clear ();
  m_intexToAddressVector.clear ();
  m_addressToIndexMap.clear ();
  m_shortestPathVector.clear ();
  m_predecessorVector.clear ();
}

void
GlobalTopology::FloydWarshal (void)
{
  // Print out the results table of connectivity matrix
  uint32_t nn = m_connectivityMatrix.size ();
  uint32_t i, j, k; // loop counters

  // Initialize data structures
  m_shortestPathVector.clear ();
  m_predecessorVector.clear ();
  m_shortestPathVector = std::vector<uint32_t> (nn * nn, 0);
  m_predecessorVector = std::vector<uint32_t> (nn * nn, 0);

  // Algorithm initialization
  for (i = 0; i < nn; i++)
    {
      for (j = 0; j < nn; j++)
        {
          if (m_connectivityMatrix[i][j] != Infty)
            {
              m_shortestPathVector[i * nn + j] = m_connectivityMatrix[i][j];
            }
          else
            {
              m_shortestPathVector[i * nn + j] = Infty; // disconnected
            }

          if (i == j)  // diagonal case
            {
              m_shortestPathVector[i * nn + j] = 0;
            }

          if ((m_shortestPathVector[ i * nn + j] > 0) && (m_shortestPathVector[ i * nn + j] < Infty))
            {
              m_predecessorVector[i * nn + j] = i;
            }
        }
    }

  // Main loop of the algorithm
  for (k = 0; k < nn; k++)
    {
      for (i = 0; i < nn; i++)
        {
          for (j = 0; j < nn; j++)
            {
              uint32_t newdist = ((m_shortestPathVector[i * nn + k] == Infty) || (m_shortestPathVector[k * nn + j] == Infty)) ? Infty : (m_shortestPathVector[i * nn + k] + m_shortestPathVector[k * nn + j]);
              if (m_shortestPathVector[i * nn + j] > newdist)
                {
                  m_shortestPathVector[i * nn + j] = newdist;
                  m_predecessorVector[i * nn + j] = k;
                }
            }
        }
    }
  for (i = 0; i < nn; i++)
    {
      for (j = 0; j < nn; j++)
        {
          if (i == j) continue;
          if (m_shortestPathVector[i * nn + j] == Infty) continue;
          uint32_t next = m_predecessorVector[i * nn + j];
          if (next == i) continue;
          while (m_predecessorVector[i * nn + next] != i)
            {
              next = m_predecessorVector[i * nn + next];
            }
          m_predecessorVector[i * nn + j] = next;
        }
    }
}

void
GlobalTopology::Print (std::ostream & os)
{
  Ipv4Address m_origin = m_intexToAddressVector[0];

  os << "# Network topology as seen by vertex " << m_origin << "\n";
  os << "digraph G {\n";

  uint32_t nn = m_connectivityMatrix.size ();
  for (uint32_t i = 0; i < nn; ++i)
    {
      // For all vertices print address and distance to origin
      Ipv4Address addr = m_intexToAddressVector[i];

      os << "\t\"" << addr << "\" [label=\"" << addr << "\"";

      if (addr == m_origin) os << ", shape=box";

      os << "];\n";

      // For all edges print metrics
      for (uint32_t j = 0; j < nn; ++j)
        {
          if ((m_connectivityMatrix[i][j] != Infty) && (i != j))
            {
              os << "\t\"" << addr << "\" -> \"" << m_intexToAddressVector[j] << "\" [label=\"" << (unsigned) m_connectivityMatrix[i][j]  << "\"];\n";
            }
        }

    }
  os << "};\n";
}

bool
GlobalTopology::HavePath (Ipv4Address from, Ipv4Address to)
{
  NS_ASSERT (m_addressToIndexMap.find (from) != m_addressToIndexMap.end ());
  NS_ASSERT (m_addressToIndexMap.find (to) != m_addressToIndexMap.end ());
  uint32_t index = m_addressToIndexMap[from] * m_addressToIndexMap.size () + m_addressToIndexMap[to];
  NS_ASSERT (m_shortestPathVector.size () > index);
  if (m_shortestPathVector[index] < Infty)
    {
      return true;
    }
  return false;
}

uint32_t
GlobalTopology::PathDistance (Ipv4Address from, Ipv4Address to)
{
  NS_ASSERT (m_addressToIndexMap.find (from) != m_addressToIndexMap.end ());
  NS_ASSERT (m_addressToIndexMap.find (to) != m_addressToIndexMap.end ());
  uint32_t index = m_addressToIndexMap[from] * m_addressToIndexMap.size () + m_addressToIndexMap[to];
  NS_ASSERT (m_shortestPathVector.size () > index);
  return m_shortestPathVector[index];
}

Ipv4Address
GlobalTopology::GetNextHop (Ipv4Address from, Ipv4Address to)
{
  NS_ASSERT (m_addressToIndexMap.find (from) != m_addressToIndexMap.end ());
  NS_ASSERT (m_addressToIndexMap.find (to) != m_addressToIndexMap.end ());
  uint32_t index = m_addressToIndexMap[from] * m_addressToIndexMap.size () + m_addressToIndexMap[to];
  NS_ASSERT (m_shortestPathVector.size () > index);
  if (!HavePath (from, to))
    {
      return Ipv4Address ();
    }
  if (from == to)
    {
      return from;
    }
  Ipv4Address predHop = m_intexToAddressVector[m_predecessorVector[index]];
  if (predHop == from)
    {
      return to;
    }
  return predHop;
}

} // namespace lrr
} // namespace ns3

