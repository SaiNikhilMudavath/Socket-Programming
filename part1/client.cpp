#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <map>
#include <algorithm>
#include <fstream>
#include <chrono>
#include "json.hpp"
using namespace std;

#define Bufferlen 1024
int k=0;
int PORT=0;
using json = nlohmann::json;


bool check_k_comma(string csv,int req)
{
    int count=0;
    for(int i=0;i<csv.size();i++) if(csv[i]==',' || csv[i]=='\n') count++;
    // file<<"count is:"<<count<<endl;
    return (count==req);
}
int main() {
    std::ifstream filejson("config.json");
    if (!filejson.is_open())
    {
        cerr<<"Unable to open config.json file!"<<endl;
        return 0;
    }
    json j;
    filejson >> j;
    k=j["k"];
    string ip=j["server_ip"];
    PORT=j["server_port"];

    // cout<<"client started"<<endl;
    long long start_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();

    int sock=0;
    struct sockaddr_in serv_addr;
    char buffer[Bufferlen]={0};

    if((sock=socket(AF_INET,SOCK_STREAM,0))< 0)
    {
        cerr<<"Socket creation failed!"<<endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET,ip.c_str(),&serv_addr.sin_addr)<=0)
    {
        cerr<<"Invalid address or address not supported!"<<endl;
        return -1;
    }

    while(true)
    {
        if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        usleep(100*1000);
        }
        else break;
    }

    bool eof=false;
    map<string,int> words;
    int offset=-k;
    while(!eof)
    {
        offset+=k;
        // cout<<"sending offset "<<offset<<endl;
        std::string modified_message = std::to_string(offset);
        modified_message+='\n';

        send(sock, modified_message.c_str(), modified_message.length(), 0);
        // std::cout << "Message sent to server" << std::endl;

        string csv="";
        while(true)
        {
            memset(buffer, 0, sizeof(buffer));
  
            read(sock, buffer, Bufferlen);
            // cout<<"Message from server: "<<buffer;
            // cout<<(strcmp(buffer,"$$\n"))<<endl;
            if(strcmp(buffer,"$$\n")==0)
            {
                eof=true;csv="";break;
            }
            csv+=buffer;
            if(csv.find("EOF")!=string::npos) break;
            if(check_k_comma(csv,k)) break;
        }

        // cout<<csv<<endl;
        string curr="";
        for(int i=0;i<csv.size();i++)
        {
            if(csv[i]==',' || csv[i]=='\n')
            {
                //cout<<curr<<endl;
                words[curr]++;curr="";
            }
            else curr+=csv[i];
        }
        if(words["EOF"]>0) eof=true;
    }
    words.erase("EOF");
    std::ofstream file("output.txt");
    if(file.is_open())
    {
        for(auto it:words) file<<it.first<<", "<<it.second<<endl;
    }

    long long end_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();

    long long execution_time=(end_time-start_time);
    std::ofstream file2("times.txt",std::ios::app);
    if(file2.is_open()) file2<<execution_time<<endl;
    // cout<<"client closed"<<endl;
    // cout<<"execution time is "<<execution_time<<endl;
    close(sock);
}


