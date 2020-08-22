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
 
int main (int argc, char *argv[])
{   
  double interPacketInterval=100;
  CommandLine cmd;
  cmd.Parse (argc, argv);
 
 
 
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // Parse again so you can override default values from the command line
  cmd.Parse (argc, argv);
 
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
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
  enbNodes.Create (1);
  ueNodes.Create (1);


  // Install Mobility Model
  MobilityHelper mobility;

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  Ptr<ListPositionAllocator> positionAlloc=CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(5,5,0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(pgw);
  BuildingsHelper::Install (pgw);

  Ptr<ListPositionAllocator> positionAlloc0=CreateObject<ListPositionAllocator>();
  positionAlloc0->Add(Vector(10,10,0));
  mobility.SetPositionAllocator(positionAlloc0);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(remoteHostContainer);
  BuildingsHelper::Install (remoteHostContainer);
 
  Ptr<ListPositionAllocator> positionAlloc1=CreateObject<ListPositionAllocator>();
  positionAlloc1->Add(Vector(3,0,0));
  mobility.SetPositionAllocator(positionAlloc1);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);
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
for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ue = ueNodes.Get (u);
    Ptr<NetDevice> ueLteDevice = ueDevs.Get (u);
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevice));
    // 为用户设置默认网关
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

// Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activate a data radio bearer
Ptr<EpcTft> tft = Create<EpcTft> ();
EpcTft::PacketFilter pf;
pf.localPortStart = 1234;
pf.localPortEnd = 1234;
tft->Add (pf);
lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);

uint16_t dlPort = 1234;
uint16_t ulPort = 2000;
PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny (), dlPort));
PacketSinkHelper ulPacketSinkHelper("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),ulPort));

UdpClientHelper ulClient (remoteHostAddr,ulPort);
ulClient.SetAttribute("Interval",TimeValue(MilliSeconds(interPacketInterval)));
ulClient.SetAttribute("MaxPackets",UintegerValue(1000000));

ApplicationContainer serverApps = packetSinkHelper.Install (ueNodes);
serverApps.Start (Seconds (0.01));
UdpClientHelper client (ueIpIface.GetAddress (0), dlPort);
ApplicationContainer clientApps = client.Install (remoteHost);
clientApps.Start (Seconds (0.01));

  //lteHelper->EnableTraces ();
 
  Simulator::Stop (Seconds (1.05));
 
  // configure all the simulation scenario here...
  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();
  AnimationInterface anim("epc1.xml");
  anim.EnablePacketMetadata (true);
  Simulator::Run ();
 
  // GtkConfigStore config;
  // config.ConfigureAttributes ();
 
  Simulator::Destroy ();
  return 0;
}
