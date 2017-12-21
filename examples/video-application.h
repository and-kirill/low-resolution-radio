#ifndef VIDEO_APPLICATION_H
#define VIDEO_APPLICATION_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Address;
class RandomVariable;
class Socket;
/**
 * \brief generates video of MPEG4 177x144@20.
 * Auto-regression is used to fit gathered statistics for I frames, B and P frames are supposed to be independent
 */
class VideoApplication : public Application
{
public:
  static TypeId GetTypeId ();

  VideoApplication ();
  void SetPeerAddress (Address address);

  virtual ~VideoApplication();

  Ptr<Socket> GetSocket () const;
private:
  ///\name Inherited from the application:
  ///\{
  virtual void DoDispose ();
  virtual void StartApplication ();
  virtual void StopApplication ();
  ///\}
  /// Connect a socket:
  void Connect ();
  /// Send a packet. To avoid IP fragmentation, big packets are splitted into smaller once to fit MSDU size 1500 (1460 with no headers) bytes
  void SendPacket ();
  /// Get next packet of a proper type and size
  uint32_t GetNextPacketSize ();
  /// generate I (reference) frame
  uint32_t GenerateILength ();
  /// We do not make any difference between B or P frames when modeling MPEG-4 video traffic
  uint32_t GenerateBPLength ();
  /// Calculate normal distribution CDF from a sample value for I-frames
  double NormalCdf (double value) const;
  /// Calculate log-normal distribution CDF from a sample value for I-frames
  double LogNormalCdf (double value) const;
  /// Calculate inverse log-normal distribution function for I-frames. Return uint32_t as it is a frame length
  uint32_t InverseLogNormalCdf (double value) const;
private:
  ///\name Application functional internals:
  ///\{
  Address m_peerAddress;
  Ptr<Socket> m_socket;
  Time m_packetInterval;
  EventId m_nextSendEvent;
  ///\}
private:
  ///\name Video codec model internals
  ///\{
  /// The number of frames on group of picture. Each GOP starts with I-frame
  uint32_t m_gopSize;
  /// Last generated frame:
  uint32_t m_frameCount;
  /// Maximum allowed frame length
  uint32_t m_maxFrameLength;
  /// Min frame length (about 30% of BP frames has length of 12 bytes)
  uint32_t m_minFrameLength;
  /// Probability for the minimum length-frame to occur (only among BP ones)
  double m_minFrameProbability;
  /// Variance of I-frame size samples, which is used to generate normally distributed correlated data to be converted to log-normal distribution
  double m_iSampleVar;
  ///\name Log normal distribution parameters for I and BP frames:
  ///\{
  double m_iLogNormalsigma;
  double m_iLogNormalmean;
  double m_bpLogNormalSigma;
  double m_bpLogNormalMean;
  ///\}
  /// The size of AR coefficients
  uint32_t m_iMemorySize;
  /// Auto-regression coefficients for I-frames
  std::vector<double> m_iFrameARcoefficients;
  /// Last generated AR values for I-frames stored in a circular buffer
  std::vector <double> m_iFramesMemory;
  /// Index in a circular buffer used for current I-frame to be generated
  uint32_t m_iFrameMemoryIndex;
  /// Generator for white noise additions in AR-model
  Ptr<NormalRandomVariable> m_iFrameGenerator;
  ///\}
  // BP frames generation variables (uncorrelated)
  Ptr<LogNormalRandomVariable> m_bpX;
  Ptr<UniformRandomVariable> m_bpY;
};

} // namespace ns3

#endif /* ONOFF_APPLICATION_H */
