/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "gpsr1-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("Gpsr1Packet");

namespace ns3 {
namespace gpsr1 {

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t = GPSR1TYPE_HELLO)
  : m_type (t),
    m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr1::TypeHeader")
    .SetParent<Header> ()
    .AddConstructor<TypeHeader> ()
  ;
  return tid;
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
TypeHeader::GetSerializedSize () const
{
  return 1;
}

void
TypeHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 ((uint8_t) m_type);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8 ();
  m_valid = true;
  switch (type)
    {
    case GPSR1TYPE_HELLO:
    case GPSR1TYPE_POS:
      {
        m_type = (MessageType) type;
        break;
      }
    default:
      m_valid = false;
    }
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
TypeHeader::Print (std::ostream &os) const
{
  switch (m_type)
    {
    case GPSR1TYPE_HELLO:
      {
        os << "HELLO";
        break;
      }
    case GPSR1TYPE_POS:
      {
        os << "POSITION";
        break;
      }
    default:
      os << "UNKNOWN_TYPE";
    }
}

bool
TypeHeader::operator== (TypeHeader const & o) const
{
  return (m_type == o.m_type && m_valid == o.m_valid);
}

std::ostream &
operator<< (std::ostream & os, TypeHeader const & h)
{
  h.Print (os);
  return os;
}

//-----------------------------------------------------------------------------
// HELLO
//-----------------------------------------------------------------------------
HelloHeader::HelloHeader (uint64_t originPosx, uint64_t originPosy, uint64_t originPosz, uint64_t originVx, uint64_t originVy, uint64_t originVz)
  : m_originPosx (originPosx),
    m_originPosy (originPosy),
    m_originPosz (originPosz),
    m_originVx(originVx),
    m_originVy(originVy),
    m_originVz(originVz)
{
}

NS_OBJECT_ENSURE_REGISTERED (HelloHeader);

TypeId
HelloHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr1::HelloHeader")
    .SetParent<Header> ()
    .AddConstructor<HelloHeader> ()
  ;
  return tid;
}

TypeId
HelloHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
HelloHeader::GetSerializedSize () const
{
  return 48;
}

void
HelloHeader::Serialize (Buffer::Iterator i) const
{
  NS_LOG_DEBUG ("Serialize X " << m_originPosx << " Y " << m_originPosy);


  i.WriteHtonU64 (m_originPosx);
  i.WriteHtonU64 (m_originPosy);
  i.WriteHtonU64 (m_originPosz);
  i.WriteHtonU64 (m_originVx);
  i.WriteHtonU64 (m_originVy);
  i.WriteHtonU64 (m_originVz);
}

uint32_t
HelloHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;

  m_originPosx = i.ReadNtohU64 ();
  m_originPosy = i.ReadNtohU64 ();
  m_originPosz = i.ReadNtohU64 ();
  m_originVx = i.ReadNtohU64 ();
  m_originVy = i.ReadNtohU64 ();
  m_originVz = i.ReadNtohU64 ();

  NS_LOG_DEBUG ("Deserialize X " << m_originPosx << " Y " << m_originPosy<< " Z " << m_originPosz);

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
HelloHeader::Print (std::ostream &os) const
{
  os << " PositionX: " << m_originPosx
     << " PositionY: " << m_originPosy
     << " PositionZ: " << m_originPosz
     << " VelocityX: " << m_originVx
     << " VelocityY: " << m_originVy
     << " VelocityZ: " << m_originVz;
}

std::ostream &
operator<< (std::ostream & os, HelloHeader const & h)
{
  h.Print (os);
  return os;
}



bool
HelloHeader::operator== (HelloHeader const & o) const
{
  return (m_originPosx == o.m_originPosx && m_originPosy == o.m_originPosy&& m_originVx == o.m_originVx&& m_originVy == o.m_originVy&& m_originVz == o.m_originVz);
}





//-----------------------------------------------------------------------------
// Position
//-----------------------------------------------------------------------------
PositionHeader::PositionHeader (uint64_t dstPosx, uint64_t dstPosy, uint64_t dstPosz, uint32_t updated, uint64_t recPosx, uint64_t recPosy, uint64_t recPosz, uint8_t inRec, uint64_t lastPosx, uint64_t lastPosy, uint64_t lastPosz, uint8_t flag, uint64_t cPosx, uint64_t cPosy, uint64_t cPosz)
  : m_dstPosx (dstPosx),
    m_dstPosy (dstPosy),
    m_dstPosz (dstPosz),
    m_updated (updated),
    m_recPosx (recPosx),
    m_recPosy (recPosy),
    m_recPosz (recPosz),
    m_inRec (inRec),
    m_lastPosx (lastPosx),
    m_lastPosy (lastPosy),
    m_lastPosz (lastPosz),
    m_flag(flag),
    m_cPosx(cPosx),
    m_cPosy(cPosy),
    m_cPosz(cPosz)
{
}

NS_OBJECT_ENSURE_REGISTERED (PositionHeader);

TypeId
PositionHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr1::PositionHeader")
    .SetParent<Header> ()
    .AddConstructor<PositionHeader> ()
  ;
  return tid;
}

TypeId
PositionHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
PositionHeader::GetSerializedSize () const
{
  return 102;
}

void
PositionHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU64 (m_dstPosx);
  i.WriteU64 (m_dstPosy);
  i.WriteU64 (m_dstPosz);
  i.WriteU32 (m_updated);
  i.WriteU64 (m_recPosx);
  i.WriteU64 (m_recPosy);
  i.WriteU64 (m_recPosz);
  i.WriteU8 (m_inRec);
  i.WriteU64 (m_lastPosx);
  i.WriteU64 (m_lastPosy);
  i.WriteU64 (m_lastPosz);
  i.WriteU8 (m_flag);
  i.WriteU64 (m_cPosx);
  i.WriteU64 (m_cPosy);
  i.WriteU64 (m_cPosz);
}

uint32_t
PositionHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_dstPosx = i.ReadU64 ();
  m_dstPosy = i.ReadU64 ();
  m_dstPosz = i.ReadU64 ();
  m_updated = i.ReadU32 ();
  m_recPosx = i.ReadU64 ();
  m_recPosy = i.ReadU64 ();
  m_recPosz = i.ReadU64 ();
  m_inRec = i.ReadU8 ();
  m_lastPosx = i.ReadU64 ();
  m_lastPosy = i.ReadU64 ();
  m_lastPosz = i.ReadU64 ();
  m_flag = i.ReadU8 ();
  m_cPosx = i.ReadU64 ();
  m_cPosy = i.ReadU64 ();
  m_cPosz = i.ReadU64 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
PositionHeader::Print (std::ostream &os) const
{
  os << " PositionX: "  << m_dstPosx
     << " PositionY: " << m_dstPosy
     << " PositionZ: " << m_dstPosz
     << " Updated: " << m_updated
     << " RecPositionX: " << m_recPosx
     << " RecPositionY: " << m_recPosy
     << " RecPositionZ: " << m_recPosz
     << " inRec: " << m_inRec
     << " LastPositionX: " << m_lastPosx
     << " LastPositionY: " << m_lastPosy
     << " LastPositionZ: " << m_lastPosz
     << " flag: " << m_flag
     << " cPositionX: " << m_cPosx
     << " cPositionY: " << m_cPosy
     << " cPositionZ: " << m_cPosz;
}

std::ostream &
operator<< (std::ostream & os, PositionHeader const & h)
{
  h.Print (os);
  return os;
}



bool
PositionHeader::operator== (PositionHeader const & o) const
{
  return (m_dstPosx == o.m_dstPosx && m_dstPosy == o.m_dstPosy && m_dstPosz == o.m_dstPosz && m_updated == o.m_updated && m_recPosx == o.m_recPosx && m_recPosy == o.m_recPosy && m_recPosz == o.m_recPosz && m_inRec == o.m_inRec && m_lastPosx == o.m_lastPosx && m_lastPosy == o.m_lastPosy && m_lastPosz == o.m_lastPosz && m_flag == o.m_flag && m_cPosx == o.m_cPosx && m_cPosy == o.m_cPosy && m_cPosz == o.m_cPosz);
}


}
}
