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
 *
 * Author: Fernando J. Cintron <fernando.cintron@nist.gov>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/psc-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/random-variable-stream.h"

// Example of udp group echo server with ON-OFF clients.
// PscUdpGroupEchoServer offers different modes of operation
// by setting the session timeout time accordingly:
//       INF_SESSION - Session last infinitely
//  NO_GROUP_SESSION - No group session.
//         <timeout> - Session timeout time in seconds.
//
// The server can be set to not echoback source client
// with the echoback parameter set to false.

// Network Topology (with at least 2 nodes)
//
//  *    *
// n0   n1   ... nExtra-1  nExtra
//  |    |    |    |         |
//  ==========================
//     LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UdpGroupEchoExample");

int
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 2;
  uint32_t nExtra = 0;
  double simTime = 10;

  CommandLine cmd;
  cmd.AddValue ("nExtra", "Number of \"extra\" CSMA nodes/devices", nExtra);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("time", "Simulation time", simTime);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpGroupEchoExample", LOG_LEVEL_INFO);
      LogComponentEnable ("PscUdpGroupEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma += nExtra;

  NS_LOG_INFO ("CSMA nodes: " << nCsma);
  NS_LOG_INFO ("Simulation Time: " << simTime);

  NodeContainer csmaNodes;
  csmaNodes.Create (nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  // Server
  uint16_t serverPort = 9;
  // Set the session timeout time in seconds, else INF_SESSION or NO_GROUP_SESSION
  double timeout = INF_SESSION;
  bool echoback = false;
  PscUdpGroupEchoServerHelper echoServer (serverPort, timeout, echoback);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma - 1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (simTime));

  // Client
  // Set Exponential Random Variables: ON(75% with mean 2.5s), OFF(25%)
  Ptr<ExponentialRandomVariable> onRv = CreateObject<ExponentialRandomVariable> ();
  onRv->SetAttribute ("Mean", DoubleValue (2.5));
  //onRv->SetAttribute ("Bound", DoubleValue (10.0));

  Ptr<ExponentialRandomVariable> offRv = CreateObject<ExponentialRandomVariable> ();
  offRv->SetAttribute ("Mean", DoubleValue (0.8333));
  //offRv->SetAttribute ("Bound", DoubleValue (40.0));

  //DataRate[bps]=Payload[bits] * PacketRate[per second] = 16400
  //EncoderFrameLength=20ms
  //PacketRate= 1/EncoderFrameLength[ms] = 50
  OnOffHelper clientONOFFHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (csmaInterfaces.GetAddress (nCsma - 1), serverPort)));
  clientONOFFHelper.SetAttribute ("PacketSize", UintegerValue (41)); //Bytes
  clientONOFFHelper.SetAttribute ("DataRate", DataRateValue (16400)); //bps
  clientONOFFHelper.SetAttribute ("OnTime", PointerValue (onRv));
  clientONOFFHelper.SetAttribute ("OffTime", PointerValue (offRv));

  NodeContainer clientNodes;
  clientNodes.Add (csmaNodes.Get (0));
  clientNodes.Add (csmaNodes.Get (1));

  ApplicationContainer clientONOFFApps = clientONOFFHelper.Install (clientNodes);
  clientONOFFApps.Start (Seconds (2.0));
  clientONOFFApps.Stop (Seconds (simTime));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  csma.EnablePcap ("gecho-client", clientNodes.Get (0)->GetId (),0);
  csma.EnablePcap ("gecho-client", clientNodes.Get (1)->GetId (),0);
  csma.EnablePcap ("gecho-server", csmaNodes.Get (nCsma - 1)->GetId (),0);

  Simulator::Stop (Seconds (simTime));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
