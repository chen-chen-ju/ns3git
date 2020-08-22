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
#include "ns3/gpsr-module.h"

#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("second");


int main (int argc, char *argv[])
{
  std::string phyMode = "DsssRate1Mbps"; 
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  //double deltaTime = 1;
  std::string traceFile = "ns2mobility15.tcl";

  CommandLine cmd;
  cmd.Parse (argc, argv);
  LogComponentEnable("second",LOG_LEVEL_INFO);

  NodeContainer node1;
  NodeContainer node2;
  node1.Create (200); 
  //node2.Create (100);

  NetDeviceContainer devices1;


  //Simulator::Schedule (Seconds (0), &showPosition, node1.Get(4),deltaTime);
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install ();

  //Simulator::Schedule (Seconds (0.0), &showPosition, n0, deltaTime);
 

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


  GpsrHelper gpsr;
  InternetStackHelper stack;
  //stack.SetRoutingHelper (aodv);
  stack.SetRoutingHelper (gpsr);
  stack.Install (node1);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces1 = address.Assign (devices1);

  int echoPort=9;

  UdpEchoClientHelper echoClient (interfaces1.GetAddress (69), echoPort);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (5));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientApps = echoClient.Install (node1.Get (116));
  clientApps.Start (Seconds (4.0));
  clientApps.Stop (Seconds (7.0));

  gpsr.Install();


     //Ptr<gpsr0::RoutingProtocol> gpsr0prot=gpsr0.Ipv4RoutingHelper::GetRouting<gpsr0::RoutingProtocol>(node1.Get(17)->GetObject<Ipv4>()->GetRoutingProtocol ()); 
     //gpsr0prot->Setmalicious(false);
    //double tt = gpsr0prot->m_neighbors.Gettrust("10.1.1.117",7);
//std::cout<< "t0" << tt <<std::endl;
  //Time t= Seconds(11.972);
  //Time t0= Seconds(int(t.GetSeconds()));
  //std::cout<< t0 <<std::endl;

  Simulator::Stop (Seconds (12.0));

  node2.Add(node1.Get(116));
  node2.Add(node1.Get(69));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.Install(node2);
  
  AnimationInterface anim("second.xml"); 
  for (int i=0;i<200;i++)
  {
     anim.UpdateNodeSize(i,20,20);
  } 
  anim.UpdateNodeColor(node2.Get(0),0,0,255);
  anim.UpdateNodeColor(node2.Get(1),0,0,255);
anim.UpdateNodeColor(node1.Get(122),0,255,0);
  //anim.EnablePacketMetadata (true);
  anim.SetMaxPktsPerTraceFile(99999999999999);
  Simulator::Run ();

 monitor->CheckForLostPackets();
 //monitor->SerializeToXmlFile("second.xml", true, true);

  //monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier=DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

uint32_t lost=(uint32_t)0;
Time delay= Time(NanoSeconds(0));
uint32_t Tx =(uint32_t)0;
uint32_t Rx =(uint32_t)0;

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin();i!=stats.end(); i++)
  {
	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
 /*
	//if ((t.sourceAddress==interfaces1.GetAddress (70) && t.destinationAddress == interfaces1.GetAddress (6)))
	//{
	  NS_LOG_INFO("Flow"<<i->first<< "(" << t.sourceAddress<< "->" <<t.destinationAddress<<")"<<std::endl);
	  NS_LOG_INFO("	 Time:  "<< i->second.timeFirstTxPacket<<std::endl);
	  NS_LOG_INFO("	 Time:  "<< i->second.timeFirstRxPacket<<std::endl);
	  //NS_LOG_INFO("	 Tx Bytes:  "<< i->second.txBytes<<std::endl);
	  NS_LOG_INFO("	 Tx Packets:  "<< i->second.txPackets<<std::endl);
	  NS_LOG_INFO("	 Rx Packets:  "<< i->second.rxPackets<<std::endl);
	  //NS_LOG_INFO("	 Rx Bytes:  "<< i->second.rxBytes<<std::endl);
	  NS_LOG_INFO("	 Lost Packets:  "<< i->second.lostPackets<<std::endl);
	  NS_LOG_INFO("	 Throughput:  "<< i->second.rxBytes*8.0/10.0/1024<<"  Kbps"<<std::endl);
          NS_LOG_INFO("	 delay:  "<< i->second.delaySum<<std::endl);
	//}
*/

if (t.sourceAddress==interfaces1.GetAddress ((uint32_t)116) && t.destinationAddress == interfaces1.GetAddress ((uint32_t)69))
{
NS_LOG_INFO("	 Tx Bytes:  "<< i->second.txBytes<<std::endl);
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

  Simulator::Destroy ();
  return 0;
}
