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
#include <ns3/buildings-module.h>
#include <ns3/buildings-helper.h>
#include <ns3/hybrid-buildings-propagation-loss-model.h>

#include <iostream>
#include <fstream>

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
int nodesID[nodesIDnb] = {-1, -1, -1, -1};   //Déclaration du tableau

int const nodesRSSInb(4);
int nodesRSSI[nodesRSSInb] = {-1, -1, -1, -1};   //Déclaration du tableau

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

    
    // std::cout << context << std::endl;
    std::cout << "\tChannel : " << channel << " pkt: " << pkt->GetUid() << "\t Signal=" << sn.signal << "\tNoise=" << sn.noise << " from node : " << i << std::endl;

    nodesRSSI[i] = sn.signal;
    
}

void MonitorTx (std::string context, Ptr<const Packet> pkt, uint16_t channel, WifiTxVector tx, MpduInfo mpdu)
{
    std::string nodeid = context.substr (10,1);
    std::string::size_type sz;   // alias of size_t
    int i_dec = std::stoi (nodeid,&sz);
    nodesID [i_dec] = pkt->GetUid();
}

static void 
 CourseChange (std::string context, Ptr<const MobilityModel> mobility)
 {
    std::ofstream positions;
    std::ofstream rssi;
    positions.open ("positions.txt", std::fstream::app);
    rssi.open ("rssi.txt", std::fstream::app);
  
   int n = sizeof(nodesRSSI)/sizeof(nodesRSSI[0]);
   
   for(int a = 0; a < n; a++){
    if (nodesRSSI[a] == -1){
      return;
    }
    rssi << nodesRSSI[a] << " ";
   }
   rssi << std::endl;
   positions << trunc(mobility->GetPosition().x) << " " << trunc(mobility->GetPosition().y) << std::endl;
   rssi.close();
   positions.close();
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

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("25.0"),
                                  "Y", StringValue ("25.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=30]"));
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                              "Mode", StringValue ("Time"),
                              "Time", StringValue ("2s"),
                              "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=5.0]"),
                              "Bounds", StringValue ("0|50|0|50"));


  mobility.Install (wifiStaNode);

  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNode);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevice);
  Ipv4InterfaceContainer apDeviceInterfaces = address.Assign (apDevices);

  Ptr<ConstantPositionMobilityModel> mm0 = wifiApNode.Get (0)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> mm1 = wifiApNode.Get (1)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> mm2 = wifiApNode.Get (2)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> mm3 = wifiApNode.Get (3)->GetObject<ConstantPositionMobilityModel> ();
  //Ptr<ConstantPositionMobilityModel> mmClient = wifiStaNode.Get (0)->GetObject<ConstantPositionMobilityModel> ();
  mm0->SetPosition (Vector (10.0, 15.0, 1.5));
  mm1->SetPosition (Vector (35.0, 30.0, 1.5));
  mm2->SetPosition (Vector (45.0, 15.0, 1.5));
  mm3->SetPosition (Vector (15.0, 40.0, 1.5));
  //mmClient->SetPosition(Vector(25.0,25.0,1.5));

  double x_min = 0.0;
  double x_max = 50.0;
  double y_min = 0.0;
  double y_max = 50.0;
  double z_min = 0.0;
  double z_max = 10.0;

  Ptr<Building> b = CreateObject<Building> ();

  b->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
  b->SetBuildingType (Building::Residential);
  b->SetExtWallsType (Building::ConcreteWithWindows);
  b->SetNFloors (1);
  b->SetNRoomsX (2);
  b->SetNRoomsY (2);

  BuildingsHelper::Install(wifiApNode);
  BuildingsHelper::Install(wifiStaNode);

  Simulator::Stop (Seconds (30));

  Config::Connect ("/NodeList/4/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback(&Monitor));
  Config::Connect ("/NodeList/0/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback(&MonitorTx));
  Config::Connect ("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback(&MonitorTx));
  Config::Connect ("/NodeList/2/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback(&MonitorTx));
  Config::Connect ("/NodeList/3/DeviceList/0/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback(&MonitorTx));
  Config::Connect ("/NodeList/4/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));

  AnimationInterface anim ("animation.xml");
  anim.SetBackgroundImage ("images.png", 0, 0, 0, 0, 0);
  anim.UpdateNodeColor (4, 0, 0, 255);
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
