#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/command-line.h"
#include "ns3/string.h"
#include <ns3/log.h>
#include "ns3/network-module.h"
#include "ns3/internet-module.h"


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("touying");

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  LogComponentEnable("touying",LOG_LEVEL_INFO);

  RngSeedManager::SetSeed (2); // Changes seed from default of 1 to 3
  RngSeedManager::SetRun (1);  

  NodeContainer node1;
  node1.Create (4);
  NetDeviceContainer devices1;

  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc1=CreateObject<ListPositionAllocator>();
  positionAlloc1->Add(Vector(9200,9400,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc1);
  mobility.Install(node1.Get(0)); //结点D

 Ptr<ListPositionAllocator> positionAlloc2=CreateObject<ListPositionAllocator>();
  positionAlloc2->Add(Vector(8600,9200,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc2);
  mobility.Install(node1.Get(1)); //结点C

  Ptr<ListPositionAllocator> positionAlloc3=CreateObject<ListPositionAllocator>();
  positionAlloc3->Add(Vector(8300,9400,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc3);
  mobility.Install(node1.Get(2)); //投影结点1

Ptr<ListPositionAllocator> positionAlloc4=CreateObject<ListPositionAllocator>();
  positionAlloc4->Add(Vector(8300,9400,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc4);
  mobility.Install(node1.Get(3)); //投影结点2

  Vector C = node1.Get(1)->GetObject<MobilityModel> ()->GetPosition ();
  Vector D = node1.Get(0)->GetObject<MobilityModel> ()->GetPosition ();
  Vector a;//向量CD
  Vector b;//向量CD的垂直向量，z轴方向为0 a,b构成平面。
  Vector c;//投影平面的法向量
  Vector x0 = node1.Get(2)->GetObject<MobilityModel> ()->GetPosition ();;//投影的点
  Vector x0t;//投影后的点
  Vector x1 = node1.Get(3)->GetObject<MobilityModel> ()->GetPosition ();;//投影的点
  Vector x1t;//投影后的点
  
  a.x = D.x - C.x;
  a.y = D.y - C.y;
  a.z = D.z - C.z; 

//增加考虑a.x=a.y=0的情况
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
//a,b外积
  c.x = a.y*b.z-a.z*b.y;
  c.y = -a.x*b.z+a.z*b.x;
  c.z = a.x*b.y-a.y*b.x;
  //平面方程 c和(x-D)的内积为0
  double r= CalculateDistance(c,Vector(0,0,0));
  c.x = c.x/r;
  c.y = c.y/r;
  c.z = c.z/r;

  double t0 = (x0.x*c.x-c.x*D.x+x0.y*c.y-c.y*D.y+x0.z*c.z-c.z*D.z)/(c.x*c.x+c.y*c.y+c.z*c.z);
  //投影的点减去t倍的法向量，落在平面内
  x0t.x = x0.x-t0*c.x;
  x0t.y = x0.y-t0*c.y;
  x0t.z = x0.z-t0*c.z;


 double t1 = (x1.x*c.x-c.x*D.x+x1.y*c.y-c.y*D.y+x1.z*c.z-c.z*D.z)/(c.x*c.x+c.y*c.y+c.z*c.z);
  //投影的点减去t倍的法向量，落在平面内
  x1t.x = x1.x-t1*c.x;
  x1t.y = x1.y-t1*c.y;
  x1t.z = x1.z-t1*c.z;

//计算逆时针夹角
  double const PI = 4*atan(1);
  Vector b0;//向量Cx0t,a是向量CD
  b0.x = x0t.x - C.x;
  b0.y = x0t.y - C.y;
  b0.z = x0t.z - C.z;
  //Cx0t与CD夹角的余弦值
  double cosvalue = (a.x*b0.x+a.y*b0.y+a.z*b0.z)/(CalculateDistance(a,Vector(0,0,0))*CalculateDistance(b0,Vector(0,0,0)));
  double angle = acos(cosvalue);
  angle *= (180/PI); //弧度转角度
//因为角度是0-2PI的,还要进行判断,利用之前构造平面的CD向量和垂直向量b是逆时针夹角大于PI,即(a,b,c)与(-b,a,0)逆时针夹角大于PI
  //因此，a与b0的外积方向若与c相同，则说明逆时针夹角大于PI，否则小于PI
  Vector c0;
  c0.x = a.y*b0.z-a.z*b0.y;
  c0.y = -a.x*b0.z+a.z*b0.x;
  c0.z = a.x*b0.y-a.y*b0.x;
  if (c.x*c0.x+c.y*c0.y+c.z*c0.z > 0)
  {
     angle = 360 - angle;
  }
  std::cout<<"CD(x,y,z):  ("<<a.x<<","<<a.y<<","<<a.z<<")"<<std::endl;
  std::cout<<"Cx0t(x,y,z):  ("<<b0.x<<","<<b0.y<<","<<b0.z<<")"<<std::endl;
  std::cout<<"x0t投影后点坐标(x,y,z):  ("<<x0t.x<<","<<x0t.y<<","<<x0t.z<<")"<<std::endl;
  std::cout<<"CD与Cx0t的逆时针角度(度): " << angle <<std::endl;

  Simulator::Stop (Seconds (5.0));

  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
