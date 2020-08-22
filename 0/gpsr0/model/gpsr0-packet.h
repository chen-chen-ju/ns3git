/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef GPSR0PACKET_H
#define GPSR0PACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {
namespace gpsr0 {



enum MessageType
{
  GPSR0TYPE_HELLO  = 1,         //!< GPSRTYPE_HELLO
  GPSR0TYPE_POS = 2,     
  GPSR0TYPE_ACK = 3,       //!< GPSRTYPE_POS
};

/**
 * \ingroup gpsr
 * \brief GPSR types
 */
class TypeHeader : public Header
{
public:
  /// c-tor
  TypeHeader (MessageType t);

  ///\name Header serialization/deserialization
  //\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;
  //\}

  /// Return type
  MessageType Get () const
  {
    return m_type;
  }
  /// Check that type if valid
  bool IsValid () const
  {
    return m_valid; //FIXME that way it wont work
  }
  bool operator== (TypeHeader const & o) const;
private:
  MessageType m_type;
  bool m_valid;
};

std::ostream & operator<< (std::ostream & os, TypeHeader const & h);

class HelloHeader : public Header
{
public:
  /// c-tor
  HelloHeader (uint64_t originPosx = 0, uint64_t originPosy = 0 ,uint64_t originPosz = 0 ,uint64_t originVx = 0, uint64_t originVy = 0 ,uint64_t originVz = 0 ,Ipv4Address a = Ipv4Address (), uint64_t trust = 0.5);

  ///\name Header serialization/deserialization
  //\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;
  //\}

  ///\name Fields
  //\{
  void SetOriginPosx (uint64_t posx)
  {
    m_originPosx = posx;
  }
  uint64_t GetOriginPosx () const
  {
    return m_originPosx;
  }
  void SetOriginPosy (uint64_t posy)
  {
    m_originPosy = posy;
  }
  uint64_t GetOriginPosy () const
  {
    return m_originPosy;
  }
  
  void SetOriginPosz (uint64_t posz)
  {
    m_originPosz = posz;
  }
  uint64_t GetOriginPosz () const
  {
    return m_originPosz;
  }

  void SetOriginVx (uint64_t Vx)
  {
    m_originVx = Vx;
  }
  uint64_t GetOriginVx () const
  {
    return m_originVx;
  }
  void SetOriginVy (uint64_t Vy)
  {
    m_originVy = Vy;
  }
  uint64_t GetOriginVy () const
  {
    return m_originVy;
  }
  
  void SetOriginVz (uint64_t Vz)
  {
    m_originVz = Vz;
  }
  uint64_t GetOriginVz () const
  {
    return m_originVz;
  }

  void Seta (Ipv4Address a)
  {
     m_a = a;
  }
  Ipv4Address Geta () const
  {
     return m_a;
  }
  void Settrust (uint64_t trust)
  {
    m_trust = trust;
  }
  uint64_t Gettrust () const
  {
    return m_trust;
  }

  //\}


  bool operator== (HelloHeader const & o) const;
private:
  uint64_t         m_originPosx;          ///< Originator Position x
  uint64_t         m_originPosy;          ///< Originator Position y
  uint64_t         m_originPosz;          ///< Originator Position z
  uint64_t         m_originVx;          ///< Originator Position x
  uint64_t         m_originVy;          ///< Originator Position y
  uint64_t         m_originVz;          ///< Originator Position z
  Ipv4Address      m_a;
  uint64_t         m_trust;
};

std::ostream & operator<< (std::ostream & os, HelloHeader const &);

class PositionHeader : public Header
{
public:
  /// c-tor
  PositionHeader (uint64_t dstPosx = 0, uint64_t dstPosy = 0, uint64_t dstPosz = 0, uint32_t updated = 0, uint64_t recPosx = 0, uint64_t recPosy = 0, uint64_t recPosz = 0, uint8_t inRec  = 0, uint64_t lastPosx = 0, uint64_t lastPosy = 0, uint64_t lastPosz = 0, Ipv4Address lastip = Ipv4Address ());

  ///\name Header serialization/deserialization
  //\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;
  //\}

  ///\name Fields
  //\{
  void SetDstPosx (uint64_t posx)
  {
    m_dstPosx = posx;
  }
  uint64_t GetDstPosx () const
  {
    return m_dstPosx;
  }
  void SetDstPosy (uint64_t posy)
  {
    m_dstPosy = posy;
  }
  uint64_t GetDstPosy () const
  {
    return m_dstPosy;
  }
  void SetDstPosz (uint64_t posz)
  {
    m_dstPosz = posz;
  }
  uint64_t GetDstPosz () const
  {
    return m_dstPosz;
  }
  void SetUpdated (uint32_t updated)
  {
    m_updated = updated;
  }
  uint32_t GetUpdated () const
  {
    return m_updated;
  }
  void SetRecPosx (uint64_t posx)
  {
    m_recPosx = posx;
  }
  uint64_t GetRecPosx () const
  {
    return m_recPosx;
  }
  void SetRecPosy (uint64_t posy)
  {
    m_recPosy = posy;
  }
  uint64_t GetRecPosy () const
  {
    return m_recPosy;
  }
  void SetRecPosz (uint64_t posz)
  {
    m_recPosz = posz;
  }
  uint64_t GetRecPosz () const
  {
    return m_recPosz;
  }
  void SetInRec (uint8_t rec)
  {
    m_inRec = rec;
  }
  uint8_t GetInRec () const
  {
    return m_inRec;
  }
  void SetLastPosx (uint64_t posx)
  {
    m_lastPosx = posx;
  }
  uint64_t GetLastPosx () const
  {
    return m_lastPosx;
  }
  void SetLastPosy (uint64_t posy)
  {
    m_lastPosy = posy;
  }
  uint64_t GetLastPosy () const
  {
    return m_lastPosy;
  }
  void SetLastPosz (uint64_t posz)
  {
    m_lastPosz = posz;
  }
  uint64_t GetLastPosz () const
  {
    return m_lastPosz;
  }
  void Setlastip (Ipv4Address a)
  {
     m_lastip = a;
  }
  Ipv4Address Getlastip () const
  {
     return m_lastip;
  }

  bool operator== (PositionHeader const & o) const;
private:
  uint64_t         m_dstPosx;          ///< Destination Position x
  uint64_t         m_dstPosy;          ///< Destination Position y
  uint64_t         m_dstPosz;          ///< Destination Position z
  uint32_t         m_updated;          ///< Time of last update
  uint64_t         m_recPosx;          ///< x of position that entered Recovery-mode
  uint64_t         m_recPosy;          ///< y of position that entered Recovery-mode
  uint64_t         m_recPosz;          ///< z of position that entered Recovery-mode
  uint8_t          m_inRec;          ///< 1 if in Recovery-mode, 0 otherwise
  uint64_t         m_lastPosx;          ///< x of position of previous hop
  uint64_t         m_lastPosy;          ///< y of position of previous hop
  uint64_t         m_lastPosz;          ///< z of position of previous hop
  Ipv4Address      m_lastip;
};

std::ostream & operator<< (std::ostream & os, PositionHeader const &);


class AckHeader : public Header
 {
 public:
   AckHeader (uint32_t reserved=0,Ipv4Address sender = Ipv4Address ());
 
   static TypeId GetTypeId ();
   TypeId GetInstanceTypeId () const;
   uint32_t GetSerializedSize () const;
   void Serialize (Buffer::Iterator start) const;
   uint32_t Deserialize (Buffer::Iterator start);
   void Print (std::ostream &os) const;
   uint32_t Getnum () const
  {
    return m_reserved;
  }
   Ipv4Address GetAdress() const
  {
    return m_sender;
  }
   bool operator== (AckHeader const & o) const;
 private:
   uint32_t       m_reserved; 
   Ipv4Address      m_sender;
 };
 
 std::ostream & operator<< (std::ostream & os, AckHeader const &);

}
}
#endif /* GPSRPACKET_H */
