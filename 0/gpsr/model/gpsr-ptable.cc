#include "gpsr-ptable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("GpsrTable");


namespace ns3 {
namespace gpsr {

/*
  GPSR position table
*/

PositionTable::PositionTable ()
{
  m_txErrorCallback = MakeCallback (&PositionTable::ProcessTxError, this);
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello

}

Time 
PositionTable::GetEntryUpdateTime (Ipv4Address id)
{
  if (id == Ipv4Address::GetZero ())
    {
      return Time (Seconds (0));
    }
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
  return i->second.second;
}

/**
 * \brief Adds entry in position table
 */
void 
PositionTable::AddEntry (Ipv4Address id, Vector position)
{
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {
      m_table.erase (id);
      m_table.insert (std::make_pair (id, std::make_pair (position, Simulator::Now ())));
      return;
    }
  

  m_table.insert (std::make_pair (id, std::make_pair (position, Simulator::Now ())));
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

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
  {
	return i->second.first;
  }
  return PositionTable::GetInvalidPosition ();

}
/*
Vector 
PositionTable::GetPosition (Ipv4Address id)
{

  NodeList::Iterator listEnd = NodeList::End ();
  for (NodeList::Iterator i = NodeList::Begin (); i != listEnd; i++)
    {
      Ptr<Node> node = *i;
      if (node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () == id)
        {
          return node->GetObject<MobilityModel> ()->GetPosition ();
        }
    }
  return PositionTable::GetInvalidPosition ();

}
*/
/**
 * \brief Checks if a node is a neighbour
 * \param id Ipv4Address of the node to check
 * \return True if the node is neighbour, false otherwise
 */
bool
PositionTable::isNeighbour (Ipv4Address id)
{

 std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
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

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.begin ();
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator listEnd = m_table.end ();
  
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
  double bestFoundDistance = CalculateDistance (m_table.begin ()->second.first, position);
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;
  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      if (bestFoundDistance > CalculateDistance (i->second.first, position))
        {
          bestFoundID = i->first;
          bestFoundDistance = CalculateDistance (i->second.first, position);
        }
    }

  if(initialDistance > bestFoundDistance)
{
    return bestFoundID;
}
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
PositionTable::BestAngle (Vector Dpos, Vector Cpos)
{
  Purge ();

  if (m_table.empty ())
    {
      //NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  Vector a;//向量CD
  a.x = Dpos.x - Cpos.x;
  a.y = Dpos.y - Cpos.y;
  a.z = Dpos.z - Cpos.z; 
  std::cout<<Dpos.x<<","<<Dpos.y<<","<<Dpos.z<<std::endl;
  std::cout<<Cpos.x<<","<<Cpos.y<<","<<Cpos.z<<std::endl;
  //构造与CD垂直的向量，构成平面
  Vector b;
  Vector c;
  if (a.x == 0 && a.y == 0)
  {
    b.x = 0;
    b.y = a.z;
    b.z = 0;
  }
  else
  {
    b.x = a.y;
    b.y = -a.x;
    b.z =0;
  }
  //a,b外积,得到法向量
  c.x = a.y*b.z-a.z*b.y;
  c.y = -a.x*b.z+a.z*b.x;
  c.z = a.x*b.y-a.y*b.x;
  //单位化
  double r= CalculateDistance(c,Vector(0,0,0));
  c.x = c.x/r;
  c.y = c.y/r;
  c.z = c.z/r;
  //平面方程 c和(x-D)的内积为0

  Ipv4Address bestFoundID = Ipv4Address::GetZero ();
  double bestFoundAngle = 360;
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;
  for (i = m_table.begin (); !(i == m_table.end ()); i++)
  {
     // 投影结点和投影后坐标
     Vector x0 = i->second.first; 
     Vector x0t; 
     double t0 = (x0.x*c.x-c.x*Dpos.x+x0.y*c.y-c.y*Dpos.y+x0.z*c.z-c.z*Dpos.z)/(c.x*c.x+c.y*c.y+c.z*c.z);
     //std::cout<<"t0: "<< t0 <<std::endl;
     //投影的点减去t0倍的法向量，落在平面内
     x0t.x = x0.x-t0*c.x;
     x0t.y = x0.y-t0*c.y;
     x0t.z = x0.z-t0*c.z;

     double tmpAngle = GetAngle(Cpos, Dpos, x0t, c);
     std::cout<<tmpAngle<<std::endl;
     if (bestFoundAngle > tmpAngle && tmpAngle != 0)
	{
	  bestFoundID = i->first;
	  bestFoundAngle = tmpAngle;
	}
  }


  if(bestFoundID == Ipv4Address::GetZero ()) //only if the only neighbour is who sent the packet
    {
      bestFoundID = m_table.begin ()->first;
    }
  return bestFoundID;
}

//Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
//x0t投影点，c是之前得到的法向量
double 
PositionTable::GetAngle (Vector Cpos, Vector Dpos, Vector x0t, Vector c){
  double const PI = 4*atan(1);
  Vector a0;//向量CD
  a0.x = Dpos.x-Cpos.x;
  a0.y = Dpos.y-Cpos.y;
  a0.z = Dpos.z-Cpos.z;

  Vector b0;//向量Cx0t
  b0.x = x0t.x - Cpos.x;
  b0.y = x0t.y - Cpos.y;
  b0.z = x0t.z - Cpos.z;

  //夹角的余弦值
  double cosvalue = (a0.x*b0.x+a0.y*b0.y+a0.z*b0.z)/(CalculateDistance(a0,Vector(0,0,0))*CalculateDistance(b0,Vector(0,0,0)));
  double angle = acos(cosvalue);
  angle *= (180/PI); //弧度转角度
  //因为角度是0-2PI的,还要进行判断,利用之前构造平面的CD向量和垂直向量b是逆时针夹角大于PI,即(a,b,c)与(-b,a,0)逆时针夹角大于PI
  //因此，a0与b0的外积方向若与c相同，则说明逆时针夹角大于PI，否则小于PI
  Vector c0;
  c0.x = a0.y*b0.z-a0.z*b0.y;
  c0.y = -a0.x*b0.z+a0.z*b0.x;
  c0.z = a0.x*b0.y-a0.y*b0.x;
  if (c.x*c0.x+c.y*c0.y+c.z*c0.z > 0)
  {
     angle = 360 - angle;
  }

  return angle;

}





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
