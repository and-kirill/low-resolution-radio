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


#ifndef RADIO_HEADER_H
#define RADIO_HEADER_H

#include "ns3/header.h"
#include "ns3/address.h"

namespace ns3 {
namespace lrr {

/**
 * \ingroup lrr
 *
 * \brief header for MAC: consists of ethernet header basic fields (source, destination, type). Address may of any type
 */
class MacHeader : public Header
{
public:
  /// Construct a null radio header
  MacHeader ();
  ///\name Setters/Getters for all fields:
  ///\{
  /// Set the type of the overlying protocol
  void SetType (uint16_t type);
  /// Get the type of the overlying protocol
  uint16_t GetType () const;
  /// Set the source address of this packet
  void SetSource (Address source);
  /// Get the source address of this packet
  Address GetSource () const;
  /// Set the destination address of this packet
  void SetDestination (Address destination);
  /// Get the destination address of this packet
  Address GetDestination () const;
  ///\}
  ///\name Inherited from Header base class:
  ///\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  void Print (std::ostream &os) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  uint32_t GetSerializedSize () const;
  ///\}
private:
  /// Overlying protocol type
  uint16_t m_type;
  /// Destination address
  Address m_destination;
  /// Source address
  Address m_source;
};
} // namespace lrr
} // namespace ns3

#endif /* RADIO_HEADER_H */
