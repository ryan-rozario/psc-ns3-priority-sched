/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * NIST-developed software is provided by NIST as a public
 * service. You may use, copy and distribute copies of the software in
 * any medium, provided that you keep intact this entire notice. You
 * may improve, modify and create derivative works of the software or
 * any portion of the software, and you may copy and distribute such
 * modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the
 * National Institute of Standards and Technology as the source of the
 * software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES
 * NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY
 * OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
 * WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED
 * OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT
 * WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of
 * using and distributing the software and you assume all risks
 * associated with its use, including but not limited to the risks and
 * costs of program errors, compliance with applicable laws, damage to
 * or loss of data, programs or equipment, and the unavailability or
 * interruption of operation. This software is not intended to be used
 * in any situation where a failure could cause risk of injury or
 * damage to property. The software developed by NIST employees is not
 * subject to copyright protection within the United States.
 */


#include "ns3/lte-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store.h"
#include <cfloat>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include "ns3/netanim-module.h"

using namespace ns3;

// This trace will log packet transmissions and receptions from the application
// layer.  The parameter 'localAddrs' is passed to this trace in case the
// address passed by the trace is not set (i.e., is '0.0.0.0' or '::').  The
// trace writes to a file stream provided by the first argument; by default,
// this trace file is 'UePacketTrace.tr'
void
UePacketTrace (Ptr<OutputStreamWrapper> stream, const Address &localAddrs, std::string context, Ptr<const Packet> p, const Address &srcAddrs, const Address &dstAddrs)
{
  std::ostringstream oss;
  *stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << context << "\t" << p->GetSize () << "\t";
  if (InetSocketAddress::IsMatchingType (srcAddrs))
    {
      oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ();
      if (!oss.str ().compare ("0.0.0.0")) //srcAddrs not set
        {
          *stream->GetStream () << Ipv4Address::ConvertFrom (localAddrs) << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 ();
          if (!oss.str ().compare ("0.0.0.0")) //dstAddrs not set
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" <<  Ipv4Address::ConvertFrom (localAddrs) << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else if (Inet6SocketAddress::IsMatchingType (srcAddrs))
    {
      oss << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 ();
      if (!oss.str ().compare ("::")) //srcAddrs not set
        {
          *stream->GetStream () << Ipv6Address::ConvertFrom (localAddrs) << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 ();
          if (!oss.str ().compare ("::")) //dstAddrs not set
            {
              *stream->GetStream () << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Ipv6Address::ConvertFrom (localAddrs) << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else
    {
      *stream->GetStream () << "Unknown address type!" << std::endl;
    }
}


// class for d2d device
 class d2d_user {
        public :
        
          NodeContainer d2dNodes; // Stores a pair of d2d nodes for communication
          // int Node_id ;
          int application_type; // Type of node pair, public safety{1} or commercial{2}
          // int count = -1;
          Ipv4Address address1, address2; // Address of the nodes
 };
 

/*
 * The topology is the following:
 *
 *    UE2 (-10.0, 0.0, 1.5).....(10 m).....eNB (0.0, 0.0, 30.0).....(10 m).....UE1 (10.0, 0.0, 1.5)
 *
 * Please refer to the Sidelink section of the LTE user documentation for more details.
 *
 */

NS_LOG_COMPONENT_DEFINE ("LteSlInCovrgCommMode1");

int main (int argc, char *argv[])
{
  Time simTime = Seconds (20);
  bool enableNsLogs = false;
  bool useIPv6 = false;

  CommandLine cmd;
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("enableNsLogs", "Enable ns-3 logging (debug builds)", enableNsLogs);
  cmd.AddValue ("useIPv6", "Use IPv6 instead of IPv4", useIPv6);
  cmd.Parse (argc, argv);

  // Configure the scheduler
  Config::SetDefault ("ns3::RrSlFfMacScheduler::Itrp", UintegerValue (0));
  //The number of RBs allocated per UE for Sidelink
  Config::SetDefault ("ns3::RrSlFfMacScheduler::SlGrantSize", UintegerValue (5));

  //Set the frequency

  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (100));
  Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", UintegerValue (100));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (18100));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (50));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (50));

  // Set error models
  Config::SetDefault ("ns3::LteSpectrumPhy::SlCtrlErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::SlDataErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (false));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  // parse again so we can override input file default values via command line
  cmd.Parse (argc, argv);

  if (enableNsLogs)
    {
      LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);

      LogComponentEnable ("LteUeRrc", logLevel);
      LogComponentEnable ("LteUeMac", logLevel);
      LogComponentEnable ("LteSpectrumPhy", logLevel);
      LogComponentEnable ("LteUePhy", logLevel);
      LogComponentEnable ("LteEnbPhy", logLevel);
    }

  //Set the UEs power in dBm
  // Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (23.0));
  //Set the eNBs power in dBm
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (30.0));

  //Sidelink bearers activation time
  Time slBearersActivationTime = Seconds (2.0);

  //Create the helpers
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  //Create and set the EPC helper
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ////Create Sidelink helper and set lteHelper
  Ptr<LteSidelinkHelper> proseHelper = CreateObject<LteSidelinkHelper> ();
  proseHelper->SetLteHelper (lteHelper);

  //Set pathloss model
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

  //Enable Sidelink
  lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));

  //Sidelink Round Robin scheduler
  lteHelper->SetSchedulerType ("ns3::RrSlFfMacScheduler");

  std::cout<<"Creating enb";
  //Create nodes (eNb + UEs)
  NodeContainer enbNode;
  enbNode.Create (1);
  NS_LOG_INFO ("eNb node id = [" << enbNode.Get (0)->GetId () << "]");
  //d2d devices - 10 pairs of d2d nodes
  d2d_user d2d[10];
  std::cout<<"Creating d2d node pairs";
  // Creating 5 pairs public safety node pairs
  for(int i=0 ; i<5; i++){
    d2d[i].d2dNodes.Create_new(2 , 1);
    d2d[i].application_type = 1; // Assigning the type to public safety pairs

    // for(int j = 0; j < 2; j++){
    //   Ptr<LteUeNetDevice> tmpDev = d2d[i].d2dNodes.Get (j)->GetDevice (0)->GetObject<LteUeNetDevice> ();
    //   Ptr<LteUePhy> uePhy1 = tmpDev->GetPhy();
    //   uePhy1->SetAttribute("TxPower", DoubleValue (12.0));
    //   // DoubleValue cnt;
    //   // uePhy1->GetAttribute("TxPower", cnt);  
    // }
  }
  // Creating 5 pairs commercial nodes
  for(int i = 5; i < 10; i++){
    d2d[i].d2dNodes.Create_new(2, 2);
    d2d[i].application_type = 2;  // Assigning the type to commercial pairs

    // for(int j = 0; j < 2; j++){
    //   Ptr<LteUeNetDevice> tmpDev = d2d[i].d2dNodes.Get (j)->GetDevice (0)->GetObject<LteUeNetDevice> ();
    //   Ptr<LteUePhy> uePhy1 = tmpDev->GetPhy();
    //   uePhy1->SetAttribute("TxPower", DoubleValue (15.0));
    //   // DoubleValue cnt;
    //   // uePhy1->GetAttribute("TxPower", cnt);  
    // }
  }
  
  std::cout<<"Allocating positions";
  //Position of the nodes
  Ptr<ListPositionAllocator> positionAllocEnb = CreateObject<ListPositionAllocator> ();
  positionAllocEnb->Add (Vector (0.0, 0.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d1 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d1->Add (Vector (-15.0, 6.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d2 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d2->Add (Vector (-16.0, 6.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d3 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d3->Add (Vector (3.0, 3.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d4 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d4->Add (Vector (4.0, 3.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d5 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d5->Add (Vector (16.0, 5.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d6 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d6->Add (Vector (17.0, 5.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d7 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d7->Add (Vector (22.0, -4.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d8 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d8->Add (Vector (24.0, -4.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d9 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d9->Add (Vector (-23.0, 7.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d10 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d10->Add (Vector (-19.0, 7.0, 0.0));

  Ptr<ListPositionAllocator> positionAllocd2d11 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d11->Add (Vector (22.0, -2.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d12 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d12->Add (Vector (23.0, -2.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d13 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d13->Add (Vector (-20.0, 6.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d14 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d14->Add (Vector (-21.0, 6.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d15 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d15->Add (Vector (-10.0, -9.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d16 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d16->Add (Vector (-10.0, -8.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d17 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d17->Add (Vector (4.0, 4.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d18 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d18->Add (Vector (4.0, 5.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d19 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d19->Add (Vector (-12.0, 10.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d20 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d20->Add (Vector (-14.0, 10.0, 0.0));


  std::cout<<"Setting up mobility";
  //Install mobility
  MobilityHelper mobilityeNodeB;
  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.SetPositionAllocator (positionAllocEnb);
  mobilityeNodeB.Install (enbNode);

  MobilityHelper mobilityd2d1;
  mobilityd2d1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d1.SetPositionAllocator (positionAllocd2d1);
  mobilityd2d1.Install (d2d[0].d2dNodes.Get (0));
  
  MobilityHelper mobilityd2d2;
  mobilityd2d2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d2.SetPositionAllocator (positionAllocd2d2);
  mobilityd2d2.Install (d2d[0].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d3;
  mobilityd2d3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d3.SetPositionAllocator (positionAllocd2d3);
  mobilityd2d3.Install (d2d[1].d2dNodes.Get (0));

  MobilityHelper mobilityd2d4;
  mobilityd2d4.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d4.SetPositionAllocator (positionAllocd2d4);
  mobilityd2d4.Install (d2d[1].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d5;
  mobilityd2d5.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d5.SetPositionAllocator (positionAllocd2d5);
  mobilityd2d5.Install (d2d[2].d2dNodes.Get (0));

  MobilityHelper mobilityd2d6;
  mobilityd2d6.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d6.SetPositionAllocator (positionAllocd2d6);
  mobilityd2d6.Install (d2d[2].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d7;
  mobilityd2d7.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d7.SetPositionAllocator (positionAllocd2d7);
  mobilityd2d7.Install (d2d[3].d2dNodes.Get (0));
  
  MobilityHelper mobilityd2d8;
  mobilityd2d8.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d8.SetPositionAllocator (positionAllocd2d8);
  mobilityd2d8.Install (d2d[3].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d9;
  mobilityd2d9.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d9.SetPositionAllocator (positionAllocd2d9);
  mobilityd2d9.Install (d2d[4].d2dNodes.Get (0));
  
  MobilityHelper mobilityd2d10;
  mobilityd2d10.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d10.SetPositionAllocator (positionAllocd2d10);
  mobilityd2d10.Install (d2d[4].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d11;
  mobilityd2d11.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d11.SetPositionAllocator (positionAllocd2d11);
  mobilityd2d11.Install (d2d[5].d2dNodes.Get (0));

  MobilityHelper mobilityd2d12;
  mobilityd2d12.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d12.SetPositionAllocator (positionAllocd2d12);
  mobilityd2d12.Install (d2d[5].d2dNodes.Get (1));

  MobilityHelper mobilityd2d13;
  mobilityd2d13.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d13.SetPositionAllocator (positionAllocd2d13);
  mobilityd2d13.Install (d2d[6].d2dNodes.Get (0));

  MobilityHelper mobilityd2d14;
  mobilityd2d14.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d14.SetPositionAllocator (positionAllocd2d14);
  mobilityd2d14.Install (d2d[6].d2dNodes.Get (1));

  MobilityHelper mobilityd2d15;
  mobilityd2d15.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d15.SetPositionAllocator (positionAllocd2d15);
  mobilityd2d15.Install (d2d[7].d2dNodes.Get (0));

  MobilityHelper mobilityd2d16;
  mobilityd2d16.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d16.SetPositionAllocator (positionAllocd2d16);
  mobilityd2d16.Install (d2d[7].d2dNodes.Get (1));

  MobilityHelper mobilityd2d17;
  mobilityd2d17.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d17.SetPositionAllocator (positionAllocd2d17);
  mobilityd2d17.Install (d2d[8].d2dNodes.Get (0));

  MobilityHelper mobilityd2d18;
  mobilityd2d18.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d18.SetPositionAllocator (positionAllocd2d18);
  mobilityd2d18.Install (d2d[8].d2dNodes.Get (1));

  MobilityHelper mobilityd2d19;
  mobilityd2d19.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d19.SetPositionAllocator (positionAllocd2d19);
  mobilityd2d19.Install (d2d[9].d2dNodes.Get (0));

  MobilityHelper mobilityd2d20;
  mobilityd2d20.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d20.SetPositionAllocator (positionAllocd2d20);
  mobilityd2d20.Install (d2d[9].d2dNodes.Get (1));


  //Install LTE devices to the nodes and fix the random number stream
  std::cout<<"Installing LTE devices to nodes";
  int64_t randomStream = 1;
  NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice (enbNode);
  randomStream += lteHelper->AssignStreams (enbDevs, randomStream);
   NetDeviceContainer ueDevs1 = lteHelper->InstallUeDevice (d2d[0].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs1, randomStream);
  NetDeviceContainer ueDevs2 = lteHelper->InstallUeDevice (d2d[1].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs2, randomStream);
  NetDeviceContainer ueDevs3 = lteHelper->InstallUeDevice (d2d[2].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs3, randomStream);
  NetDeviceContainer ueDevs4 = lteHelper->InstallUeDevice (d2d[3].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs4, randomStream);
  NetDeviceContainer ueDevs5 = lteHelper->InstallUeDevice (d2d[4].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs5, randomStream);
  
  NetDeviceContainer ueDevs6 = lteHelper->InstallUeDevice (d2d[5].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs6, randomStream);
  NetDeviceContainer ueDevs7 = lteHelper->InstallUeDevice (d2d[6].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs7, randomStream);
  NetDeviceContainer ueDevs8 = lteHelper->InstallUeDevice (d2d[7].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs8, randomStream);
  NetDeviceContainer ueDevs9 = lteHelper->InstallUeDevice (d2d[8].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs9, randomStream);
  NetDeviceContainer ueDevs10 = lteHelper->InstallUeDevice (d2d[9].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs10, randomStream);

  //Configure Sidelink Pool
  Ptr<LteSlEnbRrc> enbSidelinkConfiguration = CreateObject<LteSlEnbRrc> ();
  enbSidelinkConfiguration->SetSlEnabled (true);

  //Preconfigure pool for the group
  LteRrcSap::SlCommTxResourcesSetup pool;

  pool.setup = LteRrcSap::SlCommTxResourcesSetup::SCHEDULED;
  //BSR timers
  pool.scheduled.macMainConfig.periodicBsrTimer.period = LteRrcSap::PeriodicBsrTimer::sf16;
  pool.scheduled.macMainConfig.retxBsrTimer.period = LteRrcSap::RetxBsrTimer::sf640;
  //MCS
  pool.scheduled.haveMcs = true;
  pool.scheduled.mcs = 16;
  //resource pool
  LteSlResourcePoolFactory pfactory;
  pfactory.SetHaveUeSelectedResourceConfig (false); //since we want eNB to schedule

  //Control
  pfactory.SetControlPeriod ("sf40");
  pfactory.SetControlBitmap (0x00000000FF); //8 subframes for PSCCH
  pfactory.SetControlOffset (0);
  pfactory.SetControlPrbNum (22);
  pfactory.SetControlPrbStart (0);
  pfactory.SetControlPrbEnd (49);

  //Data: The ns3::RrSlFfMacScheduler is responsible to handle the parameters


  pool.scheduled.commTxConfig = pfactory.CreatePool ();

  uint32_t groupL2Address = 255;

  enbSidelinkConfiguration->AddPreconfiguredDedicatedPool (groupL2Address, pool);
  lteHelper->InstallSidelinkConfiguration (enbDevs, enbSidelinkConfiguration);

  //pre-configuration for the UEs
  std::cout<<"Pre-config for the UEs";
  Ptr<LteSlUeRrc> ueSidelinkConfiguration = CreateObject<LteSlUeRrc> ();
  ueSidelinkConfiguration->SetSlEnabled (true);
  LteRrcSap::SlPreconfiguration preconfiguration;
  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs1, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs2, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs3, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs4, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs5, ueSidelinkConfiguration);

  lteHelper->InstallSidelinkConfiguration (ueDevs6, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs7, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs8, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs9, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs10, ueSidelinkConfiguration);
  


  // Creating 5 pairs public safety node pairs
  for(int i=0 ; i<5; i++){
    for(int j = 0; j < 2; j++){
      Ptr<LteUeNetDevice> tmpDev = d2d[i].d2dNodes.Get (j)->GetDevice (0)->GetObject<LteUeNetDevice> ();
      Ptr<LteUePhy> uePhy1 = tmpDev->GetPhy();
      uePhy1->SetAttribute("TxPower", DoubleValue (12.0));
      // DoubleValue cnt;
      // uePhy1->GetAttribute("TxPower", cnt);  
    }
  }
  // Creating 5 pairs commercial nodes
  for(int i = 5; i < 10; i++){
    for(int j = 0; j < 2; j++){
      Ptr<LteUeNetDevice> tmpDev = d2d[i].d2dNodes.Get (j)->GetDevice (0)->GetObject<LteUeNetDevice> ();
      Ptr<LteUePhy> uePhy1 = tmpDev->GetPhy();
      uePhy1->SetAttribute("TxPower", DoubleValue (15.0));
      // DoubleValue cnt;
      // uePhy1->GetAttribute("TxPower", cnt);  
    }
  }




  std::cout<<"Setting up the internet stack";
  InternetStackHelper internet;
  internet.Install (d2d[0].d2dNodes);
  internet.Install (d2d[1].d2dNodes);
  internet.Install (d2d[2].d2dNodes);
  internet.Install (d2d[3].d2dNodes);
  internet.Install (d2d[4].d2dNodes);

  internet.Install (d2d[5].d2dNodes);
  internet.Install (d2d[6].d2dNodes);
  internet.Install (d2d[7].d2dNodes);
  internet.Install (d2d[8].d2dNodes);
  internet.Install (d2d[9].d2dNodes);
  Ipv4Address groupAddress4 ("225.0.0.0");     //use multicast address as destination
  Ipv6Address groupAddress6 ("ff0e::1");     //use multicast address as destination
  Address remoteAddress;
  Address localAddress;
  Ptr<LteSlTft> tft;

  std::cout<<"Setting up the routing protocol";
  if (!useIPv6)
    {
      

      Ipv4InterfaceContainer ueIpIface4;
      ueIpIface4 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs1));
      
      Ipv4InterfaceContainer ueIpIface5;
      ueIpIface5 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs2));
      
      Ipv4InterfaceContainer ueIpIface6;
      ueIpIface6 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs3));
      
      Ipv4InterfaceContainer ueIpIface7;
      ueIpIface7 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs4));

      Ipv4InterfaceContainer ueIpIface8;
      ueIpIface8 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs5));
      

      Ipv4InterfaceContainer ueIpIface10;
      ueIpIface10 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs6));

      Ipv4InterfaceContainer ueIpIface11;
      ueIpIface11 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs7));

      Ipv4InterfaceContainer ueIpIface12;
      ueIpIface12 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs8));

      Ipv4InterfaceContainer ueIpIface13;
      ueIpIface13 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs9));

      Ipv4InterfaceContainer ueIpIface14;
      ueIpIface14 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs10));



      // set the default gateway for the UE
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      for (uint32_t u = 0; u < d2d[0].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[0].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
        
      for (uint32_t u = 0; u < d2d[1].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[1].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
        
      for (uint32_t u = 0; u < d2d[2].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[2].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
        
      for (uint32_t u = 0; u < d2d[3].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[3].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        } 
        
      for (uint32_t u = 0; u < d2d[4].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[4].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }   

      for (uint32_t u = 0; u < d2d[5].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[5].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }   
        
      for (uint32_t u = 0; u < d2d[6].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[6].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }   

      for (uint32_t u = 0; u < d2d[7].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[7].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }   

      for (uint32_t u = 0; u < d2d[8].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[8].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }   

      for (uint32_t u = 0; u < d2d[9].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[9].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }   



      remoteAddress = InetSocketAddress (groupAddress4, 8000);
      localAddress = InetSocketAddress (Ipv4Address::GetAny (), 8000);
      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress4, groupL2Address);
    }
  else
    {
      Ipv6InterfaceContainer ueIpIface;
      ueIpIface = epcHelper->AssignUeIpv6Address (NetDeviceContainer (ueDevs1));

      // set the default gateway for the UE
      Ipv6StaticRoutingHelper Ipv6RoutingHelper;
      for (uint32_t u = 0; u < d2d[1].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[1].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv6StaticRouting> ueStaticRouting = Ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress6 (), 1);
        }
      remoteAddress = Inet6SocketAddress (groupAddress6, 8000);
      localAddress = Inet6SocketAddress (Ipv6Address::GetAny (), 8000);
      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress6, groupL2Address);
    }

  std::cout<<"Attach nodes to eNBs";
  //Attach each UE to the best available eNB
  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (0));
  lteHelper->Attach (ueDevs3, enbDevs.Get (0));
  lteHelper->Attach (ueDevs4, enbDevs.Get (0));
  lteHelper->Attach (ueDevs5, enbDevs.Get (0));
  lteHelper->Attach (ueDevs6, enbDevs.Get (0));
  lteHelper->Attach (ueDevs7, enbDevs.Get (0));
  lteHelper->Attach (ueDevs8, enbDevs.Get (0));
  lteHelper->Attach (ueDevs9, enbDevs.Get (0));
  lteHelper->Attach (ueDevs10, enbDevs.Get (0));


  for(int i=0; i<10; i++)
  {
    Ipv4Address localAddrs1 =  d2d[i].d2dNodes.Get(0)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
    d2d[i].address1 = localAddrs1;
    Ipv4Address localAddrs2 =  d2d[i].d2dNodes.Get(1)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
    d2d[i].address2 = localAddrs2;
  }

  ///*** Configure applications ***///

  //Set Application in the UEs
  OnOffHelper sidelinkClient_psc ("ns3::UdpSocketFactory", remoteAddress);
  OnOffHelper sidelinkClient_comm ("ns3::UdpSocketFactory", remoteAddress);
  sidelinkClient_psc.SetConstantRate (DataRate ("32kb/s"), 400);
  sidelinkClient_comm.SetConstantRate (DataRate ("16kb/s"), 200);


  ApplicationContainer clientApps_shard1;
  ApplicationContainer serverApps_shard1;
  int shard1[10];

  int t = 0;
  for(int i = 0; i < 5; i++){
    ApplicationContainer clientApps3 = sidelinkClient_psc.Install (d2d[i].d2dNodes.Get (0));
    clientApps3.Start (Seconds(t + 0.9));
    clientApps3.Stop (Seconds (11.0));
    clientApps_shard1.Add(clientApps3);

    shard1[t] = i;

    ApplicationContainer serverApps2;
    PacketSinkHelper sidelinkSink2 ("ns3::UdpSocketFactory", localAddress);
    serverApps2 = sidelinkSink2.Install (d2d[i].d2dNodes.Get (1));
    serverApps2.Start (Seconds (t+0.9));
    serverApps_shard1.Add(serverApps2);

    t++;
  }

  for(int i = 5; i < 10; i++){
    ApplicationContainer clientApps3 = sidelinkClient_comm.Install (d2d[i].d2dNodes.Get (0));
    clientApps3.Start (Seconds(t + 0.9));
    clientApps3.Stop (Seconds (11.0));
    clientApps_shard1.Add(clientApps3);

    shard1[t] = i;

    ApplicationContainer serverApps2;
    PacketSinkHelper sidelinkSink2 ("ns3::UdpSocketFactory", localAddress);
    serverApps2 = sidelinkSink2.Install (d2d[i].d2dNodes.Get (1));
    serverApps2.Start (Seconds (t+0.9));
    serverApps_shard1.Add(serverApps2);

    t++;
  }


  // ApplicationContainer clientApps = sidelinkClient.Install (ueNodes.Get (0));
  // //onoff application will send the first packet at :
  // //(2.9 (App Start Time) + (1600 (Pkt size in bits) / 16000 (Data rate)) = 3.0 sec
  // clientApps.Start (slBearersActivationTime + Seconds (0.9));
  // clientApps.Stop (simTime - slBearersActivationTime + Seconds (1.0));

  // ApplicationContainer serverApps;
  // PacketSinkHelper sidelinkSink ("ns3::UdpSocketFactory", localAddress);
  // serverApps = sidelinkSink.Install (ueNodes.Get (1));
  // serverApps.Start (Seconds (2.0));

  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs1, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs2, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs3, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs4, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs5, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs6, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs7, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs8, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs9, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs10, tft);
  ///*** End of application configuration ***///

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("UePacketTrace.tr");

  //Trace file table header
  *stream->GetStream () << "time(sec)\ttx/rx\tNodeID\tIMSI\tPktSize(bytes)\tIP[src]\tIP[dst]" << std::endl;

  std::ostringstream oss;

  if (!useIPv6)
    {
      // Set Tx traces
      for (uint16_t ac = 0; ac < clientApps_shard1.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          d2d[shard1[ac]].address1 = localAddrs;
          oss << "tx\t" << d2d[shard1[ac]].d2dNodes.Get (0)->GetId () << "\t" << d2d[shard1[ac]].d2dNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientApps_shard1.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }  

      // for (uint16_t ac = 0; ac < clientApps.GetN (); ac++)
      //   {
      //     Ipv4Address localAddrs =  clientApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      //     std::cout << "Tx address: " << localAddrs << std::endl;
      //     oss << "tx\t" << ueNodes.Get (0)->GetId () << "\t" << ueNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
      //     clientApps.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
      //     oss.str ("");
      //   }

      // Set Rx traces
        for (uint16_t ac = 0; ac < serverApps_shard1.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          d2d[shard1[ac]].address2 = localAddrs;
          oss << "rx\t" << d2d[shard1[ac]].d2dNodes.Get (1)->GetId () << "\t" << d2d[shard1[ac]].d2dNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverApps_shard1.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }


      // for (uint16_t ac = 0; ac < serverApps.GetN (); ac++)
      //   {
      //     Ipv4Address localAddrs =  serverApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      //     std::cout << "Rx address: " << localAddrs << std::endl;
      //     oss << "rx\t" << ueNodes.Get (1)->GetId () << "\t" << ueNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
      //     serverApps.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
      //     oss.str ("");
      //   }
    }
  else
    {
      // Set Tx traces
      for (uint16_t ac = 0; ac < clientApps_shard1.GetN (); ac++)
        {
          clientApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->AddMulticastAddress (groupAddress6);
          Ipv6Address localAddrs =  clientApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          oss << "tx\t" << d2d[1].d2dNodes.Get (0)->GetId () << "\t" << d2d[1].d2dNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientApps_shard1.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

      // Set Rx traces
      for (uint16_t ac = 0; ac < serverApps_shard1.GetN (); ac++)
        {
          serverApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->AddMulticastAddress (groupAddress6);
          Ipv6Address localAddrs =  serverApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          oss << "rx\t" << d2d[1].d2dNodes.Get (1)->GetId () << "\t" << d2d[1].d2dNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverApps_shard1.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

    }

  NS_LOG_INFO ("Enabling Sidelink traces...");
  lteHelper->EnableSidelinkTraces ();

  NS_LOG_INFO ("Starting simulation...");


  Simulator::Stop (simTime);

// Config::Set("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/TxPower/SetTxPower",DoubleValue (10.0));


// Ptr<LteUeNetDevice> ueDev1 = ueNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ();
// Ptr<LteUePhy> uePhy1 = ueDev1->GetPhy();

// uePhy1->SetAttribute("TxPower", DoubleValue (12.0));
// DoubleValue cnt;
// uePhy1->GetAttribute("TxPower", cnt);

// Ptr<LteUeNetDevice> ueDev2 = ueNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ();
// Ptr<LteUePhy> uePhy2 = ueDev2->GetPhy();

// DoubleValue cnt2;
// uePhy2->GetAttribute("TxPower", cnt2);

// uePhy1->GetAttribute("TxPower", cnt);

// std::cout<<cnt.Get()<<"    "<<cnt2.Get();
// std::cout << cnt.Get();

// Config::Set("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/TxPower",DoubleValue (10.0));

// Config::Set("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/TxPower",DoubleValue (10.0));

  AnimationInterface anim("acn_project.xml");

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
