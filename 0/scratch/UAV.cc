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
NS_LOG_COMPONENT_DEFINE ("UAV");

void showPosition (Ptr<Node> node, double deltaTime)
{
  uint32_t nodeId = node->GetId ();
  Ptr<MobilityModel> mobModel = node->GetObject<MobilityModel> ();
  Vector3D pos = mobModel->GetPosition ();
  Vector3D speed = mobModel->GetVelocity ();
  std::cout << "At " << Simulator::Now ().GetSeconds () << " node " << nodeId
            << ": Position(" << pos.x << ", " << pos.y << ", " << pos.z
            << ");   Speed(" << speed.x << ", " << speed.y << ", " << speed.z
            << ")" << std::endl;

  Simulator::Schedule (Seconds (deltaTime), &showPosition, node, deltaTime);
}


int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  LogComponentEnable("UAV",LOG_LEVEL_INFO);

  RngSeedManager::SetSeed (1); // Changes seed from default of 1 to 3
  RngSeedManager::SetRun (1);  

  NodeContainer node1;
  node1.Create (5);
  NetDeviceContainer devices1;


  Ptr<UniformRandomVariable> randomizer0= CreateObject<UniformRandomVariable>();
  randomizer0->SetAttribute ("Min", DoubleValue (0.0));
  randomizer0->SetAttribute ("Max", DoubleValue (60.0));

   Ptr<UniformRandomVariable> randomizer1= CreateObject<UniformRandomVariable>();
  randomizer1->SetAttribute ("Min", DoubleValue (0.0));
  randomizer1->SetAttribute ("Max", DoubleValue (10000.0));

  Ptr<UniformRandomVariable> randomizer2= CreateObject<UniformRandomVariable>();
  randomizer2->SetAttribute ("Min", DoubleValue (0.0));
  randomizer2->SetAttribute ("Max", DoubleValue (10000.0));
  
  Ptr<UniformRandomVariable> randomizer3= CreateObject<UniformRandomVariable>();
  randomizer3->SetAttribute ("Min", DoubleValue (0.0));
  randomizer3->SetAttribute ("Max", DoubleValue (5000.0));

  MobilityHelper mobility;
  //double deltaTime=1;
/*
  for (uint32_t i=0; i<node1.GetN()-3; i++)
  {
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
	positionAlloc->Add(Vector(randomizer1->GetValue(),randomizer2->GetValue(),randomizer3->GetValue()));
  
	mobility.SetMobilityModel ("ns3::RandomWalk3dMobilityModel",
                         "Mode", StringValue ("Time"),
                         "Time", TimeValue (Seconds (0.1)),
                         "Speed", DoubleValue(randomizer0->GetValue()),
			 "SpeedChange", StringValue ("ns3::UniformRandomVariable[Min=-10.0|Max=10.0]"),
			 "Bounds", StringValue ("0|10000|0|10000|0|5000")
                         );
//			 "Bounds", StringValue ("0|10000|0|10000|0|5000")
        mobility.SetPositionAllocator(positionAlloc);
	mobility.Install(node1.Get(i));
  }
*/

  Ptr<ListPositionAllocator> positionAlloc1=CreateObject<ListPositionAllocator>();
  positionAlloc1->Add(Vector(8300,9400,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc1);
  mobility.Install(node1.Get(0));

 Ptr<ListPositionAllocator> positionAlloc2=CreateObject<ListPositionAllocator>();
  positionAlloc2->Add(Vector(8600,9400,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc2);
  mobility.Install(node1.Get(1));

 Ptr<ListPositionAllocator> positionAlloc3=CreateObject<ListPositionAllocator>();
  positionAlloc3->Add(Vector(9200,9400,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc3);
  mobility.Install(node1.Get(2));

 Ptr<ListPositionAllocator> positionAlloc4=CreateObject<ListPositionAllocator>();
  positionAlloc4->Add(Vector(8600,9200,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc4);
  mobility.Install(node1.Get(3));

Ptr<ListPositionAllocator> positionAlloc5=CreateObject<ListPositionAllocator>();
  positionAlloc5->Add(Vector(9020,9200,4900));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc5);
  mobility.Install(node1.Get(4));
/*
  for (uint32_t i=0; i<node1.GetN(); i++)
  {
	Simulator::Schedule (Seconds (0.0), &showPosition, node1.Get(i), deltaTime);
  }
*/

 Ssid ssid1 = Ssid ("ns-3-ssid1");
  WifiMacHelper wifiMac1;
  wifiMac1.SetType ("ns3::AdhocWifiMac","Ssid", SsidValue (ssid1));

  YansWifiPhyHelper wifiPhy1 = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel1;
  wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel1.AddPropagationLoss ("ns3::LOSandNLOSPropagationLossModel"); 
//LOSandNLOSPropagationLossModel   LogDistancePropagationLossModel
  wifiPhy1.SetErrorRateModel("ns3::YansErrorRateModel");
  wifiPhy1.Set ("Frequency", UintegerValue(5150));//中心频率5.15GHz
  wifiPhy1.Set ("TxPowerStart", DoubleValue(20));
  wifiPhy1.Set ("TxPowerEnd", DoubleValue(20));
  wifiPhy1.Set ("RxNoiseFigure", DoubleValue(0));
  wifiPhy1.Set ("EnergyDetectionThreshold", DoubleValue(-107.677));
  //信道带宽使用默认值20M Frequency
  //LogDistancePropagationLossModel

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

  UdpEchoClientHelper echoClient (interfaces1.GetAddress (2), echoPort);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientApps = echoClient.Install (node1.Get (1));
  clientApps.Start (Seconds (4.0));
  clientApps.Stop (Seconds (7.0));

  gpsr.Install();

  //Ptr<PropagationLossModel> a = node1.Get(0)->GetObject<PropagationLossModel>;
  
  Simulator::Stop (Seconds (10.0));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.Install(node1);

  AnimationInterface anim("UAV.xml");
  anim.EnablePacketMetadata (true);
  for (uint32_t i=0;i<node1.GetN();i++)
  {
     anim.UpdateNodeSize(i,20,20);
  } 

  Simulator::Run ();


 monitor->CheckForLostPackets();
 Ptr<Ipv4FlowClassifier> classifier=DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
 std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin();i!=stats.end(); i++)
  {
	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	//if (t.sourceAddress==interfaces1.GetAddress (3) )
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

   }

  Simulator::Destroy ();
  return 0;
}
