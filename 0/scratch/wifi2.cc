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
#include "ns3/internet-module.h"
 
using namespace ns3;

Ptr<YansWifiPhy>
GetYansWifiPhyPtr (const NetDeviceContainer &nc)
{
  Ptr<WifiNetDevice> wnd = nc.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<WifiPhy> wp = wnd->GetPhy ();
  return wp->GetObject<YansWifiPhy> ();
}

NS_LOG_COMPONENT_DEFINE ("wifi2");
int main (int argc, char *argv[])
{   
  CommandLine cmd;
  cmd.Parse (argc, argv);
 
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  

  std::string phyMode ("OfdmRate12Mbps");

  NodeContainer c;
  c.Create (2);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
// ns-3 supports RadioTap and Prism tracing extensions for 802.11
  wifiPhy.Set ("ChannelNumber", UintegerValue (5));
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
                                "Exponent", DoubleValue (3.0));
  wifiPhy.SetChannel (wifiChannel.Create ());

// Add a non-QoS upper mac, and disable rate control (i.e. ConstantRateWifiManager)
 WifiMacHelper wifiMac;
 wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                              "DataMode",StringValue (phyMode),
                              "ControlMode",StringValue (phyMode));
// Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer Apdevice; 
  NetDeviceContainer Stadevice;


  Apdevice = wifi.Install (wifiPhy, wifiMac, c.Get(0));
  Stadevice = wifi.Install (wifiPhy, wifiMac, c.Get(1));
// Configure mobility
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

// other set up (e.g. InternetStack, Application)
  
  InternetStackHelper internet0;
  internet0.Install (c);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterfaces;
  staInterfaces=address.Assign (Stadevice);
  address.Assign (Apdevice);

  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (c.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (staInterfaces.GetAddress (0), 9);
   //设置Server服务器在端口9等待接收并回传数据
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  //将Client服务安装在wifi的AP节点上
  ApplicationContainer clientApps = echoClient.Install (c.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));



  
  AnimationInterface anim("wifi2.xml");
  anim.EnablePacketMetadata (true);

  Simulator::Run ();
  //NS_LOG_UNCOND(int(phySta->GetChannelNumber()));
 
  Simulator::Destroy ();
  return 0;
}
