/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int const nodesIDnb(4);
int nodesID[nodesIDnb];   //Déclaration du tableau

void Monitor (std::string context, Ptr<const Packet> pkt, uint16_t channel, WifiTxVector tx, MpduInfo mpdu, SignalNoiseDbm sn)
{

    int n = sizeof(nodesID)/sizeof(nodesID[0]);

    int elem = pkt->GetUid();

    int i = 0;
    while (i < n)
    {
      if (nodesID[i] == elem) {
        break;
      }
      i++;
    }
    std::cout << context << std::endl;
    std::cout << "\tChannel : " << channel << " pkt: " << pkt->GetUid() << "\t Signal=" << sn.signal << "\tNoise=" << sn.noise << " from node : " << i << std::endl;

}

void MonitorTx (std::string context, Ptr<const Packet> pkt, uint16_t channel, WifiTxVector tx, MpduInfo mpdu)
{

    std::string nodeid = context.substr (10,1);
    std::string::size_type sz;   // alias of size_t
    int i_dec = std::stoi (nodeid,&sz);
    if(i_dec < 5){
      nodesID [i_dec] = pkt->GetUid();
    }
    std::cout << "\tChannel : " << channel << " pkt: " << pkt->GetUid() << std::endl;

}

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 3;
  bool tracing = false;

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer wifiApNode;
  wifiApNode.Create (4);


  NodeContainer wifiStaNode;
  wifiStaNode.Create (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevice;
  staDevice = wifi.Install (phy, mac, wifiStaNode);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (2.0),
                                 "MinY", DoubleValue (2.0),
                                 "GridWidth", UintegerValue (40),
                                 "LayoutType", StringValue ("RowFirst"));

  //mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (wifiStaNode);

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (40),
                                 "LayoutType", StringValue ("RowFirst"));


  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNode);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevice);
  Ipv4InterfaceContainer apDeviceInterfaces = address.Assign (apDevices);

  /*UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (wifiApNode);
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (apDeviceInterfaces.GetAddress(0),9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
    echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();**/

  Simulator::Stop (Seconds (1));

  Config::Connect ("/NodeList/4/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback(&Monitor));
  Config::Connect ("/NodeList/0/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback(&MonitorTx));
  Config::Connect ("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback(&MonitorTx));
  Config::Connect ("/NodeList/2/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback(&MonitorTx));
  Config::Connect ("/NodeList/3/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback(&MonitorTx));

  AnimationInterface anim ("animation.xml");

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
