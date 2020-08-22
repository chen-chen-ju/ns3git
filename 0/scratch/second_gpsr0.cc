#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"


#include "ns3/command-line.h"
#include "ns3/string.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-phy.h"

#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-helper.h"
#include "ns3/wifi-net-device.h"
#include <ns3/log.h>
#include "ns3/applications-module.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-phy.h"
#include "ns3/gpsr0-module.h"

#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("second_gpsr0");


int main (int argc, char *argv[])
{
  std::string phyMode = "DsssRate1Mbps"; 
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  //double deltaTime = 1;
  std::string traceFile = "ns2mobility200.tcl";

  CommandLine cmd;
  cmd.Parse (argc, argv);
  LogComponentEnable("second_gpsr0",LOG_LEVEL_INFO);

  NodeContainer node1;
  node1.Create (200);
  NetDeviceContainer devices1;


  //Simulator::Schedule (Seconds (0), &showPosition, node1.Get(4),deltaTime);
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install ();

  //Simulator::Schedule (Seconds (0.0), &showPosition, n0, deltaTime);
  NodeContainer node2;
  NodeContainer node_mali;
  int mal_num=20;
  int source_num=20;
  int All_num=2*source_num+mal_num;
 
  //node2.Create (100);
//----随机数----
int flag[200]= {0};
int select[All_num];
int count=0;
RngSeedManager::SetSeed (5); // Changes seed from default of 1 to 3
RngSeedManager::SetRun (1);
Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
x->SetAttribute ("Min", DoubleValue (0.0));
x->SetAttribute ("Max", DoubleValue (99.0));
while (count<All_num)
{
   int tyloo = (int) x->GetInteger ();
   if (flag[tyloo]!=1)
   {
      select[count]=tyloo;
      flag[tyloo]=1;
      count++;
   }
}

for (int i=0; i<source_num; i++)
{
   node2.Add(node1.Get((uint32_t)select[2*i]));
   node2.Add(node1.Get((uint32_t)select[2*i+1]));
}



for (int i=0; i<mal_num; i++)
{
   node_mali.Add(node1.Get((uint32_t)select[i+2*source_num]));
}

//----结束----

  Ssid ssid1 = Ssid ("ns-3-ssid1");
  WifiMacHelper wifiMac1;
  wifiMac1.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid1));

  YansWifiPhyHelper wifiPhy1 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel1;
  wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel1.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(250)); 
  wifiPhy1.SetErrorRateModel("ns3::YansErrorRateModel");
/*
  wifiPhy1.Set ("TxPowerStart", DoubleValue(16.0206));
  wifiPhy1.Set ("TxPowerEnd", DoubleValue(16.0206));
  wifiPhy1.Set ("TxGain", DoubleValue(0));
  wifiPhy1.Set ("RxGain", DoubleValue (0));

  wifiPhy1.Set ("EnergyDetectionThreshold", DoubleValue(-71.9842));
  wifiPhy1.Set ("CcaMode1Threshold", DoubleValue(-74.9842));
*/
  wifiPhy1.SetChannel (wifiChannel1.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager"); 
  
  devices1 = wifi.Install (wifiPhy1, wifiMac1, node1);


  Gpsr0Helper gpsr0;
  InternetStackHelper stack;
  //stack.SetRoutingHelper (aodv);
  stack.SetRoutingHelper (gpsr0);
  stack.Install (node1);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces1 = address.Assign (devices1);

  int echoPort=9;

  for (int i=0; i<source_num; i++)
 {
  echoPort=echoPort+i;	
  UdpEchoClientHelper echoClient (interfaces1.GetAddress ((uint32_t)select[2*i]), echoPort);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (2000));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientApps = echoClient.Install (node1.Get((uint32_t)select[2*i+1]));
  clientApps.Start (Seconds (50.0));
  clientApps.Stop (Seconds (100.0));
 }
  gpsr0.Install();

  for (uint32_t u = 0; u < node_mali.GetN (); ++u)
  {
     Ptr<Node> ue = node_mali.Get (u);
     Ptr<gpsr0::RoutingProtocol> gpsr0prot=gpsr0.Ipv4RoutingHelper::GetRouting<gpsr0::RoutingProtocol>(ue->GetObject<Ipv4>()->GetRoutingProtocol ());
     gpsr0prot->Setmalicious(true);
  }

  Simulator::Stop (Seconds (105.0));

  //node2.Add(node1.Get(0));
  //node2.Add(node1.Get(73));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.Install(node2);
  
  AnimationInterface anim("second.xml"); 
  for (int i=0;i<200;i++)
  {
     anim.UpdateNodeSize(i,20,20);
  } 
  for (int i=0; i<source_num; i++)
  {
	  anim.UpdateNodeColor(node2.Get(2*i),10*i,10*i,255-10*i);
	  anim.UpdateNodeColor(node2.Get(2*i+1),10*i,10*i,255-10*i);
  }

  for (int i=0; i<mal_num; i++)
  {anim.UpdateNodeColor(node_mali.Get(i),0,255,0);}
  //anim.EnablePacketMetadata (true);
  anim.SetMaxPktsPerTraceFile(99999999999999);
  Simulator::Run ();

  monitor->CheckForLostPackets();
  monitor->SerializeToXmlFile("second0_gpsr0.xml", true, true);

  Ptr<Ipv4FlowClassifier> classifier=DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();


  std::vector<uint32_t> lost (source_num ,(uint32_t)0);
  std::vector<Time> delay (source_num ,Time(NanoSeconds(0)));
  std::vector<uint32_t> Tx (source_num ,(uint32_t)0);
  std::vector<uint32_t> Rx (source_num ,(uint32_t)0);
  
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin();i!=stats.end(); i++)
  {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      for (int j=0; j<source_num; j++)
      {
          if (t.sourceAddress==interfaces1.GetAddress ((uint32_t)select[2*j+1]) && t.destinationAddress == interfaces1.GetAddress ((uint32_t)select[2*j]))
          {
	     lost.at(j)=lost.at(j)+i->second.lostPackets;
	     delay.at(j)=delay.at(j)+i->second.delaySum;
	     Tx.at(j)=Tx.at(j)+i->second.txPackets;
	     Rx.at(j)=Rx.at(j)+i->second.rxPackets;
	     break;	     
          }
      } 
	
  }
  for (int j=0; j<source_num; j++)
  {
      NS_LOG_INFO("flag:  "<< j <<std::endl);
      NS_LOG_INFO("Lost Packets:  "<< lost.at(j) <<std::endl);
      NS_LOG_INFO("delay:  "<< delay.at(j) <<std::endl);
      NS_LOG_INFO("Tx Packets:  "<< Tx.at(j) <<std::endl);
      NS_LOG_INFO("Rx Packets:  "<< Rx.at(j) <<std::endl);
  }

/*
uint32_t lost=(uint32_t)0;
Time delay= Time(NanoSeconds(0));
uint32_t Tx =(uint32_t)0;
uint32_t Rx =(uint32_t)0;

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin();i!=stats.end(); i++)
  {
	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
if (t.sourceAddress==interfaces1.GetAddress ((uint32_t)select[1]) && t.destinationAddress == interfaces1.GetAddress ((uint32_t)select[0]))
{
   	     lost=lost+i->second.lostPackets;
	     delay=delay+i->second.delaySum;
	     Tx=Tx+i->second.txPackets;
	     Rx=Rx+i->second.rxPackets;
}
  }


      NS_LOG_INFO("Lost Packets:  "<< lost <<std::endl);
      NS_LOG_INFO("delay:  "<< delay <<std::endl);
      NS_LOG_INFO("Tx Packets:  "<< Tx <<std::endl);
      NS_LOG_INFO("Rx Packets:  "<< Rx <<std::endl);

*/
  Simulator::Destroy ();
  return 0;
}
