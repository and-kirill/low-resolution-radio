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

#include "ns3/log.h"
#include "ns3/address-utils.h"
#include "lrr-mac-header.h"

NS_LOG_COMPONENT_DEFINE ("LrrMacHeader");

namespace ns3 {
namespace lrr {

NS_OBJECT_ENSURE_REGISTERED (MacHeader);
// Setters and getters {
MacHeader::MacHeader ()
  : m_type (0),
  m_destination (Address ()),
  m_source (Address ())
{
}

void
MacHeader::SetType (uint16_t type)
{
  m_type = type;
}

uint16_t
MacHeader::GetType () const
{
  return m_type;
}

void
MacHeader::SetSource (Address source)
{
  m_source = source;
}

Address
MacHeader::GetSource () const
{
  return m_source;
}

void
MacHeader::SetDestination (Address dst)
{
  m_destination = dst;
}

Address
MacHeader::GetDestination () const
{
  return m_destination;
}
// }
//  Header base class methods: {
TypeId
MacHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::MacHeader")
    .SetParent<Header> ()
    .AddConstructor<MacHeader> ()
  ;
  return tid;
}

TypeId
MacHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

void
MacHeader::Print (std::ostream &os) const
{
  os << " type=0x" << std::hex << m_type << std::dec
     << ", source=" << m_source
     << ", destination=" << m_destination;
}

void
MacHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  NS_ASSERT (m_source.GetLength () == m_destination.GetLength ());
  i.WriteU8 (m_source.GetLength ());
  WriteTo (i, m_destination);
  WriteTo (i, m_source);
  i.WriteHtonU16 (m_type);
}

uint32_t
MacHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint32_t hardwareAddressLen = i.ReadU8 ();
  ReadFrom (i, m_destination, hardwareAddressLen);
  ReadFrom (i, m_source, hardwareAddressLen);
  m_type = i.ReadNtohU16 ();
  return GetSerializedSize ();
}

uint32_t
MacHeader::GetSerializedSize () const
{
  return 1 + m_source.GetLength () * 2 + 2;
}
// }
} // namespace lrr
} // namespace ns3
