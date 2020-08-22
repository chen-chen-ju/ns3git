#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include "ns3/building.h"
#include "ns3/netanim-module.h"
#include <ns3/log.h>
//#include "ns3/gtk-config-store.h"
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("lte1");
int main (int argc, char *argv[])
{   
  CommandLine cmd;
  cmd.Parse (argc, argv);
 
 
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // Parse again so you can override default values from the command line
  cmd.Parse (argc, argv);
 
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
 
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::LogDistancePropagationLossModel"));
  // Uncomment to enable logging
//  lteHelper->EnableLogComponents ();
 
  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (3);
 
double x_min = 5.0;
double x_max = 15.0;
double y_min = 5.0;
double y_max = 25.0;
double z_min = 0.0;
double z_max = 10.0;
Ptr<Building> b = CreateObject <Building> ();
b->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
b->SetBuildingType (Building::Residential);
b->SetExtWallsType (Building::ConcreteWithWindows);
b->SetNFloors (3);
b->SetNRoomsX (3);
b->SetNRoomsY (2);  

// Install Mobility Model
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);
//  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//  mobility.Install (ueNodes);
//  BuildingsHelper::Install (ueNodes);

  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", StringValue ("100.0"),
                                 "Y", StringValue ("100.0"),
                                 "Rho", StringValue ("ns3::UniformRandomVariable [Min=0|Max=30]"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes.Get(0));
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", StringValue ("200.0"),
                                 "Y", StringValue ("200.0"),
                                 "Rho", StringValue ("ns3::UniformRandomVariable [Min=0|Max=30.0]"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes.Get(1));
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", StringValue ("10.0"),
                                 "Y", StringValue ("10.0"),
                                 "Rho", StringValue ("ns3::UniformRandomVariable [Min=0|Max=5.0]"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes.Get(2));
  //mobility.Install(ueNodes.Get(2));
  BuildingsHelper::Install (ueNodes);
 
  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
 
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);
 
  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));
 
  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

  Ptr<LteEnbNetDevice> lteEnbDev = enbDevs.Get (0)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy = lteEnbDev->GetPhy ();
  enbPhy->SetAttribute ("TxPower", DoubleValue (30.0));
  enbPhy->SetAttribute ("NoiseFigure", DoubleValue (5.0));
  //lteHelper->EnableTraces ();
 
  double statsStartTime = 0.300; // need to allow for RRC connection establishment + SRS
  double statsDuration = 0.4;
  Simulator::Stop (Seconds (1.05));
 
  // configure all the simulation scenario here...
  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();

  AnimationInterface anim("first.xml");
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (statsStartTime)));
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (statsDuration)));
  
  
  Simulator::Run ();
  NS_LOG_UNCOND("User" << "\t" << "thr"<<"\t" << "thRatio");
  std::vector <uint64_t> dlDataRxed;
  double totalData = 0;
  int num=3;
  for (int i = 0; i < num; i++)
    {
      // get the imsi
      uint64_t imsi = ueDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetImsi ();
      // get the lcId
      uint8_t lcId = 3;
      dlDataRxed.push_back (rlcStats->GetDlRxData (imsi, lcId));
      totalData += (double)dlDataRxed.at (i);
    }
  // GtkConfigStore config;
  // config.ConfigureAttributes ();
 for (int i = 0; i < num; i++)
    {
      double thrRatio = (double)dlDataRxed.at (i) / totalData;
      NS_LOG_UNCOND( i << "\t" << (double)dlDataRxed.at (i) / statsDuration<< "\t" <<thrRatio);
    }
  Simulator::Destroy ();
  return 0;
}
