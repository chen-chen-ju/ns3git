
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

#include "ns3/rng-seed-manager.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("aodv");


void PrintIPAddress(Ptr<Node> node)
{
    Ptr<Ipv4> ipv4;
    Ipv4Address ipv4Address;

    ipv4 = node->GetObject<Ipv4>();

    for (uint32_t i_interface = 0; i_interface < ipv4->GetNInterfaces(); i_interface++) {
      for (uint32_t count = 0; count < ipv4->GetNAddresses(i_interface); count++) {
         Ipv4InterfaceAddress iaddr = ipv4->GetAddress(i_interface, count);
         ipv4Address = iaddr.GetLocal();

         //each node has a local ip address: 127.0.0.1
         if (ipv4Address != Ipv4Address("127.0.0.1")) {
            std::ostringstream addrOss;
            ipv4Address.Print(addrOss);
            //std::cout << 
	    NS_LOG_INFO(i_interface << ": " << addrOss.str().c_str() << std::endl);
         }
      }
    }
}



int
main (int argc, char *argv[])
{
  uint32_t rng=3;
  CommandLine cmd;
  cmd.Parse (argc, argv);
  LogComponentEnable("aodv",LOG_LEVEL_INFO);
//LogComponentEnable("DsdvRoutingProtocol",LOG_LEVEL_INFO);
  std::string phyMode ("OfdmRate6Mbps");


RngSeedManager::SetSeed (1); // Changes seed from default of 1 to 3
  RngSeedManager::SetRun (rng);
  
  //Time::SetResolution (Time::NS);
  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  int Numnode1=6;
  int Numnode2=6;
  NodeContainer node1;
  NodeContainer node2;
  node1.Create (Numnode1);
  node2.Create (Numnode2);

  NodeContainer main_node;
  NodeContainer node_all;
  
  main_node.Create(1);
  main_node.Add(node1.Get(0));
  main_node.Add(node2.Get(0));
  node_all.Add(node1);
  node_all.Add(node2);
  node_all.Add(main_node.Get(0));
  
  NetDeviceContainer devices1;
  NetDeviceContainer devices2;
  NetDeviceContainer devices3;

MobilityHelper mobility;
Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
				"X",StringValue("40"),
				"Y",StringValue("40"),
				"Rho",StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=40]"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (node1);

  mobility.Install (main_node.Get(0));


  mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
				"X",StringValue("100"),
				"Y",StringValue("100"),
				"Rho",StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=30]"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (node2);

//node2.Add(node1.Get(Numnode1-1));

  Ssid ssid1 = Ssid ("ns-3-ssid1");
  Ssid ssid2 = Ssid ("ns-3-ssid2");  

  WifiMacHelper wifiMac1;
  wifiMac1.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid1));

  YansWifiPhyHelper wifiPhy1 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel1;
  wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel1.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(20)); 
  wifiPhy1.SetErrorRateModel("ns3::YansErrorRateModel");
  wifiPhy1.SetChannel (wifiChannel1.Create ());


  WifiMacHelper wifiMac2;
  wifiMac2.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid2));

  YansWifiPhyHelper wifiPhy2 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel2;
  wifiChannel2.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel2.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(30)); 
  wifiPhy2.SetErrorRateModel("ns3::YansErrorRateModel");
  wifiPhy2.SetChannel (wifiChannel2.Create ());


  YansWifiPhyHelper wifiPhy3 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel3;
  wifiChannel3.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel3.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(200)); 
  wifiPhy3.SetErrorRateModel("ns3::YansErrorRateModel");
  wifiPhy3.SetChannel (wifiChannel3.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "ControlMode",StringValue (phyMode));

  devices2 = wifi.Install (wifiPhy2, wifiMac2, node2);
  devices2.Add(wifi.Install (wifiPhy3, wifiMac2, main_node.Get(0)));
  devices1 = wifi.Install (wifiPhy1, wifiMac1, node1);
  devices1.Add(wifi.Install (wifiPhy3, wifiMac1, main_node.Get(0)));
  
//PointToPointHelper pointToPoint;
  //devices2 = pointToPoint.Install (node2);

  AodvHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv);
  stack.Install (node_all);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces1 = address.Assign (devices1);
  //interfaces1.Add(address.Assign (devices3));
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces2 = address.Assign (devices2);
  //interfaces2.Add(address.Assign (devices3));
 

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



Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);

aodv.PrintRoutingTableAllAt (Seconds (30.00), routingStream);
PrintIPAddress(main_node.Get(0));


  //stack.EnablePcapIpv4All("aodv");

  
  //Ptr<dsdv::RoutingProtocol> dsdvprot=dsdv.Ipv4RoutingHelper::GetRouting<dsdv::RoutingProtocol>(node1.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol ());
//
  //node1.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ()
  //dsdvprot->PrintRoutingTable(routingStream1,Time::Unit(4));

  Simulator::Stop (Seconds (60.00));

FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.Install(main_node);
  
  AnimationInterface anim("aodv.xml");
anim.SetMaxPktsPerTraceFile(99999999999999);
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
	  NS_LOG_INFO("	 Time:  "<< i->second.timeFirstRxPacket<<std::endl);
	  NS_LOG_INFO("	 Tx Bytes:  "<< i->second.txBytes<<std::endl);
	  NS_LOG_INFO("	 Tx Packets:  "<< i->second.txPackets<<std::endl);
	  NS_LOG_INFO("	 Rx Bytes:  "<< i->second.rxBytes<<std::endl);
	  NS_LOG_INFO("	 Lost Packets:  "<< i->second.lostPackets<<std::endl);
	  NS_LOG_INFO("	 Throughput:  "<< i->second.rxBytes*8.0/10.0/1024/1024<<"  Mbps"<<std::endl);
  }


  Simulator::Destroy ();
  return 0;
}

