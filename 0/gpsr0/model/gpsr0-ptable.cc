#include "gpsr0-ptable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>
#include <math.h>

NS_LOG_COMPONENT_DEFINE ("Gpsr0Table");


namespace ns3 {
namespace gpsr0 {


 TableEntry::TableEntry (Vector position,
	     		 uint8_t routingtimes,
	     		 uint32_t packetcounts,
	     		 uint32_t packetsuccess,
			 double trust,
 			 Vector velocity, 
			 Time Updatetime,
			 Time livetime			 
			)
   : m_position (position),
     m_times(routingtimes),
     m_count(packetcounts),
     m_suc(packetsuccess),
     m_trust (trust),
     m_v (velocity),
     m_time(Updatetime),
     m_livetime(livetime)
 {

 }
TableEntry::~TableEntry()
{
}



 TableEntry2::TableEntry2 (Ipv4Address dst,
			   Ipv4Address nextHop,	
			   Time livetime2	 
			)
   : m_dst (dst),
     m_nextHop(nextHop),
     m_livetime2(livetime2)

 {

 }
TableEntry2::~TableEntry2()
{
}


/*
  GPSR position table
*/

PositionTable::PositionTable ()
{
  m_txErrorCallback = MakeCallback (&PositionTable::ProcessTxError, this);
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello
  //m_trust.resize(8);
  
}
Time 
PositionTable::GetEntryUpdateTime (Ipv4Address id)
{
  if (id == Ipv4Address::GetZero ())
    {
      return Time (Seconds (0));
    }
  std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  return i->second.Gettime();
}

/**
 * \brief Adds entry in position table
 */
void 
PositionTable::AddEntry (Ipv4Address id, TableEntry rt)
{
  std::map<Ipv4Address, TableEntry >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {
      TableEntry rt0 = i->second;
      rt0.setPos(rt.GetPos());
      rt0.setVel(rt.GetVel());
      rt0.settime(Simulator::Now ());
      m_table.erase (id);
      m_table.insert (std::make_pair (id, rt0));
      return;
    }
  std::vector<double> trust(8,0.5);
  m_table.insert (std::make_pair (id, rt));
  m_trust.insert (std::make_pair (id, trust));
  
}

void 
PositionTable::updatetrust (Ipv4Address a, Ipv4Address b, double trust)
{
   if ( a != Ipv4Address::GetZero ())
{
   std::map<Ipv4Address, std::vector<indirecttrust_t> >::iterator i = m_table3.find(a);
   if (i == m_table3.end())
   {
	struct indirecttrust_t intrust={b,trust};
	std::vector<indirecttrust_t> trust_list;
	trust_list.insert(trust_list.begin(),intrust);
	m_table3.insert(std::make_pair (a, trust_list));
   }
   else
   {
	int count = i->second.size();	
	for (int ii=0; ii<count; ii++)
	{
	    if (i->second.at(ii).nodeaddress == b)
	    {
		i->second.at(ii).trustvalue = trust;
		return;
	    }
	}
 	struct indirecttrust_t intrust={b,trust};
	i->second.insert(i->second.begin(),intrust);
   }
}
}

/**
 * \brief Deletes entry in position table
 */
void PositionTable::DeleteEntry (Ipv4Address id)
{
  m_table.erase (id);
}

void PositionTable::GetListOfAlltrust (std::map<Ipv4Address, double> &trustlist)
{
  for (std::map<Ipv4Address, TableEntry>::iterator i= m_table.begin (); i != m_table.end (); i++)
  {
      trustlist.insert(std::make_pair (i->first,i->second.Gettrust0()));
	//std::cout<<"1: "<< i->second.Gettrust0()<<std::endl;
  }
}

/**
 * \brief Gets position from position table
 * \param id Ipv4Address to get position from
 * \return Position of that id or NULL if not known
 */

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


Vector 
PositionTable::GetVelocity (Ipv4Address id)
{

  NodeList::Iterator listEnd = NodeList::End ();
  for (NodeList::Iterator i = NodeList::Begin (); i != listEnd; i++)
    {
      Ptr<Node> node = *i;
      if (node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () == id)
        {
          return node->GetObject<MobilityModel> ()->GetVelocity ();
        }
    }
  return PositionTable::GetInvalidPosition ();

}

uint8_t 
PositionTable::Getroutingtimes (Ipv4Address id)
{
  std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  return i->second.Gettimes();
}

uint32_t 
PositionTable::Getpacketcounts (Ipv4Address id)
{
  std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  return i->second.Getcount();
}

uint32_t 
PositionTable::Getpacketsuc (Ipv4Address id)
{
  std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  return i->second.Getcount();
}

double 
PositionTable::Gettrust(Ipv4Address id, int a)
{
  std::map<Ipv4Address, std::vector<double> >::iterator j = m_trust.find (id);
  if (j != m_trust.end () || id.IsEqual (j->first))
  {
  	return j->second.at(a);
  }
  else
  {
  	return -1;
  }
}


void 
PositionTable::Settrust (Ipv4Address id, TableEntry rt)
{
  //std::ostringstream addrOss;
  //id.Print(addrOss);
  //std::cout << addrOss.str().c_str() << std::endl;
  //std::cout<<"5"<<std::endl;
  //std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  std::map<Ipv4Address, std::vector<double> >::iterator j = m_trust.find (id);
  std::map<Ipv4Address, TableEntry >::iterator i = m_table.find (id);
  int count=(int)rt.Getcount();
  double trust;
  if (count == 0)
  { trust = 0.5; }
  else{
  int suc = (int)rt.Getsuc();
  double U=(double)(1-(0.1)/(count+0.1));
  double D;
  if (count==0)
  { D= 0.5; }
  else
  { D= (double)((double)suc/(double)count);}

/*
  if (D>1)
  {
     std::cout<<suc<<std::endl;
     std::cout<<count<<std::endl;
     std::ostringstream addrOss;
     id.Print(addrOss);
     std::cout << addrOss.str().c_str() << std::endl;
     std::cout<<Simulator::Now ().GetSeconds()<<std::endl;
  }
*/

  trust = U*D;

  }
  //std::cout<<"trust: "<<count<<std::endl;
  j->second.erase(j->second.begin());
  j->second.push_back(trust);
  
  double fo_trust0 = 0;
  for (int ii=0; ii<8; ii++)
  {
	fo_trust0 = fo_trust0 + j->second.at(ii);
       
  }  
  fo_trust0 = fo_trust0/8;
  i->second.settrust0(fo_trust0);

}


void 
PositionTable::Settrust (Ipv4Address id, double trust)
{
  std::map<Ipv4Address, std::vector<double> >::iterator j = m_trust.find (id);
  std::map<Ipv4Address, TableEntry >::iterator i = m_table.find (id);
  j->second.erase(j->second.begin());
  j->second.push_back(trust);
  double fo_trust0 = 0;
  for (int ii=0; ii<8; ii++)
  {
	fo_trust0 = fo_trust0 + j->second.at(ii);
       
  }  
  fo_trust0 = fo_trust0/8;
  i->second.settrust0(fo_trust0);
}


void
PositionTable::Setroutingtimes (Ipv4Address id,uint8_t times)
{
  std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  uint8_t tt=i->second.Gettimes()+times;
  i->second.settimes(tt);
}


void 
PositionTable::Settable2 (Ipv4Address dst, Ipv4Address nextHop)
{
   std::map<Ipv4Address, TableEntry2>::iterator i2 = m_table2.find (dst);
   if (i2 == m_table2.end())
   {
	TableEntry2 table2(dst, nextHop, Seconds(int(Simulator::Now ().GetSeconds())));
	m_table2.insert(std::make_pair(dst,table2));
   }

}


void
PositionTable::Setpacketcounts (Ipv4Address id,uint32_t counts)
{
  //std::cout<<"6"<<std::endl;
  std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  std::map<Ipv4Address, Time>::iterator lastTime = m_lastTime.find (id);
  if (lastTime == m_lastTime.end ())
  {
      m_lastTime.insert(std::make_pair(id,Time (Seconds (0))));
      lastTime = m_lastTime.find (id);
  }
  if (lastTime->second+Time (Seconds (8))<Simulator::Now ())
  {
     i->second.setcount(counts);
     i->second.settimes((uint8_t)1);
     i->second.setsuc((uint32_t)0);
     std::map<Ipv4Address, std::vector<double> >::iterator j = m_trust.find (id);
     for (int ii=0; ii<8; ii++)
     {
	j->second.at(ii)=0.5;
     }
     
     lastTime->second = Seconds(int(Simulator::Now ().GetSeconds()));
    // std::cout<< lastTime->second <<std::endl;
  }
  else
  {
       if (lastTime->second +Time (Seconds (1))<Simulator::Now ())
       { 
        //int64_t temptime =ToInteger (Time::S);
        Settrust(id,i->second);
        i->second.setcount(counts);
        i->second.settimes((uint8_t)1);
        i->second.setsuc((uint32_t)0);
        lastTime->second = lastTime->second +Time (Seconds (1));
        Redopacketcounts(id);
       }
       else
       {
        uint32_t tt=i->second.Getcount()+counts;
        i->second.setcount(tt);

/*
	if (i->second.Getsuc() > tt)
	{
	   std::ostringstream addrOss;
    	   id.Print(addrOss);
    	   std::cout << addrOss.str().c_str() << std::endl;
	   //std::cout<<lastTime->second.GetSeconds()<<std::endl;
	   //std::cout<<Simulator::Now ().GetSeconds()<<std::endl;
	   std::cout<<"success: "<<(int)i->second.Getsuc()<<std::endl;
	   std::cout<<"counts: "<<tt<<std::endl;
	}
*/

       }
  }
}

void
PositionTable::Redopacketcounts (Ipv4Address id)
{
   std::map<Ipv4Address, Time>::iterator lastTime = m_lastTime.find (id);
   if (lastTime->second +Time (Seconds (1))<Simulator::Now ())
       {
           Settrust(id, 0.5);
           lastTime->second= lastTime->second+Time (Seconds (1));
	   Redopacketcounts(id);
       }
}


void
PositionTable::Setpacketsuc (Ipv4Address id,uint32_t suc)
{
  //std::cout<<"7"<<std::endl;
  std::map<Ipv4Address, TableEntry>::iterator i = m_table.find (id);
  uint32_t tt=i->second.Getsuc()+suc;
  i->second.setsuc(tt);
}



/**
 * \brief Checks if a node is a neighbour
 * \param id Ipv4Address of the node to check
 * \return True if the node is neighbour, false otherwise
 */
bool
PositionTable::isNeighbour (Ipv4Address id)
{

 std::map<Ipv4Address, TableEntry >::iterator i = m_table.find (id);
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
  if (!m_table2.empty())
  {
    std::list<Ipv4Address> toErase2;
    std::map<Ipv4Address, TableEntry2 >::iterator i2 = m_table2.begin ();
    std::map<Ipv4Address, TableEntry2 >::iterator listEnd2 = m_table2.end ();
    for (; !(i2 == listEnd2); i2++)
    {
        if (i2->second.Getlivetime2()+Time (Seconds (1))<=Simulator::Now ())
	{
		toErase2.insert (toErase2.begin (), i2->first);
	}
    }
    toErase2.unique ();
    std::list<Ipv4Address>::iterator end2 = toErase2.end ();
  
  for (std::list<Ipv4Address>::iterator it2 = toErase2.begin (); it2 != end2; ++it2)
    {
      m_table2.erase (*it2);

    }
  }


  if(m_table.empty ())
    {
      return;
    }

  std::list<Ipv4Address> toErase;

  std::map<Ipv4Address, TableEntry >::iterator i = m_table.begin ();
  std::map<Ipv4Address, TableEntry >::iterator listEnd = m_table.end ();
  
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
PositionTable::BestNeighbor (Ipv4Address dst, Vector position, Vector nodePos, Vector myvel)
{
  Purge ();
  if (m_table.empty ())
  {
     NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << position);
     return Ipv4Address::GetZero ();
  }     //if table is empty (no neighbours)

  if (!m_table2.empty())
  {
     std::map<Ipv4Address, TableEntry2>::iterator i20 = m_table2.find (dst);
     if (i20 != m_table2.end () && (isNeighbour(i20->second.GetnextHop())))
     {

//std::cout<< "time0"<<Simulator::Now ()<<std::endl;
	return i20->second.GetnextHop();
     }
  }
// 计算信任值
  double all_trust = 0;
  int all_times=0;
  std::map<Ipv4Address, double> mean_trust;
  std::map<Ipv4Address, double> final_trust;
  for (std::map<Ipv4Address, TableEntry>::iterator i= m_table.begin (); i != m_table.end (); i++)
  {
     Ipv4Address id = i->first;
     std::map<Ipv4Address, std::vector<double> >::iterator j = m_trust.find (id);
     double all_trust = 0;
     double fo_trust0 = i->second.Gettrust0();


     double temp_trust = fo_trust0*i->second.Gettimes();
     all_times =  all_times + (int)(i->second.Gettimes());
     all_trust = all_trust + temp_trust;
     mean_trust.insert(std::make_pair(id, temp_trust));
  }
  
  

  for (std::map<Ipv4Address, TableEntry>::iterator i= m_table.begin (); i != m_table.end (); i++)
  {
     Ipv4Address id = i->first;
     std::map<Ipv4Address, std::vector<double> >::iterator j = m_trust.find (id);
     
     std::map<Ipv4Address, double >::iterator part2 = mean_trust.find (id);
     double trust_part1 = i->second.Gettrust0();
     double trust_part2 = 0;
     double alpha;
    if(all_times!=0)
   {
     for (std::map<Ipv4Address, double>::iterator k= mean_trust.begin (); k!= mean_trust.end (); k++)
     {
	double tt=-1 ;

	if (all_trust-part2->second ==0)
	{ 
		break;
	}
	if (k->first != id)
        {	  

        std::map<Ipv4Address, std::vector<indirecttrust_t> >::iterator intrust = m_table3.find (k->first);
	if (intrust != m_table3.end())
	{    
	    int count = intrust->second.size();
	    if (count == 0)
	   { tt=0.5; }
	    else
	   {	
	    for (int ii=0; ii<count; ii++)
	    {
	    	if (intrust->second.at(ii).nodeaddress == id)
	    	{
		   tt = intrust->second.at(ii).trustvalue ;
		   break;
	    	}
		else
		{ tt =0.5; }
	    }

	   }
	}

	   //tt = 1; 
	   //std::cout<< tt <<std::endl;
	   trust_part2 = trust_part2 + tt*(k->second)/(all_trust-part2->second);
        }
     }
     int w = (int) i-> second.Gettimes();
     double alpha_1 = w/all_times;
     double alpha_2 = 1- w/all_times;
     if (alpha_1 > alpha_2)
     {
        alpha=alpha_1;
     }
     else
     { alpha=alpha_2;}
   }
   else
   { alpha=1;}

     double final_trust0 = alpha*trust_part1 + (1-alpha)*trust_part2;
if(!(final_trust0==final_trust0))
{
	std::cout << "yes: " << trust_part2 <<std::endl;
}
     //std::cout << final_trust0 <<std::endl;
     final_trust.insert(std::make_pair(id, final_trust0));
 
 }

// 信任度计算完成 final_trust
  double a_x = fmod(position.x, 500);
  if (a_x >250)
  {
     a_x = a_x-500;
  }
  double a_y = fmod(position.y, 300);
  if (a_y >150)
  {
     a_y = a_y-300;
  }
  double R= CalculateDistance (Vector(a_x,a_y,0), Vector(0,0,0));
  if (R<=100)
  {
   //std::cout<<"1"<<std::endl;
// 交叉路口
  Ipv4Address bestFoundID = m_table.begin ()->first;
  Ipv4Address bestFoundID1 = m_table.begin ()->first;
  std::map<Ipv4Address, TableEntry >::iterator i;
  double bestFoundDistance = 50000;


  double bestFoundDistance1 = 50000;
  int t=0;
  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      Ipv4Address id = i->first;
      Vector vel = i->second.GetVel();
      std::map<Ipv4Address, double >::iterator j = final_trust.find (id);
      double trust_value = j->second;
/*
if (trust_value < 0.1)
{
 std::cout<<"trust_value: "<<trust_value<<std::endl;
std::ostringstream addrOss;
id.Print(addrOss);
std::cout << addrOss.str().c_str() << std::endl;
}
*/

      if (trust_value >= 0.1)
        {

          double temp_flag = vel.x*(position.x-nodePos.x)+vel.y*(position.y-nodePos.y)+vel.z*(position.z-nodePos.z);
	  if (temp_flag !=0)
	{
	  temp_flag=temp_flag/(CalculateDistance (nodePos, position)* CalculateDistance (vel, Vector(0,0,0)));
	}

          if (temp_flag > 100)
          {
		t=1;
             if (bestFoundDistance > CalculateDistance (i->second.GetPos(), position))
	     {
          	bestFoundID = i->first;
          	bestFoundDistance = CalculateDistance (i->second.GetPos(), position);
	     }
	  }
	  else
	  {

	     if (t==0)
	    { 
	     if (bestFoundDistance1 > CalculateDistance (i->second.GetPos(), position))
	     {
          	bestFoundID1 = i->first;
          	bestFoundDistance1 = CalculateDistance (i->second.GetPos(), position);
	     }
	    }
	  }          
        }
/*
     else
	{
		t=1;
 	    if (bestFoundDistance > CalculateDistance (i->second.GetPos(), position))
	     {
          	bestFoundID = i->first;
          	bestFoundDistance = CalculateDistance (i->second.GetPos(), position);
	     }
	}
*/
     }

  if (t==1)
  {
  	return bestFoundID;
  }
  else
  {

        return bestFoundID1;
  }
  	
  }
  else
  {
// 直行路段
     //std::cout<<"4"<<std::endl;
     double initialDistance = CalculateDistance (nodePos, position); 
     //double R0 = 250;
     double best_flag=-1;
     std::map<Ipv4Address, TableEntry >::iterator i;
     Ipv4Address bestFoundID = m_table.begin ()->first;
     for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      Ipv4Address id = i->first;
      //Vector nexvel = i->second.GetVel();
      //Vector nexPos = i->second.GetPos();
      std::map<Ipv4Address, double >::iterator j = final_trust.find (id);
      double trust_value = j->second;
      double temp_flag;
      double temp_distance = CalculateDistance (i->second.GetPos(), position);
      //std::cout << trust_value <<std::endl;

/*
if (trust_value < 0.1)
{
 std::cout<<"trust_value2: "<<trust_value<<std::endl;
 std::ostringstream addrOss;
 id.Print(addrOss);
 std::cout << addrOss.str().c_str() << std::endl;
}
*/
      if (trust_value >= 0.1)
      {
	//double T;
	/*
        if (myvel.x * nexvel.x + myvel.y * nexvel.y == 0)
        { 
	  continue; 
	}
        if (myvel.y == 0) //东西方向移动
        {
	   if (myvel.x - nexvel.x < 0.5 && myvel.x - nexvel.x > -0.5)
           { 
		bestFoundID = id;
		break;
           }
	   if ( myvel.x * nexvel.x > 0 ) //同向
	   {
		if ( myvel.x >= nexvel.x )
		{
		   T = R0*R0-(nodePos.y-nexPos.y)*(nodePos.y-nexPos.y);
		   T = ( sqrt(T) + (nexPos.x-nodePos.x) )/(myvel.x-nexvel.x);
		  //T = T*15/R0;
		}
		else
		{
		   T = R0*R0-(nodePos.y-nexPos.y)*(nodePos.y-nexPos.y);
		   T = ( sqrt(T) - (nexPos.x-nodePos.x) )/(nexvel.x-myvel.x);
		  // T = T*15/R0;
		}
	   }
	   else 
	   {
		if ((myvel.x-nexvel.x)*(nodePos.x-nexPos.x)>0)//相向
		{
		   T = R0*R0-(nodePos.y-nexPos.y)*(nodePos.y-nexPos.y);
		   T = ( sqrt(T) - (nexPos.x-nodePos.x) )/(myvel.x+nexvel.x);
		   //T = T*15/R0;
		}
		else//相对
		{
 		   T = R0*R0-(nodePos.y-nexPos.y)*(nodePos.y-nexPos.y);
		   T = ( sqrt(T) + (nexPos.x-nodePos.x) )/(myvel.x+nexvel.x);
		   //T = T*15/R0;
		}
	   }
        }
	else //南北方向
	{
             if (myvel.y - nexvel.y < 0.5 && myvel.y - nexvel.y > -0.5)
           { 
		bestFoundID = id;
		break;
           }
	   if ( myvel.y * nexvel.y > 0 ) //同向
	   {
		if ( myvel.y >= nexvel.y )
		{
		   T = R0*R0-(nodePos.x-nexPos.x)*(nodePos.x-nexPos.x);
		   T = ( sqrt(T) + (nexPos.y-nodePos.y) )/(myvel.y-nexvel.y);
		   //T = T*15/R0;
		}
		else
		{
		   T = R0*R0-(nodePos.x-nexPos.x)*(nodePos.x-nexPos.x);
		   T = ( sqrt(T) - (nexPos.y-nodePos.y) )/(nexvel.y-myvel.y);
		   //T = T*15/R0;
		}
	   }
	   else 
	   {
		if ((myvel.y-nexvel.y)*(nodePos.y-nexPos.y)>0)//相向
		{
		   T = R0*R0-(nodePos.x-nexPos.x)*(nodePos.x-nexPos.x);
		   T = ( sqrt(T) - (nexPos.y-nodePos.y) )/(myvel.y+nexvel.y);
		  // T = T*15/R0;
		}
		else//相对
		{
 		   T = R0*R0-(nodePos.x-nexPos.x)*(nodePos.x-nexPos.x);
		   T = ( sqrt(T) + (nexPos.y-nodePos.y) )/(myvel.y+nexvel.y);
		   //T = T*15/R0;
		}
	   }
	}
*/
//std::cout<<T<<std::endl;
//std::cout<<"distance: "<<initialDistance/temp_distance<<std::endl;
	if (i->second.GetPos().z == nodePos.z)
        {
	   
	   //temp_flag = 0.5*(initialDistance/temp_distance)+(1-0.5)*T;
		temp_flag = 0.5*(initialDistance/temp_distance);
        }
	else
        {
  	   //temp_flag = 0.5*(initialDistance/temp_distance)+(1-0.5)*T;
		temp_flag = 0.5*(initialDistance/temp_distance);
        }
      }
      else
      {  temp_flag = -1; }

      if (temp_flag > best_flag)
      {
	 bestFoundID = i->first;
	 best_flag = temp_flag;
      }
 
    }
    if (best_flag == -1)
    { bestFoundID=Ipv4Address::GetZero (); }
    //std::ostringstream addrOss;
    //bestFoundID.Print(addrOss);
    //std::cout << addrOss.str().c_str() << std::endl;
    //std::cout << "value: " << best_flag << std::endl;
    return bestFoundID;

  }

}






/**
 * \brief Gets next hop according to GPSR recovery-mode protocol (right hand rule)
 * \param previousHop the position of the node that sent the packet to this node
 * \param nodePos the position of the destination node
 * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
 */
/*
Ipv4Address
PositionTable::BestAngle (Vector previousHop, Vector nodePos)
{
  Purge ();

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  double tmpAngle;
  Ipv4Address bestFoundID = Ipv4Address::GetZero ();
  double bestFoundAngle = 360;
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;

  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      tmpAngle = GetAngle(nodePos, previousHop, i->second.first);
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
/*
void
 PositionTable::Print (Ptr<OutputStreamWrapper> stream) const
 {
   Purge ();
   *stream->GetStream () << "\nAODV Routing table\n"
                         << "Destination\tGateway\t\tInterface\tFlag\tExpire\t\tHops\n";
   for (std::map<Ipv4Address, TableEntry>::const_iterator i =
          m_table.begin (); i != m_table.end (); ++i)
     {
       i->second.Print (stream);
     }
   *stream->GetStream () << "\n";
 }
*/
}   // gpsr
} // ns3
