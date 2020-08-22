#ifndef GPSR0_PTABLE_H
#define GPSR0_PTABLE_H

#include <map>
#include <cassert>
#include <stdint.h>
#include "ns3/ipv4.h"
#include "ns3/timer.h"
#include <sys/types.h>
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/random-variable-stream.h"
#include <complex>
#include "ns3/output-stream-wrapper.h"

namespace ns3 {
namespace gpsr0 {

class TableEntry
{
public:
  TableEntry(Vector position=Vector(0,0,0),
	     uint8_t routingtimes=0,
	     uint32_t packetcounts=0,
	     uint32_t packetsuccess=0,
	     //std::vector<uint8_t> m_trust,
	     double trust=0,
 	     Vector velocity=Vector(0,0,0),
             Time Updatetime=Simulator::Now(),
	     Time livetime=Simulator::Now()
            );
  ~TableEntry();
  Vector GetPos() const
  {
	return   m_position;
  } 
  void setPos(Vector position) 
  {
	m_position=position;
  }

  uint8_t Gettimes() const
  {
	return   m_times;
  } 
  void settimes(uint8_t times)
  {
     m_times=m_times + times;
    
  }
   uint32_t Getcount() const
  {
	return  m_count;
  } 
  void setcount(uint32_t count) 
  {
	m_count=count;
  }

  uint32_t Getsuc() const
  {
	return  m_suc;
  } 
  void setsuc(uint32_t suc) 
  {
	m_suc=suc;
  }

  double Gettrust0() const
  {
	return  m_trust;
  } 
  void settrust0(double trust) 
  {
	m_trust=trust;
  }

  Vector GetVel() const
  {
	return   m_v;
  } 
  void setVel(Vector velocity)
  {
	m_v=velocity;
  }

  Time Gettime() const
  {
	return  m_time;
  } 
  void settime(Time t)
  {
	m_time=t;
  }
  Time Getlivetime() const
  {
	return  m_livetime;
  } 
  void setlivetime(Time t)
  {
	m_livetime=t;
  }

//void Print (Ptr<OutputStreamWrapper> stream) const;

  //std::vector<double> m_trust;
private:
  Vector m_position;
  uint8_t m_times;
  uint32_t m_count;
  uint32_t m_suc;
  double m_trust;
  Vector m_v;
  Time m_time;
  Time m_livetime;
};


class TableEntry2
{
public:
  TableEntry2(Ipv4Address dst = Ipv4Address(),
	     Ipv4Address nextHop=Ipv4Address(),
	     Time livetime2=Time(Seconds(0))
            );
  ~TableEntry2();
  Ipv4Address Getdst() const
  {
	return m_dst;

  }
  void setdst(Ipv4Address dst)
  {
	m_dst=dst;
  }

  Ipv4Address GetnextHop() const
  {
	return m_nextHop;

  }
  void setnextHop(Ipv4Address nextHop)
  {
	m_nextHop=nextHop;
  }

  Time Getlivetime2() const
  {
	return  m_livetime2;
  } 
  void setlivetime2(Time t)
  {
	m_livetime2=t;
  }

//void Print (Ptr<OutputStreamWrapper> stream) const;

  //std::vector<double> m_trust;
private:
  Ipv4Address m_dst;
  Ipv4Address m_nextHop;
  Time m_livetime2;
};


struct indirecttrust_t
 {
    Ipv4Address nodeaddress;  
    double trustvalue; 
 };

/*
 * \ingroup gpsr
 * \brief Position table used by GPSR
 */
class PositionTable
{
public:
  /// c-tor
  PositionTable ();

  /**
   * \brief Gets the last time the entry was updated
   * \param id Ipv4Address to get time of update from
   * \return Time of last update to the position
   */
  Time GetEntryUpdateTime (Ipv4Address id);

  /**
   * \brief Adds entry in position table
   */
  void AddEntry (Ipv4Address id, TableEntry rt);

void updatetrust (Ipv4Address a, Ipv4Address b, double trust);

  /**
   * \brief Deletes entry in position table
   */
  void DeleteEntry (Ipv4Address id);


void GetListOfAlltrust (std::map<Ipv4Address, double> &trustlist);
  /**
   * \brief Gets position from position table
   * \param id Ipv4Address to get position from
   * \return Position of that id or NULL if not known
   */
  Vector GetPosition (Ipv4Address id);
  Vector GetVelocity (Ipv4Address id);
  uint8_t Getroutingtimes (Ipv4Address id);
  uint32_t Getpacketcounts (Ipv4Address id);
  uint32_t Getpacketsuc (Ipv4Address id);
  double Gettrust(Ipv4Address id, int a);

  void Settrust (Ipv4Address id, TableEntry rt);
  void Settrust (Ipv4Address id, double trust);

  void Setroutingtimes (Ipv4Address id, uint8_t times);
  void Settable2 (Ipv4Address dst, Ipv4Address nextHop);
  void Setpacketcounts (Ipv4Address id, uint32_t counts);
  void Setpacketsuc (Ipv4Address id,uint32_t suc);

  void Redopacketcounts (Ipv4Address id);
  /**
   * \brief Checks if a node is a neighbour
   * \param id Ipv4Address of the node to check
   * \return True if the node is neighbour, false otherwise
   */
  bool isNeighbour (Ipv4Address id);

  /**
   * \brief remove entries with expired lifetime
   */
  void Purge ();

  /**
   * \brief clears all entries
   */
  void Clear ();

  /**
   * \Get Callback to ProcessTxError
   */
  Callback<void, WifiMacHeader const &> GetTxErrorCallback () const
  {
    return m_txErrorCallback;
  }

  /**
   * \brief Gets next hop according to GPSR protocol
   * \param position the position of the destination node
   * \param nodePos the position of the node that has the packet
   * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
   */
  Ipv4Address BestNeighbor (Ipv4Address dst, Vector position, Vector nodePos, Vector myvel);

  bool IsInSearch (Ipv4Address id);

  bool HasPosition (Ipv4Address id);

  static Vector GetInvalidPosition ()
  {
    return Vector (-1, -1, 0);
  }

  //void Print (Ptr<OutputStreamWrapper> stream) const;
  /**
   * \brief Gets next hop according to GPSR recovery-mode protocol (right hand rule)
   * \param previousHop the position of the node that sent the packet to this node
   * \param nodePos the position of the destination node
   * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
   */
  //Ipv4Address BestAngle (Vector previousHop, Vector nodePos);

  //Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
  //double GetAngle (Vector centrePos, Vector refPos, Vector node);



private:
  Time m_entryLifeTime;
  std::map<Ipv4Address, Time> m_lastTime;
  std::map<Ipv4Address, TableEntry> m_table;
  std::map<Ipv4Address, TableEntry2> m_table2;
  std::map<Ipv4Address, std::vector<double> > m_trust;
  std::map<Ipv4Address, std::vector<indirecttrust_t> > m_table3;
  // TX error callback
  Callback<void, WifiMacHeader const &> m_txErrorCallback;
  // Process layer 2 TX error notification
  void ProcessTxError (WifiMacHeader const&);




};

}   // gpsr
} // ns3
#endif /* GPSR_PTABLE_H */
