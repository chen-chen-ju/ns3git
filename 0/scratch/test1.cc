#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/mobility-helper.h"

#include "ns3/ssid.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("test1");

int 
main (int argc, char *argv[])
{
  std::string phyMode ("OfdmRate6Mbps");

  CommandLine cmd;

  cmd.Parse (argc,argv);


  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  NodeContainer csmaNodes;
  csmaNodes.Create (3);

 MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (10.0, 5.0, 0.0));
  positionAlloc->Add (Vector (5.0, 5.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (csmaNodes);

Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
  positionAlloc1->Add (Vector (15.0, 0.0, 0.0));
  positionAlloc1->Add (Vector (10.0, 10.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc1);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (p2pNodes);

csmaNodes.Add (p2pNodes.Get (1));

  Ssid ssid2 = Ssid ("ns-3-ssid2");
  WifiMacHelper wifiMac2;
  wifiMac2.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid2));

  YansWifiPhyHelper wifiPhy2 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel2;
  wifiChannel2.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel2.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy2.SetChannel (wifiChannel2.Create ());

  WifiHelper wifi2;
  wifi2.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi2.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "ControlMode",StringValue (phyMode));
  
  NetDeviceContainer devices2;
  devices2 = wifi2.Install (wifiPhy2, wifiMac2, p2pNodes);

  Ssid ssid1 = Ssid ("ns-3-ssid1");

  WifiMacHelper wifiMac1;
  wifiMac1.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid1));

  YansWifiPhyHelper wifiPhy1 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel1;
  wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel1.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy1.SetChannel (wifiChannel1.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "ControlMode",StringValue (phyMode));

  NetDeviceContainer devices1;
  devices1 = wifi.Install (wifiPhy1, wifiMac1, csmaNodes);

  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (devices2);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (devices1);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  
AnimationInterface anim("test1.xml");
anim.EnablePacketMetadata(true);

  Simulator::Run ();


monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier=DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin();i!=stats.end(); i++)
  {
	//if(i->first)
	//{
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
	  std::cout<<"Flow"<<i->first<< "(" << t.sourceAddress<< "->" <<t.destinationAddress<<")"<<std::endl;
	  std::cout<<"	 Time:  "<< i->second.timeFirstRxPacket<<std::endl;
	  std::cout<<"	 Tx Bytes:  "<< i->second.txBytes<<std::endl;
	  std::cout<<"	 Tx Packets:  "<< i->second.txPackets<<std::endl;
	  std::cout<<"	 Rx Bytes:  "<< i->second.rxBytes<<std::endl;
	  std::cout<<"	 Throughput:  "<< i->second.rxBytes*8.0/10.0/1024/1024<<"  Mbps"<<std::endl;
	//}
  }


  Simulator::Destroy ();
  return 0;
}
