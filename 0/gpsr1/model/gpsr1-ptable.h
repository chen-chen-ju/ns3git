#ifndef GPSR1_PTABLE_H
#define GPSR1_PTABLE_H

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

namespace ns3 {
namespace gpsr1 {


class TableEntry
{
public:
  TableEntry(Vector position=Vector(0,0,0),
 	     Vector velocity=Vector(0,0,0),
	     Vector accelerate=Vector(0,0,0),
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


  Vector GetVel() const
  {
	return   m_v;
  } 
  void setVel(Vector velocity)
  {
	m_v=velocity;
  }

  Vector GetAcce() const
  {
	return   m_acce;
  } 
  void setAcce(Vector acce)
  {
	m_acce=acce;
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
  Vector m_v;
  Vector m_acce;
  Time m_livetime;
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
  void AddEntry (Ipv4Address id, TableEntry rt, Time HelloInterval);

  /**
   * \brief Deletes entry in position table
   */
  void DeleteEntry (Ipv4Address id);

  /**
   * \brief Gets position from position table
   * \param id Ipv4Address to get position from
   * \return Position of that id or NULL if not known
   */
  Vector GetPosition (Ipv4Address id);

  Vector GetVelocity (Ipv4Address id);

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
//, Ipv4Address myId, int packetsize
  Ipv4Address BestNeighbor (Vector position, Vector nodePos);

  bool IsInSearch (Ipv4Address id);

  bool HasPosition (Ipv4Address id);

  static Vector GetInvalidPosition ()
  {
    return Vector (-1, -1, 0);
  }

  /**
   * \brief Gets next hop according to GPSR recovery-mode protocol (right hand rule)
   * \param previousHop the position of the node that sent the packet to this node
   * \param nodePos the position of the destination node
   * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
   */
  Ipv4Address BestAngle (Vector dstpos, Vector mypos);

  //Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
  //double GetAngle (Vector dstpos, Vector mypos);



private:
  Time m_entryLifeTime;
  std::map<Ipv4Address, TableEntry> m_table;
  // TX error callback
  Callback<void, WifiMacHeader const &> m_txErrorCallback;
  // Process layer 2 TX error notification
  void ProcessTxError (WifiMacHeader const&);




};

}   // gpsr
} // ns3
#endif /* GPSR_PTABLE_H */
