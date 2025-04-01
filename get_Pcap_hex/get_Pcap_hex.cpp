#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct PcapHeader {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
};

struct PacketHeader {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t caplen;
    uint32_t len;
};

//C:\\Program_Code\\C++\\project_Tshark\\try_cat.pcap
//C: / Reverse_tools / Wireshark / http_traffic.pcap

int main()
{
    ifstream file("C:\\Program_Code\\C++\\project_Tshark\\try_cat.pcap", ios::binary);
    if (!file) {
        cerr << "無法打開檔案" << endl;
        return 0;
    }

    PcapHeader pcapHeader;
    file.read(reinterpret_cast<char*>(&pcapHeader), sizeof(PcapHeader));
    while (file) {

        PacketHeader packetHeader;
        file.read(reinterpret_cast<char*>(&packetHeader), sizeof(PacketHeader));

        if (!file) break;

        vector<unsigned char> data(packetHeader.caplen);
        file.read(reinterpret_cast<char*>(data.data()), packetHeader.caplen);

        printf("數據包[時間：%d 長度：%d]：", packetHeader.ts_sec, packetHeader.caplen);
        int i = 0;
        for (unsigned char byte : data) {
            if (i > 100) break;
            printf("%02X", byte);
            i++;
        }
        cout << "\n";
    }
    file.close();
    return 0;
    
}

