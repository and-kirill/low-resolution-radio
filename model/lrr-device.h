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
 * Author: Kirill Andreev <k.andreev@skoltech.ru>
 */

#ifndef LRR_NET_DEVICE_H_
#define LRR_NET_DEVICE_H_


#include "ns3/net-device.h"

namespace ns3
{
namespace lrr
{
/**
 * \ingroup lrr
 * \brief Represents a networking device, which  provides
 *  global topology information using GetNeigbors method
 */
class NeighborAwareDevice : public NetDevice
{
public:
  static TypeId GetTypeId (void);
  virtual ~NeighborAwareDevice ();
  ///\name Global topology knowledge functionality:
  ///\{
  /// Obtain neighbors: depends on MAC type
  virtual std::vector<Ptr<NetDevice> > GetCommunicationNeighbors () const = 0;
  ///\}
};
}
}
#endif //RADIO_NET_DEVICE_H_
