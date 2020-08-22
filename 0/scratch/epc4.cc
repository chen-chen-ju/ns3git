#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include"ns3/internet-module.h"
#include"ns3/point-to-point-module.h"
#include"ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include <ns3/buildings-helper.h>
//#include "ns3/gtk-config-store.h"
 
using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("epc4");


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



int main (int argc, char *argv[])
{   
  //double interPacketInterval=100;
  uint16_t numBearersPerUe = 1;
  CommandLine cmd;
  cmd.Parse (argc, argv);
 
  LogComponentEnable("epc4",LOG_LEVEL_INFO);

  Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (100)));
  Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
  //ConfigStore inputConfig;
  //inputConfig.ConfigureDefaults ();

  // Parse again so you can override default values from the command line
  //cmd.Parse (argc, argv);
 
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm"); // disable automatic handover
  // Uncomment to enable logging
//  lteHelper->EnableLogComponents ();
 
  Ptr<Node> pgw = epcHelper->GetPgwNode (); 
// 创建一个远程主机
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1); 
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);
// 创建 internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s"))); 
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500)); 
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010))); 
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost); 
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
// 接口 0 为本地主机，1 为 p2p 设备
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ()); 
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (2);
  ueNodes.Create (3);


  // Install Mobility Model
  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc=CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(30,30,0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(pgw);
  BuildingsHelper::Install (pgw);

  Ptr<ListPositionAllocator> positionAlloc0=CreateObject<ListPositionAllocator>();
  positionAlloc0->Add(Vector(40,40,0));
  mobility.SetPositionAllocator(positionAlloc0);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(remoteHostContainer);
  BuildingsHelper::Install (remoteHostContainer);
 
  Ptr<ListPositionAllocator> positionAlloc1 =
  CreateObject<ListPositionAllocator>();
  positionAlloc1->Add (Vector (20.0, 30.0, 0.0));
  positionAlloc1->Add (Vector (30.0, 20.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc1);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  Ptr<ListPositionAllocator> positionAlloc2=CreateObject<ListPositionAllocator>();
  positionAlloc2->Add(Vector(20,10,0));
  mobility.SetPositionAllocator(positionAlloc2);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes.Get(0));

   Ptr<ListPositionAllocator> positionAlloc3=CreateObject<ListPositionAllocator>();
  positionAlloc3->Add(Vector(10,20,0));
  mobility.SetPositionAllocator(positionAlloc3);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes.Get(1));

   Ptr<ListPositionAllocator> positionAlloc4=CreateObject<ListPositionAllocator>();
  positionAlloc4->Add(Vector(10,10,0));
  mobility.SetPositionAllocator(positionAlloc4);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes.Get(2));

  BuildingsHelper::Install (ueNodes);
 
  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  // Default scheduler is PF, uncomment to use RR
  //lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
 
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);
 
  // 在用户上安装 IP 协议栈
  InternetStackHelper internet0;
  internet0.Install (ueNodes);
  // 给用户分配 IP 地址
  
  Ipv4InterfaceContainer ueIpIface;

  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
 
/*
  Ipv4InterfaceContainer ueIpIface;
for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ue = ueNodes.Get (u);
    Ptr<NetDevice> ueLteDevice = ueDevs.Get (u);
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevice));
    // 为用户设置默认网关
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }
*/
// Attach a UE to a eNB
  lteHelper->Attach (ueDevs.Get(0), enbDevs.Get (1));
  lteHelper->Attach (ueDevs.Get(1), enbDevs.Get (0));
  lteHelper->Attach (ueDevs.Get(2), enbDevs.Get (1));

  // Activate a data radio bearer
/*
Ptr<EpcTft> tft = Create<EpcTft> ();
EpcTft::PacketFilter pf;
pf.localPortStart = 1234;
pf.localPortEnd = 1234;
tft->Add (pf);
lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);
*/
uint16_t dlPort = 1000;
uint16_t ulPort = 2000;

for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      for (uint32_t b = 0; b < numBearersPerUe; ++b)
        {
          ++dlPort;
          ++ulPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

          //NS_LOG_LOGIC ("installing UDP DL app for UE " << u);
          UdpClientHelper dlClientHelper (ueIpIface.GetAddress (u), dlPort);
          clientApps.Add (dlClientHelper.Install (remoteHost));
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ue));

          //NS_LOG_LOGIC ("installing UDP UL app for UE " << u);
          UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
          clientApps.Add (ulClientHelper.Install (ue));
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);
          EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          lteHelper->ActivateDedicatedEpsBearer (ueDevs.Get (u), bearer, tft);

          
          serverApps.Start (Seconds (0.00));
          clientApps.Start (Seconds (0.00));

        } // end for b
    }
  // Add X2 interface
  lteHelper->AddX2Interface (enbNodes);

  // X2-based Handover
  lteHelper->HandoverRequest (Seconds (1.00), ueDevs.Get (2), enbDevs.Get (1), enbDevs.Get (0));



  // we use a /8 net for all UEs
  // m_uePgwAddressHelper.SetBase ("7.0.0.0", "255.0.0.0");
  //m_s1uIpv4AddressHelper.SetBase ("10.0.0.0", "255.255.255.252");
  // m_x2Ipv4AddressHelper.SetBase ("12.0.0.0", "255.255.255.252");
  NS_LOG_INFO("remoteHost"<<std::endl);
  PrintIPAddress(remoteHost);
  NS_LOG_INFO("pgw"<<std::endl);
  PrintIPAddress(pgw);
  NS_LOG_INFO("enbNodes"<<std::endl);
  PrintIPAddress(enbNodes.Get(0));
  PrintIPAddress(enbNodes.Get(1));
  NS_LOG_INFO("ueNodes"<<std::endl);
  PrintIPAddress(ueNodes.Get(0));
  PrintIPAddress(ueNodes.Get(1));
  PrintIPAddress(ueNodes.Get(2));
  

  p2ph.EnablePcapAll  ("first");

  Simulator::Stop (Seconds (2.500));
 
  // configure all the simulation scenario here...
  
  AnimationInterface anim("epc4.xml");
  //anim.EnablePacketMetadata (true);
  Simulator::Run ();
 
  // GtkConfigStore config;
  // config.ConfigureAttributes ();
 
  Simulator::Destroy ();
  return 0;
}

