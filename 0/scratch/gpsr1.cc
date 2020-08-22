#include "ns3/core-module.h"
#include "ns3/command-line.h"
#include "ns3/string.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-phy.h"

#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-net-device.h"
#include <ns3/log.h>
#include "ns3/applications-module.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-phy.h"
#include "ns3/aodv-helper.h"

//#include "ns3/dsdv-routing-protocol.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"

#include "ns3/gpsr0-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("aodv1");



int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  LogComponentEnable("aodv1",LOG_LEVEL_INFO);
//LogComponentEnable("DsdvRoutingProtocol",LOG_LEVEL_INFO);
  //std::string phyMode ("OfdmRate6Mbps");
  std::string phyMode = "DsssRate1Mbps"; 
  //Time::SetResolution (Time::NS);


  // Set non-unicast data rate to be the same as that of unicast
Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  int Numnode1=4;
  NodeContainer node1;
  node1.Create (Numnode1); 

  NetDeviceContainer devices1;
  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc1=CreateObject<ListPositionAllocator>();
  positionAlloc1->Add(Vector(1000,1000,0));
  //positionAlloc1->Add(Vector(1100,1190,20));
  positionAlloc1->Add(Vector(1100,1210,0));
  positionAlloc1->Add(Vector(1000,1400,0));
  //positionAlloc1->Add(Vector(900,1191,0));
  positionAlloc1->Add(Vector(900,1200,00));
  mobility.SetPositionAllocator(positionAlloc1);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(node1);

/*
  Ptr<ListPositionAllocator> positionAlloc2=CreateObject<ListPositionAllocator>();
  positionAlloc2->Add(Vector(500,1000,0));
  mobility.SetPositionAllocator(positionAlloc2);
  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  mobility.Install(node1.Get(1));
  Ptr<ConstantVelocityMobilityModel> VelocityModel=node1.Get(1)-> GetObject<ConstantVelocityMobilityModel>();
  VelocityModel-> SetPosition(Vector(500,1000,0));
  VelocityModel->SetVelocity(Vector(100,0.0,0.0));
*/

/*
  Ptr<ListPositionAllocator> positionAlloc3=CreateObject<ListPositionAllocator>();
  positionAlloc3->Add(Vector(6,15,0));
  mobility.SetPositionAllocator(positionAlloc3);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(node1.Get(2));
*/

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
 

/*
  Ptr<MobilityModel> mobModel = node1.Get(1)->GetObject<MobilityModel>();
  Vector3D pos = mobModel->GetPosition ();
  std::cout << " Position(" << pos.x << ", " << pos.y << ", " << pos.z
            << "); " << std::endl;
*/

 
  int echoPort=9;

  UdpEchoServerHelper echoServer (echoPort);
  ApplicationContainer serverApps = echoServer.Install (node1.Get (0));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (20.0));


  UdpEchoClientHelper echoClient (interfaces1.GetAddress (0), echoPort);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (3));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientApps = echoClient.Install (node1.Get (2));
  clientApps.Start (Seconds (6.0));
  clientApps.Stop (Seconds (20.0));

  gpsr0.Install();


  //Ptr<gpsr0::RoutingProtocol> gpsr0prot=gpsr0.Ipv4RoutingHelper::GetRouting<gpsr0::RoutingProtocol>(node1.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol ());
  
  //gpsr0prot->Setmalicious(true);
  Simulator::Stop (Seconds (20.00));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.Install(node1);

  AnimationInterface anim("aodv1.xml");
  anim.EnablePacketMetadata (true);

  Simulator::Run ();


    monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier=DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin();i!=stats.end(); i++)
  {

	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	//if (t.sourceAddress=="10.1.1.1" )
	//{
	  NS_LOG_INFO("Flow"<<i->first<< "(" << t.sourceAddress<< "->" <<t.destinationAddress<<")"<<std::endl);
	  NS_LOG_INFO("	 Time:  "<< i->second.timeFirstTxPacket<<std::endl);
	  NS_LOG_INFO("	 Time:  "<< i->second.timeFirstRxPacket<<std::endl);
	  NS_LOG_INFO("	 Tx Bytes:  "<< i->second.txBytes<<std::endl);
	  NS_LOG_INFO("	 Tx Packets:  "<< i->second.txPackets<<std::endl);
	  NS_LOG_INFO("	 Rx Bytes:  "<< i->second.rxBytes<<std::endl);
	  NS_LOG_INFO("	 Lost Packets:  "<< i->second.lostPackets<<std::endl);
	  NS_LOG_INFO("	 Throughput:  "<< i->second.rxBytes*8.0/10.0/1024/1024<<"  Mbps"<<std::endl);
	//}
  }

  Simulator::Destroy ();
  return 0;
}

