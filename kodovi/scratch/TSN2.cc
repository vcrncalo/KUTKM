#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/seq-ts-header.h"
#include <iostream>
#include <string>
#include <vector>

#include "ns3/tsn-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleTSN");

struct PacketTrace {
    double sendTime;
    std::string path;
};

std::map<uint32_t, PacketTrace> packetTracker;
std::vector<std::string> nodeNames = {"TSN Client", "Relay1", "BestEffor Node", "Relay2", "Destination"};

static uint32_t g_sequenceNumber = 0;
static std::map<uint32_t, uint32_t> uidToSeqMap;
static std::map<uint32_t, std::string> uidToTrafficType;

static Time g_firstPacketTime = Seconds(0.0);
static Time g_lastPacketTime = Seconds(0.0);
static bool g_firstPacket = true;

static std::map<uint32_t, double> PacketStartTimes;
static double totalDelay = 0.0;
static int packetCount = 0; 

uint32_t m_bytes_sent = 0;
uint32_t m_bytes_received = 0;

uint32_t m_packets_sent = 0;
uint32_t m_packets_received = 0;

//Create help variable m_time
double m_time = 0;

double lastDelay = 0.0;
double jitter = 0.0;

//TSN stats
double tsnDelay = 0.0;
int tsnCount = 0;
double beDelay = 0.0;
int beCount = 0;

//Create c++ map for measuring delay time
std::map<uint32_t, double> m_delayTable;


static void 
SentPacket(Ptr<const Packet> p) {
    m_bytes_sent += p->GetSize();
    m_packets_sent++;
    
    
    if (g_firstPacket) {
    	g_firstPacketTime = Simulator::Now();
    	g_firstPacket = false;
    }
    
    g_lastPacketTime = Simulator::Now();
    PacketStartTimes[p->GetUid()] = Simulator::Now().GetSeconds();
    uidToSeqMap[p->GetUid()] = g_sequenceNumber++;
    //TSN uslov
    	if (Simulator::GetContext() == 0) {
        uidToTrafficType[p->GetUid()] = "TSN";
    }   
    	else if (Simulator::GetContext() == 2) {
        uidToTrafficType[p->GetUid()] = "BE";
    }

    std::cout << "\nPacket " << uidToSeqMap[p->GetUid()] << " sent at time " << Simulator::Now().GetSeconds() << "s" << std::endl;
    
}


static void
ReceivedPacket(Ptr<const Packet> p) {
    m_bytes_received += p->GetSize();
    m_packets_received++;
    
    /*
    //HELP LINES USED FOR TESTING
    std::cout << "\n ..................ReceivedPacket....." << p->GetUid() << "..." <<  p->GetSize() << ".......  \n";
    p->Print(std::cout);
    std::cout << "\n ............................................  \n";
    */
	
        double endTime = Simulator::Now().GetSeconds();
        double startTime = PacketStartTimes[p->GetUid()];
        double packetDelay = endTime - startTime;
        
        double delayDifference = std::abs(packetDelay - lastDelay);
        jitter += (delayDifference - jitter) / 16.0;
        lastDelay = packetDelay;
         
        
        totalDelay += packetDelay;
        packetCount++;
        
        uint32_t seqNum = uidToSeqMap[p->GetUid()];
        std::string trafficType = uidToTrafficType[p->GetUid()];
	//TSN uslov
	if (trafficType == "TSN") {
    	 tsnDelay += packetDelay;
   	 tsnCount++;
	} 
	else if (trafficType == "BE") {
    	 beDelay += packetDelay;
    	 beCount++;
}



    std::cout << "\nPacket " << seqNum << " received at time " << endTime << "s with Delay of :" << packetDelay << " s  | Jitter: " << jitter << " s" << std::endl;
  }
        
void
Ratio() {
    std::cout << "\n=== TSN Statistics ===\n" << std::endl;
    std::cout << "Transmission Summary : " << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "Total Bytes Sent:\t" << m_bytes_sent << std::endl;
    std::cout << "Total Bytes Received:\t" << m_bytes_received << std::endl;
    std::cout << "Total Packets Sent:\t" << m_packets_sent << std::endl;
    std::cout << "Total Packets Received:\t" << m_packets_received << std::endl;
    std::cout << "Delivery Ratio (bytes):\t" 
              << (float)m_bytes_received/(float)m_bytes_sent * 100 << "%" << std::endl;
    std::cout << "Delivery Ratio (packets):\t" 
              << (float)m_packets_received/(float)m_packets_sent * 100 << "%" << std::endl;
              
    
    double duration = Simulator::Now().GetSeconds();
    if (duration > 0){
    	double troughputBps = (m_bytes_received * 8.0) / duration ;
    	
    	std::cout << "Troughput (bps) :\t " << troughputBps << " bps " << std::endl;
    	std::cout << "Troughput (kbps) :\t " << troughputBps/1000.0 << " kbps " << std::endl;
    }
    
              
    if (packetCount > 0) {
       
       std::cout << "Average End - to - End Delay :\t" << totalDelay/packetCount << "s" << std::endl; 
     }
     //TSN ispis
     if (tsnCount > 0)
      std::cout << "Avg TSN Delay:\t" << tsnDelay / tsnCount << " s" << std::endl;
     if (beCount > 0)
      std::cout << "Avg BE Delay:\t" << beDelay / beCount << " s" << std::endl;
          
}


Time callbackfunc() {
    return Simulator::Now(); // Used by the TAS scheduler
}

//TSN paketi 
int32_t ipv4PacketFilter(Ptr<QueueDiscItem> item) {
    Ipv4Header ipHeader;
    item->GetPacket()->PeekHeader(ipHeader);

    Ipv4Address srcAddr = ipHeader.GetSource();

    if (srcAddr == "10.1.1.1") { // TSN Client 
        return 4; // Viši prioritet
    } else if (srcAddr == "10.1.2.1") { // BestEffort 
        return 1; // Niži prioritet
    }
    return 0;
}


int main(int argc, char *argv[]) {

    Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents",BooleanValue(true));

    double simulationTime = 300;  
    double maxPackets = 30;
    double packetSize = 128;
 
    Packet::EnablePrinting();
    PacketMetadata::Enable ();  
 
    CommandLine cmd;
    cmd.AddValue ("simulationTime", "simulationTime", simulationTime);
    cmd.AddValue ("maxPackets", "maxPackets", maxPackets);
    cmd.AddValue ("PacketSize", "PacketSize", packetSize);
    cmd.Parse (argc, argv);
  
    Time::SetResolution (Time::NS);
    //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
    //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
    LogComponentEnable ("SimpleTSN", LOG_LEVEL_ALL);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("256kbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("20ms"));

    NodeContainer nodes;
    nodes.Create(5); 

    InternetStackHelper stack;
    stack.Install(nodes);

    NetDeviceContainer dev01 = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer dev21 = pointToPoint.Install(nodes.Get(2), nodes.Get(1));
    NetDeviceContainer dev13 = pointToPoint.Install(nodes.Get(1), nodes.Get(3));
    NetDeviceContainer dev34 = pointToPoint.Install(nodes.Get(3), nodes.Get(4));

    // Assign IP addresses
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface01 = address.Assign(dev01);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interface21 = address.Assign(dev21);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interface13 = address.Assign(dev13);

    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer interface34 = address.Assign(dev34);

    // Enable global routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //TAS Scheduling
	TsnHelper tsnHelperClient, tsnHelperServer;
	NetDeviceListConfig schedulePlanClient, schedulePlanServer;

	Time scheduleDuration = Seconds(1);

	// ON/OFF pattern
	for (int i = 0; i < 5; i++) {
    	  schedulePlanClient.Add(scheduleDuration,{0,0,1,1,1,1,0,0});
  	  schedulePlanClient.Add(scheduleDuration,{0,0,0,0,0,0,0,0});
  	  
    	  schedulePlanServer.Add(scheduleDuration,{0,0,0,0,0,0,0,0});
  	  schedulePlanServer.Add(scheduleDuration,{1,1,1,1,1,1,1,1});
  	  
	}

	// Link: Node 0 (TSN client) Node 1 (Relay1)
	tsnHelperClient.SetRootQueueDisc("ns3::TasQueueDisc",
    	"NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanClient),
    	"TimeSource", CallbackValue(MakeCallback(&callbackfunc)),
    	"DataRate", StringValue("5Mbps"));

	tsnHelperClient.AddPacketFilter(0, "ns3::TsnIpv4PacketFilter", "Classify",
    	CallbackValue(MakeCallback(&ipv4PacketFilter)));

	tsnHelperServer.SetRootQueueDisc("ns3::TasQueueDisc",
	"NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanServer),
	"TimeSource", CallbackValue(MakeCallback(&callbackfunc)),
    	"DataRate", StringValue("5Mbps"));

    // UdpEchoServer on Destination (Node 4)
    uint16_t echoPort = 9;
    UdpEchoServerHelper echoServer(echoPort);

    ApplicationContainer serverApp = echoServer.Install(nodes.Get(4));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(20.0));

    // TSN Client 
    UdpEchoClientHelper tsnClient(interface34.GetAddress(1), echoPort);
    tsnClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
    tsnClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); 
    tsnClient.SetAttribute("PacketSize", UintegerValue(64));

    ApplicationContainer tsnApp = tsnClient.Install(nodes.Get(0));
    tsnApp.Start(Seconds(1.0));
    tsnApp.Stop(Seconds(30.0));

    // Best-Effort Client 
    UdpEchoClientHelper beClient(interface34.GetAddress(1), echoPort);
    beClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
    beClient.SetAttribute("Interval", TimeValue(Seconds(100))); 
    beClient.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer beApp = beClient.Install(nodes.Get(2));
    beApp.Start(Seconds(1.0));
    beApp.Stop(Seconds(30.0));

   
    Config::ConnectWithoutContext("/NodeList/0/ApplicationList/*/$ns3::UdpEchoClient/Tx", MakeCallback(&SentPacket));
    Config::ConnectWithoutContext("/NodeList/2/ApplicationList/*/$ns3::UdpEchoClient/Tx", MakeCallback(&SentPacket));
    Config::ConnectWithoutContext("/NodeList/4/ApplicationList/*/$ns3::UdpEchoServer/Rx", MakeCallback(&ReceivedPacket));


    // Mobility Setup
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                              "MinX", DoubleValue(50.0), 
                              "MinY", DoubleValue(80.0),  
                              "DeltaX", DoubleValue(60.0), 
                              "DeltaY", DoubleValue(70.0),
                              "GridWidth", UintegerValue(4),
                              "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Set custom positions
    Ptr<MobilityModel> mob0 = nodes.Get(0)->GetObject<MobilityModel>(); // TSN Client
    mob0->SetPosition(Vector(0.0, 50.0, 0.0)); 

    Ptr<MobilityModel> mob1 = nodes.Get(1)->GetObject<MobilityModel>(); // Relay 1
    mob1->SetPosition(Vector(100.0, 50.0, 0.0)); 

    Ptr<MobilityModel> mob2 = nodes.Get(2)->GetObject<MobilityModel>(); // BestEffort Node
    mob2->SetPosition(Vector(100.0, 100.0, 0.0)); 

    Ptr<MobilityModel> mob3 = nodes.Get(3)->GetObject<MobilityModel>(); // Relay 2
    mob3->SetPosition(Vector(200.0, 50.0, 0.0)); 

    Ptr<MobilityModel> mob4 = nodes.Get(4)->GetObject<MobilityModel>(); // Destination
    mob4->SetPosition(Vector(300.0, 50.0, 0.0)); 

    // NetAnim
    AnimationInterface anim("TSN2.xml");
    anim.SetMaxPktsPerTraceFile(5000);
    
    anim.UpdateNodeDescription(0, "TSN Client"); 
    anim.UpdateNodeDescription(1, "Relay 1");	
    anim.UpdateNodeDescription(2, "BestEffort Node"); 
    anim.UpdateNodeDescription(3, "Relay 2"); 
    anim.UpdateNodeDescription(4, "Destination"); 
    
    anim.UpdateNodeColor(0, 255, 0, 0); // Red for TSN Client
    anim.UpdateNodeColor(1, 0, 0, 255); // Blue for Relay 1
    anim.UpdateNodeColor(2, 0, 255, 0); // Green for Best Effor Node
    anim.UpdateNodeColor(3, 255, 255, 0); // Yellow for Relay 2
    anim.UpdateNodeColor(4, 128, 128, 128); // Gray for Destination
    
    /*  anim.UpdateNodeColor(0, 255, 0, 0); // Red for TSN Client
    	anim.UpdateNodeColor(2, 0, 0, 255); // Blue for Relay 1
    	anim.UpdateNodeColor(1, 0, 255, 0); // Green for Best Effor Node
    	anim.UpdateNodeColor(3, 255, 255, 0); // Yellow for Relay 2
    	anim.UpdateNodeColor(4, 128, 128, 128); // Gray for Destination
    	STARE VRIJEDOSTI AKO SE STA POKVARI 	*/
    //;

    
    pointToPoint.EnablePcapAll("TSN2_packet_trace");
    
    Simulator::Schedule(Seconds(simulationTime), &Ratio);

    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}
