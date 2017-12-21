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

#include "lrr-mac-access-manager.h"
#include "lrr-mac-impl.h"
#include "lrr-phy.h"
#include "lrr-device-impl.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/queue.h"

NS_LOG_COMPONENT_DEFINE ("LrrAccessManager");

namespace ns3 {
namespace lrr {

NS_OBJECT_ENSURE_REGISTERED (AccessManager);

TypeId
AccessManager::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::lrr::AccessManager")
    .SetParent<Object> ()
    .AddConstructor<AccessManager> ()
    .AddAttribute ( "GuardInterval",
                    "Time gap for propagation delay and preamble",
                    TimeValue (MicroSeconds (100)),
                    MakeTimeAccessor (&AccessManager::m_guardInterval),
                    MakeTimeChecker ()
                    )

    .AddAttribute ("InterferenceNbrUpdatePeriod",
                   "How often interference neighbors are reconstructed",
                   TimeValue (Seconds (0.5)), /// Depends on mobility conditions
                   MakeTimeAccessor (&AccessManager::m_interferenceNeighborUpdatePeriod),
                   MakeTimeChecker ())
  ;
  return tid;
}

AccessManager::AccessManager ()
  : m_queue (0),
  m_timeoutEnd (MicroSeconds (0)),
  m_mac (0),
  m_guardInterval (MicroSeconds (150)),
  m_lastInterferenceNeighborsUpdate (Seconds (0)),
  m_interferenceNeighborUpdatePeriod (Seconds (0.5))
{
}

AccessManager::~AccessManager ()
{
  m_accessTimeout.Cancel ();
  m_interferenceNeighbors.clear ();
  m_mac = 0;
  m_queue = 0;
}

void
AccessManager::Enqueue (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_queue->Enqueue (packet->Copy ());
  StartAccessIfNeeded ();
}

void
AccessManager::SetMac (Ptr<Mac> mac)
{
  m_mac = mac;
}

void
AccessManager::StartAccessIfNeeded ()
{
  NS_LOG_FUNCTION (this);
  if (m_queue->IsEmpty () || m_accessTimeout.IsRunning ())
    {
      // Nothing to do
      NS_LOG_DEBUG ("Queue not empty or access timer is running");
      return;
    }
  Time timeoutDelay = GetTimeoutEnd () - Simulator::Now ();
  NS_LOG_DEBUG ("Timeout delay: " << timeoutDelay);
  NS_ASSERT (m_accessTimeout.IsExpired ());
  m_accessTimeout = Simulator::Schedule (timeoutDelay, &AccessManager::StartAccess, this);
}

void
AccessManager::StartAccess ()
{
  NS_LOG_FUNCTION (this);
  if (m_queue->IsEmpty ())
    {
      return;
    }
  // 1. Calculate TX-start time:
  Time txStart = CalculateTxStartTime ();
  Time txDelay = txStart - Simulator::Now ();
  // 2. Construct a packet burst (timed out packets are dropped automatically by queue, so txDelay is passed)
  PacketList burst;
  Ptr<const Packet> currentPacket = m_queue->Dequeue ();
  if (currentPacket == 0)
    {
      // May be if packet was timed out:
      return;
    }
  // 3. Update timeout end:
  Ptr<Phy> phy = m_mac->GetPhy ()->GetObject<Phy> ();
  NS_ASSERT_MSG (phy != 0, "Collision free MAC works with LrrPhy or any other that supports GetRate method");
  m_timeoutEnd = txStart + phy->GetRate ().CalculateBytesTxTime (currentPacket->GetSize ()) + m_guardInterval;
  NS_LOG_DEBUG ("now=" << Simulator::Now ().GetSeconds () << "s\t"
                       << "txStart = " << txStart.GetSeconds () << "s\t"
                       << "txEnd = " << GetTimeoutEnd ().GetSeconds () << "s");
  // 4. Schedule MAC-low send event for packet burst:
  NS_ASSERT (m_accessTimeout.IsExpired ());
  m_txStartEvent = Simulator::Schedule (txDelay, &CollisionFreeMacImpl::StartTransmission, m_mac, currentPacket);
  StartAccessIfNeeded ();
}

Time
AccessManager::CalculateTxStartTime ()
{
  Time nbrTimeoutEnd = Simulator::Now ();
  NS_ASSERT (m_mac->GetPhy ()->GetObject<Phy> () != 0);
  UpdateInterferenceNeighbors ();
  for (std::vector<Ptr<NetDevice> >::const_iterator i = m_interferenceNeighbors.begin (); i != m_interferenceNeighbors.end (); i++)
    {
      Ptr<NeighborAwareDeviceImpl> device = (*i)->GetObject<NeighborAwareDeviceImpl> ();
      if (device == 0)
        {
          continue;
        }
      Ptr<CollisionFreeMacImpl> mac = device->GetMac ()->GetObject<CollisionFreeMacImpl>();
      NS_ASSERT (mac != 0);
      Time current = mac->GetAccessManager ()->GetTimeoutEnd ();
      if (nbrTimeoutEnd < current)
        {
          nbrTimeoutEnd = current;
        }
    }
  return nbrTimeoutEnd;
}

Time
AccessManager::GetTimeoutEnd ()
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now ();
  if (m_timeoutEnd < now)
    {
      return now;
    }
  NS_LOG_DEBUG (this << ". Timeout end:" << m_timeoutEnd);
  return m_timeoutEnd;
}

void
AccessManager::SetQueue (Ptr<Queue<Packet> > queue)
{
  m_queue = queue;
}

Ptr<Queue<Packet> >
AccessManager::GetQueue () const
{
  return m_queue;
}

void
AccessManager::DoDispose ()
{
  m_accessTimeout.Cancel ();
  m_txStartEvent.Cancel ();
  m_queue = 0;
  m_mac = 0;
}

void
AccessManager::UpdateInterferenceNeighbors ()
{
  Time now = Simulator::Now ();
  if (m_lastInterferenceNeighborsUpdate + m_interferenceNeighborUpdatePeriod > now)
    {
      return;
    }
  m_lastInterferenceNeighborsUpdate = now;
  m_interferenceNeighbors = m_mac->GetPhy ()->GetObject<Phy> ()->GetInterferenceNeighbors ();
}

} // namespace lrr
} // namespace ns3
