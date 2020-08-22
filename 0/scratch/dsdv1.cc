
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
#include "ns3/dsdv-helper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/dsdv-routing-protocol.h"
#include "ns3/ipv4-global-routing-helper.h"


#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
//#include "ns3/packet-socket-address.h"
#include "ns3/ipv4-address-helper.h"



//#include <fstream>
//#include "ns3/stats-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("dsdv1example");


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
  CommandLine cmd;
  cmd.Parse (argc, argv);
  //LogComponentEnable("dsdv1example",LOG_LEVEL_INFO);
LogComponentEnable("Ipv4L3Protocol",LOG_LEVEL_INFO);
  std::string phyMode ("OfdmRate6Mbps");
  

  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

//Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  NodeContainer node1;
  NodeContainer node2;
  node1.Create (3);
  node2.Create (1);
  
  NetDeviceContainer devices1;
  NetDeviceContainer devices2;

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (10.0, 5.0, 0.0));
  positionAlloc->Add (Vector (5.0, 5.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (node1);

Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
  positionAlloc1->Add (Vector (15.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc1);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (node2);

  node2.Add(node1.Get(1));
  
  Ssid ssid1 = Ssid ("ns-3-ssid1");
  Ssid ssid2 = Ssid ("ns-3-ssid2");  

  WifiMacHelper wifiMac1;
  wifiMac1.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid1));

  YansWifiPhyHelper wifiPhy1 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel1;
  wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel1.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy1.SetChannel (wifiChannel1.Create ());

  WifiHelper wifi1;
  wifi1.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi1.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "ControlMode",StringValue (phyMode));

  devices1 = wifi1.Install (wifiPhy1, wifiMac1, node1);

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
  
  devices2 = wifi2.Install (wifiPhy2, wifiMac2, node2);

//PointToPointHelper pointToPoint;

  //devices2 = pointToPoint.Install (node2);





  DsdvHelper dsdv;
  dsdv.Set ("PeriodicUpdateInterval", TimeValue (Seconds (6.00)));
  dsdv.Set ("SettlingTime", TimeValue (Seconds (6.00)));

  InternetStackHelper stack;

  stack.SetRoutingHelper (dsdv);
  stack.Install (node1);
  stack.Install (node2.Get(0));
  
  Ipv4AddressHelper address;
  

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces2 = address.Assign (devices2);

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces1 = address.Assign (devices1);

   
/*
  ApplicationContainer cbrApps;
  uint16_t cbrPort = 9;
  OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (interfaces2.GetAddress(1), cbrPort));
  onOffHelper.SetAttribute ("PacketSize", UintegerValue (1024));
  onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  onOffHelper.SetAttribute ("DataRate", StringValue ("3000000bps"));
  onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.000000)));
  cbrApps.Add (onOffHelper.Install (node1.Get (0)));
*/


  uint16_t  echoPort = 9;


  //UdpEchoServerHelper echoServer (echoPort);
  //ApplicationContainer serverApps = echoServer.Install (node1.Get (0));

  UdpEchoClientHelper echoClientHelper (interfaces1.GetAddress(1,0), echoPort);
  echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (100));
  echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (2)));
  echoClientHelper.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer pingApps;

  echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.000)));
  pingApps.Add (echoClientHelper.Install (node2.Get (0)));

  //PrintIPAddress(node1.Get(1));  
  //PrintIPAddress(node2.Get(1));
/*
  PacketSocketAddress socket;
  socket.SetSingleDevice (devices1.Get (0)->GetIfIndex ());
  socket.SetPhysicalAddress (devices2.Get (0)->GetAddress ());
  socket.SetProtocol (1);

   OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
   onoff.SetConstantRate (DataRate (60000000));
   onoff.SetAttribute ("PacketSize", UintegerValue (1000));
 
   ApplicationContainer apps = onoff.Install (node1.Get (0));
   apps.Start (Seconds (1.0));
   apps.Stop (Seconds (60.0));
   Ptr<Socket> recvSink = SetupPacketReceive (node2.Get (0));
*/

//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //std::string probeType;
  //std::string tracePath;
  //probeType = "ns3::Ipv4PacketProbe";
  //tracePath = "/NodeList/*/$ns3::Ipv4L3Protocol/Tx";
   
/*
FileHelper fileHelper;

  fileHelper.ConfigureFile ("dsdv1-packet-byte-count",
                            FileAggregator::FORMATTED);
  fileHelper.Set2dFormat ("Time (Seconds) = %.3e\tPacket Byte Count = %.0f");

  fileHelper.WriteProbe (probeType,
                         tracePath,
                         "OutputBytes");
*/

Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll();



  //PrintIPAddress(node2.Get(1));  

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dsdv1.routes", std::ios::out);

  dsdv.PrintRoutingTableAllAt (Seconds (30.00), routingStream);

  wifiPhy2.EnablePcapAll  ("first");
  //wifiPhy2.EnablePcap ("first", devices2.Get(1), true);
  
  
  
  AnimationInterface anim("dsdv1.xml");
anim.EnablePacketMetadata(true);
  //anim.AddSourceDestination(0,"10.1.2.1");
  //anim.TrackIpv4RoutePaths ();
  //anim.EnableIpv4RouteTracking ("dsdv2.route", Seconds(0.0), Seconds(60.0), node1, Seconds(10)); 


  Simulator::Stop (Seconds (60.00));
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

