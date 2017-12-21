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

#ifndef GLOBALGRAPHIMPL_H_
#define GLOBALGRAPHIMPL_H_

#include "ns3/ipv4-address.h"
#include <map>
#include <vector>

namespace ns3
{
namespace lrr
{

class GlobalTopology : public SimpleRefCount<GlobalTopology>
{
public:
  GlobalTopology ();
  ~GlobalTopology ();

  void AddVertex (Ipv4Address vertexAddr);
  bool SetEdges (Ipv4Address srcAddr, std::map<Ipv4Address, uint16_t> & dstAddrVect);
  void Clear (void);

  /**
   * \brief Floyd-Warshall algorithm
   *
   * based on C version from http://en.wikipedia.org/wiki/Floyd–Warshall_algorithm
   */
  void FloydWarshal (void);

  /// Print network graph in graphviz .dot format
  void Print (std::ostream & os);

  /**
   * \return true if path from Vertex with \param from address
   * to Vertex with \param to address exists
   */
  bool HavePath (Ipv4Address from, Ipv4Address to);
  /**
   * \return distance (measured by link metrics) from Vertex
   * with \param from address to Vertex with \param to address
   */
  uint32_t PathDistance (Ipv4Address from, Ipv4Address to);
  /**
   * \return address of next Vertex in path from Vertex
   * with \param from address to Vertex with \param to address,
   * return Ipv4Address() if path doesn't exist (!HavePath())
   */
  Ipv4Address GetNextHop (Ipv4Address from, Ipv4Address to);
private:
  /// Infinite distance (vertex is unreachable)
  static const uint32_t Infty;
private:
  /// Initiate connectivity matix (between all registered nodes)
  void InitConnectivityMatrix ();
  /// Debug check: conectivity matrix must be symmetric
  bool CheckSymmetricity () const;
private:
  ///\name Graph information
  ///\{
  /// vector of vectors for storing the connectivity matrix
  std::vector <std::vector<uint32_t> > m_connectivityMatrix;
  /// vector for storing the shortest distances matrix
  std::vector <uint32_t> m_shortestPathVector;
  /// vector for storing the predicate matrix, useful in reconstructing shortest routes
  std::vector <uint32_t> m_predecessorVector;
  /// number -> Address
  std::vector<Ipv4Address> m_intexToAddressVector;
  /// Address -> number
  std::map<Ipv4Address, uint32_t> m_addressToIndexMap;
  ///\}
};

} // namespace lrr
} // namespace ns3

#endif  /* GLOBALGRAPHIMPL_H_ */
