#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleTSN");

struct PacketTrace {
    double sendTime;
    std::string path;
};

std::map<uint32_t, PacketTrace> packetTracker;
std::vector<std::string> nodeNames = {"TSN Client", "Relay1", "BestEffor Node", "Relay2", "Destination"};

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
    
    std::cout << "\nPacket " << p->GetUid() << " sent at time " <<    Simulator::Now().GetSeconds() << "s" << std::endl;
    
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
        
        std::cout << "\nPacket " << p->GetUid() << " received at time " << endTime << "s with Delay of :"<< packetDelay << " s " << " | Jitter: " << jitter << " s" << std::endl;
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
}




int main(int argc, char *argv[]) {

    Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents",BooleanValue(true));

    double simulationTime = 300;  
    double maxPackets = 10;
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
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("25ms"));

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

    // UdpEchoServer on Destination (Node 4)
    uint16_t echoPort = 9;
    UdpEchoServerHelper echoServer(echoPort);

    ApplicationContainer serverApp = echoServer.Install(nodes.Get(4));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(20.0));

    // TSN Client 
    UdpEchoClientHelper tsnClient(interface34.GetAddress(1), echoPort);
    tsnClient.SetAttribute("MaxPackets", UintegerValue(5));
    tsnClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); 
    tsnClient.SetAttribute("PacketSize", UintegerValue(64));

    ApplicationContainer tsnApp = tsnClient.Install(nodes.Get(0));
    tsnApp.Start(Seconds(2.0));
    tsnApp.Stop(Seconds(20.0));

    // Best-Effort Client 
    UdpEchoClientHelper beClient(interface34.GetAddress(1), echoPort);
    beClient.SetAttribute("MaxPackets", UintegerValue(5));
    beClient.SetAttribute("Interval", TimeValue(Seconds(1.5))); 
    beClient.SetAttribute("PacketSize", UintegerValue(64));

    ApplicationContainer beApp = beClient.Install(nodes.Get(2));
    beApp.Start(Seconds(2.0));
    beApp.Stop(Seconds(20.0));

   
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
    AnimationInterface anim("TSN.xml");
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

    
    pointToPoint.EnablePcapAll("TSN_packet_trace");
    
    Simulator::Schedule(Seconds(simulationTime), &Ratio);

    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}
