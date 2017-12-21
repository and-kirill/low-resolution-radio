/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// ns3 - On/Off Data Source Application class
// George F. Riley, Georgia Tech, Spring 2007
// Adapted from ApplicationOnOff in GTNetS.

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/double.h"

#include "video-application.h"

NS_LOG_COMPONENT_DEFINE ("VideoApplication");

using namespace std;

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (VideoApplication);

TypeId
VideoApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::VideoApplication")
    .SetParent<Application> ()
    .AddConstructor<VideoApplication> ()
  ;
  return tid;
}


VideoApplication::VideoApplication () :
  m_peerAddress (Address ()),
  m_socket (0),
  m_packetInterval (MilliSeconds (50)),   // 177X144@20 video MPEG4
  m_gopSize (12),
  m_frameCount (0),
  m_maxFrameLength (178 * 144 * 3),   /// 24 bit BMP image for a given resolution
  m_minFrameLength (12),
  m_minFrameProbability (0.3),
  m_iSampleVar (726564.2),
  m_iLogNormalsigma (0.456426),
  m_iLogNormalmean (7.496176),
  m_bpLogNormalSigma (0.5881435),
  m_bpLogNormalMean (5.8396991),
  m_iMemorySize (19),
  m_iFrameMemoryIndex (0)
{
  m_iFrameGenerator = CreateObject<NormalRandomVariable> ();
  m_iFrameGenerator->SetAttribute ("Mean", DoubleValue (0.0));
  m_iFrameGenerator->SetAttribute ("Variance", DoubleValue (287275.0));

  m_bpX = CreateObject<LogNormalRandomVariable> ();
  m_bpX->SetAttribute ("Mu", DoubleValue (m_bpLogNormalMean));
  m_bpX->SetAttribute ("Sigma", DoubleValue (m_bpLogNormalSigma));
  m_bpY = CreateObject<UniformRandomVariable> ();
  m_bpY->SetAttribute ("Min", DoubleValue (0));
  m_bpY->SetAttribute ("Max", DoubleValue (1));


  /// Initialize memory with independent variables:
  for (unsigned int i = 0; i < m_iMemorySize + 1; i++)
    {
      m_iFramesMemory.push_back (m_iFrameGenerator->GetValue ());
    }
  /// Init AR coefficients:
  m_iFrameARcoefficients.push_back (0.5242628562);
  m_iFrameARcoefficients.push_back (0.1164876915);
  m_iFrameARcoefficients.push_back (0.0341488931);
  m_iFrameARcoefficients.push_back (0.0069402560);
  m_iFrameARcoefficients.push_back (0.0448347600);
  m_iFrameARcoefficients.push_back (0.0217189761);
  m_iFrameARcoefficients.push_back (0.0159739153);
  m_iFrameARcoefficients.push_back (0.0054698796);
  m_iFrameARcoefficients.push_back (0.0272730866);
  m_iFrameARcoefficients.push_back (0.0245200361);
  m_iFrameARcoefficients.push_back (0.0127411007);
  m_iFrameARcoefficients.push_back (0.0314113756);
  m_iFrameARcoefficients.push_back (-0.0333259145);
  m_iFrameARcoefficients.push_back (0.0226299530);
  m_iFrameARcoefficients.push_back (0.0096562527);
  m_iFrameARcoefficients.push_back (0.0135567587);
  m_iFrameARcoefficients.push_back (0.0044526434);
  m_iFrameARcoefficients.push_back (-0.0004917876);
  m_iFrameARcoefficients.push_back (0.0326122941);
}

VideoApplication::~VideoApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

Ptr<Socket>
VideoApplication::GetSocket () const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

void
VideoApplication::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_nextSendEvent.Cancel ();
  m_socket = 0;
  Application::DoDispose ();
}

void
VideoApplication::SetPeerAddress (Address address)
{
  m_peerAddress = address;
}

void
VideoApplication::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  // Create the socket if not already
  if (m_socket == 0)
    {
      Ptr<SocketFactory> socketFactory = GetNode ()->GetObject<SocketFactory> (UdpSocketFactory::GetTypeId ());
      NS_ASSERT (socketFactory != 0);
      m_socket = socketFactory->CreateSocket ();
      m_socket->Bind ();
      NS_ASSERT (m_peerAddress != Address ());
      m_socket->Connect (m_peerAddress);
      m_socket->ShutdownRecv ();
    }
  SendPacket ();
}

void
VideoApplication::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS ();

  m_nextSendEvent.Cancel ();
  if (m_socket != 0)
    {
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("VideoApplication found null socket to close in StopApplication");
    }
}

void
VideoApplication::SendPacket ()
{
  uint32_t size = GetNextPacketSize () + 12 /*RTP header*/;
  // Work-around for IP fragmentation
  static const uint32_t maxPacketSize = 1460;
  while (size != 0)
    {
      if (size > maxPacketSize)
        {
          Ptr<Packet> packet = Create<Packet> (maxPacketSize);
          m_socket->Send (packet);
          size -= maxPacketSize;
        }
      else
        {
          Ptr<Packet> packet = Create<Packet> (size);
          m_socket->Send (packet);
          size = 0;
        }
    }
  m_nextSendEvent = Simulator::Schedule (m_packetInterval, &VideoApplication::SendPacket, this);
}

uint32_t
VideoApplication::GetNextPacketSize ()
{
  uint32_t retval;
  if (m_frameCount % m_gopSize == 0)
    {
      retval = GenerateILength ();
    }
  else
    {
      retval = GenerateBPLength ();
    }
  m_frameCount++;
  return retval;
}

uint32_t
VideoApplication::GenerateILength ()
{
  /// Update current value of AR process using the last ones:
  m_iFramesMemory[m_iFrameMemoryIndex] = 0;
  for (uint32_t j = 0; j < m_iMemorySize; j++)
    {
      unsigned int access = (m_iFrameMemoryIndex + m_iMemorySize - j) % (m_iMemorySize + 1);
      m_iFramesMemory[m_iFrameMemoryIndex] += m_iFramesMemory[access] * m_iFrameARcoefficients[j];
    }
  /// Add white noise:
  m_iFramesMemory[m_iFrameMemoryIndex] += m_iFrameGenerator->GetValue ();
  /// Get the value of CDF function for normal distribution (which is formed by AR process:)
  double normalCdf = NormalCdf (m_iFramesMemory[m_iFrameMemoryIndex]);
  /// Update index in a circular buffer:
  m_iFrameMemoryIndex++;
  if (m_iFrameMemoryIndex == m_iMemorySize + 1)
    {
      m_iFrameMemoryIndex = 0;
    }
  /// return projection to a log-normal distribution:
  return InverseLogNormalCdf (normalCdf);
}

uint32_t
VideoApplication::GenerateBPLength ()
{
  if (m_bpY->GetValue () < m_minFrameProbability)
    {
      return m_minFrameLength;
    }
  return lrint (m_bpX->GetValue ());
}

double
VideoApplication::NormalCdf (double value) const
{
  return 0.5 * (1 + erf ((value) / sqrt (2 * m_iSampleVar)));
}

double
VideoApplication::LogNormalCdf (double value) const
{
  return 0.5 * erfc (-(log (value) - m_iLogNormalmean) / (sqrt (2) * m_iLogNormalsigma));
}

uint32_t
VideoApplication::InverseLogNormalCdf (double cdf) const
{
  static const double precision = 0.5;
  double min = 1;
  double max = m_maxFrameLength;
  double median = (min + max) / 2;
  if (LogNormalCdf (m_maxFrameLength) < cdf)
    {
      return m_maxFrameLength;
    }
  while ((max - min) >= precision)
    {
      median = (max + min) / 2;
      if (LogNormalCdf (median) > cdf)
        {
          max = median;
        }
      else
        {
          min = median;
        }
    }
  return lrint (median);
}
} // Namespace ns3
