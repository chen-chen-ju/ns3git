#include "ns3/core-module.h"
#include "ns3/command-line.h"
#include "ns3/string.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-phy.h"

#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-net-device.h"
#include <ns3/log.h>
#include "ns3/applications-module.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-phy.h"
#include "ns3/aodv-helper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-helper.h"
//#include "ns3/dsdv-routing-protocol.h"

#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"

#include "ns3/constant-acceleration-mobility-model.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("aodv_ess");


void Setmobility(Vector velocity,Vector acceleration,NodeContainer node)
{
    for (uint32_t i=0;i<node.GetN();i++)
{
  velocity.y=velocity.y+3*i;
  acceleration.y=acceleration.y-0.1*i;
  Ptr<ConstantAccelerationMobilityModel> VelocityModel=node.Get(i)->    GetObject<ConstantAccelerationMobilityModel>();
  VelocityModel->SetVelocityAndAcceleration(velocity,acceleration);
}

}



int
main (int argc, char *argv[])
{
  //uint32_t rng=3;
  CommandLine cmd;
  cmd.Parse (argc, argv);
  LogComponentEnable("aodv_ess",LOG_LEVEL_INFO);
//LogComponentEnable("DsdvRoutingProtocol",LOG_LEVEL_INFO);
  std::string phyMode ("OfdmRate6Mbps");

  
  //Time::SetResolution (Time::NS);
  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  int Numnode1=6;
  int Numnode2=5;
  NodeContainer node1;
  NodeContainer node2;
  node1.Create (Numnode1);
  node2.Create (Numnode2);

  NodeContainer main_node;
  NodeContainer node_all;
  main_node.Add(node1.Get(0));
  main_node.Add(node1.Get(Numnode1-1));
  main_node.Add(node2.Get(0));
  node_all.Add(node1);
  node_all.Add(node2);
  
  NetDeviceContainer devices1;
  NetDeviceContainer devices2;

MobilityHelper mobility;
 Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint32_t i=0;i<node1.GetN();i++)
{
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
}
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantAccelerationMobilityModel");
  mobility.Install (node1);

  Setmobility(Vector(10,30,0),Vector(0,-1,0),node1);



  for (uint32_t i=0;i<node2.GetN();i++)
{
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
}
  mobility.SetMobilityModel ("ns3::ConstantAccelerationMobilityModel");
  mobility.Install (node2);
  
  Simulator::Schedule(Seconds(20), &(Setmobility), Vector(10,30,0), Vector(0,-1,0),node2);




node2.Add(node1.Get(Numnode1-1));

  Ssid ssid1 = Ssid ("ns-3-ssid1");
  Ssid ssid2 = Ssid ("ns-3-ssid2");  

  WifiMacHelper wifiMac1;
  wifiMac1.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid1));

  YansWifiPhyHelper wifiPhy1 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel1;
  wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel1.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(200)); 
  wifiPhy1.SetErrorRateModel("ns3::YansErrorRateModel");
  wifiPhy1.SetChannel (wifiChannel1.Create ());


  WifiMacHelper wifiMac2;
  wifiMac2.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid2));

  YansWifiPhyHelper wifiPhy2 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel2;
  wifiChannel2.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel2.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(200)); 
  wifiPhy2.SetErrorRateModel("ns3::YansErrorRateModel");
  wifiPhy2.SetChannel (wifiChannel2.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "ControlMode",StringValue (phyMode));

  devices2 = wifi.Install (wifiPhy2, wifiMac2, node2);
  devices1 = wifi.Install (wifiPhy1, wifiMac1, node1);
  
//PointToPointHelper pointToPoint;
  //devices2 = pointToPoint.Install (node2);

  AodvHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv);
  stack.Install (node_all);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces1 = address.Assign (devices1);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces2 = address.Assign (devices2);

 

  int echoPort=9;

  UdpEchoServerHelper echoServer (echoPort);
  ApplicationContainer serverApps = echoServer.Install (node2.Get (0));

  UdpEchoClientHelper echoClient (interfaces2.GetAddress (0), echoPort);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (2.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (node1.Get (0));
  clientApps.Start (Seconds (0.0));
  clientApps.Stop (Seconds (60.0));



Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv_ess.routes", std::ios::out);

aodv.PrintRoutingTableAllAt (Seconds (0.230), routingStream);


  stack.EnablePcapIpv4("aodv", main_node);

  
  //Ptr<dsdv::RoutingProtocol> dsdvprot=dsdv.Ipv4RoutingHelper::GetRouting<dsdv::RoutingProtocol>(node1.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol ());
//
  //node1.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ()
  //dsdvprot->PrintRoutingTable(routingStream1,Time::Unit(4));

  Simulator::Stop (Seconds (60.00));

FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.Install(main_node);
  
  AnimationInterface anim("aodv_ess.xml");
anim.SetMaxPktsPerTraceFile(99999999999999);
anim.UpdateNodeColor(node1.Get(0),10,10,255);
anim.UpdateNodeColor(node2.Get(0),10,255,0);
anim.UpdateNodeSize(5,20,20);
anim.UpdateNodeSize(0,20,20);
anim.UpdateNodeSize(6,20,20);

//anim.EnablePacketMetadata(true);
  //anim.AddSourceDestination(0,"10.1.2.1");
  //anim.TrackIpv4RoutePaths ();
  //anim.EnableIpv4RouteTracking ("dsdv2.route", Seconds(0.0), Seconds(60.0), node1, Seconds(10)); 
  

  Simulator::Run ();




  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier=DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin();i!=stats.end(); i++)
  {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
	  NS_LOG_INFO("Flow"<<i->first<< "(" << t.sourceAddress<< "->" <<t.destinationAddress<<")"<<std::endl);
	  NS_LOG_INFO("	 Time:  "<< i->second.timeLastRxPacket<<std::endl);
	  NS_LOG_INFO("	 Tx Bytes:  "<< i->second.txBytes<<std::endl);
	  NS_LOG_INFO("	 Tx Packets:  "<< i->second.txPackets<<std::endl);
	  NS_LOG_INFO("	 Rx Bytes:  "<< i->second.rxBytes<<std::endl);
	  NS_LOG_INFO("	 Lost Packets:  "<< i->second.lostPackets<<std::endl);
	  NS_LOG_INFO("	 Throughput:  "<< i->second.rxBytes*8.0/10.0/1024/1024<<"  Mbps"<<std::endl);
  }


  Simulator::Destroy ();
  return 0;
}

