/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef GPSR1PACKET_H
#define GPSR1PACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {
namespace gpsr1 {



enum MessageType
{
  GPSR1TYPE_HELLO  = 1,         //!< GPSRTYPE_HELLO
  GPSR1TYPE_POS = 2,            //!< GPSRTYPE_POS
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
  HelloHeader (uint64_t originPosx = 0, uint64_t originPosy = 0 ,uint64_t originPosz = 0, uint64_t originVx = 0, uint64_t originVy = 0 ,uint64_t originVz = 0 );

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

  //\}


  bool operator== (HelloHeader const & o) const;
private:
  uint64_t         m_originPosx;          ///< Originator Position x
  uint64_t         m_originPosy;          ///< Originator Position y
  uint64_t         m_originPosz;          ///< Originator Position z
  uint64_t         m_originVx;          ///< Originator Velocity x
  uint64_t         m_originVy;          ///< Originator Velocity y
  uint64_t         m_originVz;          ///< Originator Velocity z
};

std::ostream & operator<< (std::ostream & os, HelloHeader const &);

class PositionHeader : public Header
{
public:
  /// c-tor
  PositionHeader (uint64_t dstPosx = 0, uint64_t dstPosy = 0, uint64_t dstPosz = 0, uint32_t updated = 0, uint64_t recPosx = 0, uint64_t recPosy = 0, uint64_t recPosz = 0, uint8_t inRec  = 0, uint64_t lastPosx = 0, uint64_t lastPosy = 0, uint64_t lastPosz = 0, uint8_t flag  = 0, uint64_t cPosx = 0, uint64_t cPosy = 0, uint64_t cPosz = 0);

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

  void SetFlag (uint8_t flag)
  {
    m_flag = flag;
  }
  uint8_t GetFlag () const
  {
    return m_flag;
  }
  void SetCPosx (uint64_t cposx)
  {
    m_cPosx = cposx;
  }
  uint64_t GetCPosx () const
  {
    return m_cPosx;
  }
  void SetCPosy (uint64_t cposy)
  {
    m_cPosy = cposy;
  }
  uint64_t GetCPosy () const
  {
    return m_cPosy;
  }
  void SetCPosz (uint64_t cposz)
  {
    m_cPosz = cposz;
  }
  uint64_t GetCPosz () const
  {
    return m_cPosz;
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
  uint8_t          m_flag;          ///< 1 if in Recovery-mode, 0 otherwise
  uint64_t         m_cPosx;          ///< x of position of previous hop
  uint64_t         m_cPosy;          ///< y of position of previous hop
  uint64_t         m_cPosz;          ///< z of position of previous hop

};

std::ostream & operator<< (std::ostream & os, PositionHeader const &);

}
}
#endif /* GPSRPACKET_H */
