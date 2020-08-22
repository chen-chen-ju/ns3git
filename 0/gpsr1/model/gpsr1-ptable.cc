#include "gpsr1-ptable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("Gpsr1Table");


namespace ns3 {
namespace gpsr1 {


TableEntry::TableEntry (Vector position,
 			Vector velocity, 
 			Vector accelerate,
			Time livetime			 
		       )
   : m_position (position),
     m_v (velocity),
     m_acce (accelerate),
     m_livetime(livetime)
 {

 }
TableEntry::~TableEntry()
{
}


/*
  GPSR position table
*/

PositionTable::PositionTable ()
{
  m_txErrorCallback = MakeCallback (&PositionTable::ProcessTxError, this);
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello, 2倍的HelloInterval

}

Time 
PositionTable::GetEntryUpdateTime (Ipv4Address id)
{
  if (id == Ipv4Address::GetZero ())
    {
      return Time (Seconds (0));
    }
  std::map<Ipv4Address, TableEntry >::iterator i = m_table.find (id);
  return i->second.Getlivetime();
}

/**
 * \brief Adds entry in position table
 */
void 
PositionTable::AddEntry (Ipv4Address id, TableEntry rt, Time HelloInterval)
{
  std::map<Ipv4Address, TableEntry >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {
      Time delta = rt.Getlivetime() - i->second.Getlivetime();
      if (delta <= 2*HelloInterval)
      {
      	Vector v0 = i->second.GetVel();
      	Vector v1 = rt.GetVel();
      	Vector a;
	double deltavalue = delta.GetSeconds();
      	a.x = (v1.x-v0.x)/(deltavalue);
	a.y = (v1.y-v0.y)/(deltavalue);
	a.z = (v1.z-v0.z)/(deltavalue);
	rt.setAcce(a);
      }
      m_table.erase (id);
      m_table.insert (std::make_pair (id, rt));
      return;
    }
  

  m_table.insert (std::make_pair (id, rt));
}

/**
 * \brief Deletes entry in position table
 */
void PositionTable::DeleteEntry (Ipv4Address id)
{
  m_table.erase (id);
}

/**
 * \brief Gets position from position table
 * \param id Ipv4Address to get position from
 * \return Position of that id or NULL if not known
 */
Vector 
PositionTable::GetPosition (Ipv4Address id)
{

  std::map<Ipv4Address, TableEntry >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
  {
	return i->second.GetPos();
  }
  return PositionTable::GetInvalidPosition ();

}

Vector 
PositionTable::GetVelocity (Ipv4Address id)
{

  std::map<Ipv4Address, TableEntry >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
  {
	return i->second.GetVel();
  }
  return PositionTable::GetInvalidPosition ();

}


/**
 * \brief Checks if a node is a neighbour
 * \param id Ipv4Address of the node to check
 * \return True if the node is neighbour, false otherwise
 */
bool
PositionTable::isNeighbour (Ipv4Address id)
{

 std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {
      return true;
    }

  return false;
}


/**
 * \brief remove entries with expired lifetime
 */
void 
PositionTable::Purge ()
{

  if(m_table.empty ())
    {
      return;
    }

  std::list<Ipv4Address> toErase;

  std::map<Ipv4Address, TableEntry>::iterator i = m_table.begin ();
  std::map<Ipv4Address, TableEntry>::iterator listEnd = m_table.end ();
  
  for (; !(i == listEnd); i++)
    {

      if (m_entryLifeTime + GetEntryUpdateTime (i->first) <= Simulator::Now ())
        {
          toErase.insert (toErase.begin (), i->first);

        }
    }
  toErase.unique ();

  std::list<Ipv4Address>::iterator end = toErase.end ();

  for (std::list<Ipv4Address>::iterator it = toErase.begin (); it != end; ++it)
    {

      m_table.erase (*it);

    }
}

/**
 * \brief clears all entries
 */
void 
PositionTable::Clear ()
{
  m_table.clear ();
}

/**
 * \brief Gets next hop according to GPSR protocol
 * \param position the position of the destination node
 * \param nodePos the position of the node that has the packet
 * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
 */
// , Ipv4Address myId, int packetsize
Ipv4Address 
PositionTable::BestNeighbor (Vector position, Vector nodePos)
{
  Purge ();

  double initialDistance = CalculateDistance (nodePos, position);

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << position);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)


  Ipv4Address bestFoundID = m_table.begin ()->first;
  double bestFoundDistance = CalculateDistance (m_table.begin ()->second.GetPos(), position);
  std::map<Ipv4Address, TableEntry>::iterator i;
  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      if (bestFoundDistance > CalculateDistance (i->second.GetPos(), position))
        {
          bestFoundID = i->first;
          bestFoundDistance = CalculateDistance (i->second.GetPos(), position);
        }
    }

  if(initialDistance > bestFoundDistance)
{
    return bestFoundID;
}
/*
//首先获得发送结点当前的位置和速度
  Vector LocalC;
  Vector VelC;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); i++)
    {
      Ptr<Node> node = *i;
      if (node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () == id)
        {
          LocalC = node->GetObject<MobilityModel> ()->GetPosition ();
	  VelC = node->GetObject<MobilityModel> ()->GetVelocity ();
        }
    }
//对每个邻居结点
  std::map<Ipv4Address, TableEntry>::iterator i;
  for (i = m_table.begin (); !(i == m_table.end ()); i++)
  {
//邻居结点HELLO包的位置、速度、预测加速度
      Vector Locali = i->second.GetPos();
      Vector Veli = i->second.GetVel();
      Vector Accei = i->second.GetAcce();
      Time 
  }

  if()
  {
    return bestFoundID;
  }
*/
  else
    return Ipv4Address::GetZero (); //so it enters Recovery-mode

}


/**
 * \brief Gets next hop according to GPSR recovery-mode protocol (right hand rule)
 * \param previousHop the position of the node that sent the packet to this node
 * \param nodePos the position of the destination node
 * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
 */

Ipv4Address
PositionTable::BestAngle (Vector dstpos, Vector mypos) //三维的右手法则
{
  Purge ();

  if (m_table.empty ())
    {
      //NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  double dCD=CalculateDistance (dstpos, mypos);
  double dCi;
  double diD;
  double cosangle;
  Ipv4Address bestFoundID = Ipv4Address::GetZero ();
  std::list<Ipv4Address> Possid;
  double bestFoundcos = 360;
  std::map<Ipv4Address, TableEntry >::iterator i;
  for (i = m_table.begin (); !(i == m_table.end ()); i++)
  {
      dCi=CalculateDistance (mypos, i->second.GetPos());
      diD=CalculateDistance (i->second.GetPos(), dstpos);
      cosangle= (dCi*dCi+dCD*dCD-diD*diD)/(2*dCi*dCD);
      if (cosangle<bestFoundcos)
      {
	bestFoundcos=cosangle;
        Possid.clear();
        Possid.insert (Possid.begin (), i->first);
      }
      if (cosangle==bestFoundcos)
      {
	 Possid.insert (Possid.begin (), i->first);
      }
  }
/*
  if (Possid.size()>1)
  {
      bestFoundID = maxunity();
  }
  else
  {
      bestFoundID = Possid.back();
  }
*/
  return bestFoundID;
}

/*
//Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
double 
PositionTable::GetAngle (Vector centrePos, Vector refPos, Vector node){
  double const PI = 4*atan(1);

  std::complex<double> A = std::complex<double>(centrePos.x,centrePos.y);
  std::complex<double> B = std::complex<double>(node.x,node.y);
  std::complex<double> C = std::complex<double>(refPos.x,refPos.y);   //Change B with C if you want angles clockwise

  std::complex<double> AB; //reference edge
  std::complex<double> AC;
  std::complex<double> tmp;
  std::complex<double> tmpCplx;

  std::complex<double> Angle;

  AB = B - A;
  AB = (real(AB)/norm(AB)) + (std::complex<double>(0.0,1.0)*(imag(AB)/norm(AB)));

  AC = C - A;
  AC = (real(AC)/norm(AC)) + (std::complex<double>(0.0,1.0)*(imag(AC)/norm(AC)));

  tmp = log(AC/AB);
  tmpCplx = std::complex<double>(0.0,-1.0);
  Angle = tmp*tmpCplx;
  Angle *= (180/PI);
  if (real(Angle)<0)
    Angle = 360+real(Angle);

  return real(Angle);

}
*/




/**
 * \ProcessTxError
 */
void PositionTable::ProcessTxError (WifiMacHeader const & hdr)
{
}



//FIXME ainda preciso disto agr que o LS ja n está aqui???????

/**
 * \brief Returns true if is in search for destionation
 */
bool PositionTable::IsInSearch (Ipv4Address id)
{
  return false;
}

bool PositionTable::HasPosition (Ipv4Address id)
{
  return true;
}


}   // gpsr
} // ns3
