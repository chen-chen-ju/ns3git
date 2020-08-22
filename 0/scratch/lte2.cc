#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include <ns3/building.h>
#include "ns3/netanim-module.h"
#include "ns3/buildings-propagation-loss-model.h"
#include "ns3/mobility-building-info.h"
#include "ns3/oh-buildings-propagation-loss-model.h"
#include <ns3/log.h>

 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("lte2");
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
 
  //lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::LogDistancePropagationLossModel"));
//change to use the fading model with building
lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::OhBuildingsPropagationLossModel"));
//lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::OkumuraHataPropagationLossModel"));
  // Uncomment to enable logging
  //lteHelper->EnableLogComponents ();
 

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (3);

//building model 
double x_min = 80.0;
double x_max = 120.0;
double y_min = -10.0;
double y_max = 10.0;
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
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add (Vector (100.0, 100.0, 20.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  Ptr<ListPositionAllocator> positionAlloc1 =
    CreateObject<ListPositionAllocator>();
  positionAlloc1->Add (Vector (0.0, 100.0, 1.0));
  positionAlloc1->Add (Vector (100.0, 0.0, 1.0));
  positionAlloc1->Add (Vector (200.0, 100.0, 1.0));
  mobility.SetPositionAllocator (positionAlloc1);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);
  BuildingsHelper::Install (ueNodes);
  BuildingsHelper::MakeMobilityModelConsistent ();

Ptr<MobilityModel> mm= enbNodes.Get(0)->GetObject<MobilityModel> ();
Ptr<MobilityModel> mm0 = ueNodes.Get(0)->GetObject<MobilityModel> ();
Ptr<MobilityBuildingInfo> bmm0 = mm0->GetObject<MobilityBuildingInfo> ();
Ptr<MobilityModel> mm1 = ueNodes.Get(1)->GetObject<MobilityModel> ();
Ptr<MobilityBuildingInfo> bmm1 = mm1->GetObject<MobilityBuildingInfo> ();
Ptr<MobilityModel> mm2 = ueNodes.Get(2)->GetObject<MobilityModel> ();
Ptr<MobilityBuildingInfo> bmm2 = mm2->GetObject<MobilityBuildingInfo> ();
NS_LOG_UNCOND(bmm0->IsIndoor());
NS_LOG_UNCOND(bmm1->IsIndoor());
NS_LOG_UNCOND(bmm2->IsIndoor());

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  // Default scheduler is PF, uncomment to use RR
  //lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
 
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

  AnimationInterface anim("lte2.xml");
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (statsStartTime)));
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (statsDuration)));
  
  
  Simulator::Run ();
  Ptr<OhBuildingsPropagationLossModel> propagationLossModel = CreateObject<OhBuildingsPropagationLossModel> ();
  double loss0 = propagationLossModel->GetLoss (mm0, mm);
  double loss1 = propagationLossModel->GetLoss (mm1, mm);
  double loss2 = propagationLossModel->GetLoss (mm2, mm);
  NS_LOG_UNCOND(loss0<< "\t"<<loss1<< "\t"<<loss2);
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
