/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Five base nodes, one mover node
 *
 *      m ->
 *     /
 *    /
 *   /
 *  b1         b2         b3         b4         b5
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
//#include "nqos-wifi-mac-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-velocity-mobility-model.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


NS_LOG_COMPONENT_DEFINE ("wireless");

using namespace ns3;

Ptr<ConstantVelocityMobilityModel> cvmm;
double position_interval = 1.0;
std::string tracebase = "wifi-example";

// two callbacks
void printPosition() 
{
  Vector thePos = cvmm->GetPosition();
  Simulator::Schedule(Seconds(position_interval), &printPosition);
  std::cout << "position: " << thePos << std::endl;
}

void stopMover() 
{
  cvmm -> SetVelocity(Vector(0,0,0));
}

int main (int argc, char *argv[])
{
  // Dsss options: DsssRate1Mbps, DsssRate2Mbps, DsssRate5_5Mbps, DsssRate11Mbps
  // also ErpOfdmRate to 54 Mbps and OfdmRate to 150Mbps w 40MHz band-width
  std::string phyMode = "DsssRate1Mbps";                
  //phyMode = "DsssRate11Mbps";

  int bottomrow = 5;            // number of bottom-row nodes
  int spacing = 200;            // between bottom-row nodes
  int mheight = 150;            // height of mover above bottom row
  int brheight = 50;            // height of bottom row

  int X = (bottomrow-1)*spacing+1;              // X and Y are the dimensions of the field
  //int Y = 300; 
  int packetsize = 500;
  double factor = 1.0;  // allows slowing down rate and extending runtime; same total # of packets
  int endtime = (int)100*factor;
  double speed = (X-1.0)/endtime;       
  double bitrate = 80*1000.0/factor;  // bits/sec
  uint32_t interval = 1000*packetsize*8/bitrate*1000;    // in microsec, computed from packetsize and bitrate
  uint32_t packetcount = 1000000*endtime/ interval;
  std::cout << "interval = " << interval <<", rate=" << bitrate << ", packetcount=" << packetcount << std::endl;

  CommandLine cmd;              // no options, actually
  cmd.Parse (argc, argv);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Set non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  // Create nodes
  NodeContainer fixedpos;
  fixedpos.Create(bottomrow);
  Ptr<Node> lowerleft = fixedpos.Get(0);
  Ptr<Node> mover = CreateObject<Node>();

  // The below set of helpers will help us to put together the desired Wi-Fi behavior
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager"); // Use AARF rate control
  // to view AARF rate changes, set in the shell NS_LOG=AarfWifiManager=level_debug

  // The PHY layer here is "yans"
  YansWifiPhyHelper wifiPhyHelper =  YansWifiPhyHelper::Default ();
  // for .pcap tracing
  // wifiPhyHelper.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  YansWifiChannelHelper wifiChannelHelper;              // *not* ::Default() !
  wifiChannelHelper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel"); // pld: default?
  // the following has an absolute cutoff at distance > 250
  wifiChannelHelper.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(250)); 
  Ptr<YansWifiChannel> pchan = wifiChannelHelper.Create ();
  wifiPhyHelper.SetChannel (pchan);

  // Add a non-QoS upper-MAC layer "AdhocWifiMac", and set rate control
  WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType ("ns3::AdhocWifiMac"); 
  NetDeviceContainer devices = wifi.Install (wifiPhyHelper, wifiMacHelper, fixedpos);
  devices.Add (wifi.Install (wifiPhyHelper, wifiMacHelper, mover));

  // set positions. 
  MobilityHelper sessile;               // for fixed nodes
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  int Xpos = 0;
  for (int i=0; i<bottomrow; i++) {
        positionAlloc->Add(Vector(Xpos, brheight, 0.0));
        Xpos += spacing;
  }
  sessile.SetPositionAllocator (positionAlloc);
  sessile.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  sessile.Install (fixedpos);

  // ConstantVelocityMobilityModel is a subclass of MobilityModel
  Vector pos (0, mheight+brheight, 0);
  Vector vel (speed, 0, 0);
  MobilityHelper mobile; 
  mobile.SetMobilityModel("ns3::ConstantVelocityMobilityModel");        // no Attributes
  mobile.Install(mover);
  cvmm = mover->GetObject<ConstantVelocityMobilityModel> ();
  cvmm->SetPosition(pos);
  cvmm->SetVelocity(vel);
  std::cout << "position: " << cvmm->GetPosition() << " velocity: " << cvmm->GetVelocity() << std::endl;
  std::cout << "mover mobility model: " << mobile.GetMobilityModelType() << std::endl; // just for confirmation

  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  Ipv4ListRoutingHelper listrouting;
  //listrouting.Add(olsr, 10);                          // generates less traffic
  listrouting.Add(aodv, 10);                            // fastest to find new routes

  InternetStackHelper internet;
  internet.SetRoutingHelper(listrouting);
  internet.Install (fixedpos);
  internet.Install (mover);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");           // there is only one subnet
  Ipv4InterfaceContainer i = ipv4.Assign (devices);
  
  uint16_t port = 80;

  // create a receiving application (UdpServer) on node mover
  Address sinkaddr(InetSocketAddress (Ipv4Address::GetAny (), port));
  Config::SetDefault("ns3::UdpServer::Port", UintegerValue(port));

  Ptr<UdpServer> UdpRecvApp = CreateObject<UdpServer>();
  UdpRecvApp->SetStartTime(Seconds(0.0));
  UdpRecvApp->SetStopTime(Seconds(endtime+60));
  mover->AddApplication(UdpRecvApp);

// create sender application on node lowerleft of type UdpClient
// packets are marked in the tracefile with ns3::SeqTsHeader, including sequence numbers

  Ptr<Ipv4> m4 = mover->GetObject<Ipv4>();
  Ipv4Address Maddr = m4->GetAddress(1,0).GetLocal();
  std::cout << "IPv4 address of mover: " << Maddr << std::endl;
  Address moverAddress (InetSocketAddress (Maddr, port));

  Config::SetDefault("ns3::UdpClient::MaxPackets", UintegerValue(packetcount));
  Config::SetDefault("ns3::UdpClient::PacketSize", UintegerValue(packetsize));
  Config::SetDefault("ns3::UdpClient::Interval",   TimeValue (MicroSeconds (interval)));

  Ptr<UdpClient> UdpSendApp = CreateObject<UdpClient>();
  UdpSendApp -> SetRemote(Maddr, port);
  UdpSendApp -> SetStartTime(Seconds(0.0));
  UdpSendApp -> SetStopTime(Seconds(endtime));
  lowerleft->AddApplication(UdpSendApp);

  // Tracing
  //wifiPhyHelper.EnablePcap (tracebase, devices);

  AsciiTraceHelper ascii;
  wifiPhyHelper.EnableAsciiAll (ascii.CreateFileStream (tracebase + ".tr"));

  // create animation file, to be run with 'netanim'
  AnimationInterface anim (tracebase +".xml"); 
  anim.SetMobilityPollInterval(Seconds(0.1));

  // uncomment the next line to verify that node 'mover' is actually moving
  //Simulator::Schedule(Seconds(position_interval), &printPosition);

  Simulator::Schedule(Seconds(endtime), &stopMover);

  Simulator::Stop(Seconds (endtime+60));
  Simulator::Run ();
  Simulator::Destroy ();

  int pktsRecd = UdpRecvApp->GetReceived();
  std::cout << "packets received: " << pktsRecd << std::endl;
  std::cout << "packets recorded as lost: " << (UdpRecvApp->GetLost()) << std::endl;
  std::cout << "packets actually lost: " << (packetcount - pktsRecd) << std::endl;
 
  return 0;
}
