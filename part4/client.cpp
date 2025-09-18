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
#include <pthread.h>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

int PORT=10;
int k=10;
int NO_CLIENTS=0;
string ip="";
vector<long long> execution_times;
#define Bufferlen 1024

bool check_k_comma(string csv,int req)
{
    int count=0;
    for(int i=0;i<csv.size();i++) if(csv[i]==',' || csv[i]=='\n') count++;
    return (count==req);
}
void* handle_clients(void* arg)
{
    int client_id=*((int*)arg);

    long long client_start_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();

    int sock = 0;
    
    struct sockaddr_in serv_addr;
    char buffer[Bufferlen] = {0};
    if((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        cerr << "Socket creation failed for client: "<<client_id<<endl;
        pthread_exit(nullptr);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr<<"Invalid address or address not supported for client"<<client_id<<endl;
        pthread_exit(nullptr);
    }

    while(true)
    {
        if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        // std::cerr << "Connection failed!!!" << std::endl;
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
        std::string modified_message = std::to_string(offset);
        modified_message+='\n';

        send(sock, modified_message.c_str(), modified_message.length(), 0);
        // std::cout << "Message sent to server for client:"<<client_id<<" and offset: "<<modified_message << std::endl;

        string csv="";
        while(true)
        {
            memset(buffer, 0, sizeof(buffer));

            read(sock, buffer, Bufferlen);
            // cout<<"Message from server for client: "<<client_id<<" "<<buffer;
            //cout<<(strcmp(buffer,"$$\n"))<<endl;
            if(strcmp(buffer,"$$\n")==0)
            {
                // cout<<"indollar"<<endl;
                eof=true;csv="";break;
            }
            csv+=buffer;
            // cout<<"for client:"<<client_id<<" csv: "<<csv<<endl;
            if(csv.find("EOF")!=string::npos) break;
            if(check_k_comma(csv,k)) break;
        }
    
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
    string filename="output_"+to_string(client_id)+".txt";
    std::ofstream file(filename);
    if(file.is_open())
    {
        for(auto it:words) file<<it.first<<", "<<it.second<<endl;
    }
    file.close();
    close(sock);

    long long client_end_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    long long execution_time=client_end_time-client_start_time;
    execution_times[client_id]=execution_time;

    pthread_exit(nullptr);
}
int main()
{
    std::ifstream filejson("config.json");
    if (!filejson.is_open())
    {
        cerr << "Unable to open the config file!"<<endl;
        return 1;
    }
    json j;filejson >> j;
    k=j["k"];
    ip=j["server_ip"];
    string ip=j["server_ip"];
    PORT=j["server_port"];
    NO_CLIENTS=j["num_clients"];
    execution_times.resize(NO_CLIENTS);
    
    pthread_t threads[NO_CLIENTS];
    
    for(int i=0;i<NO_CLIENTS;i++)
    {
        int* client_id=new int (i);
        if(pthread_create(&threads[i],nullptr,handle_clients,(void*)client_id)!=0)
        {
            std::cerr<<"Failed to create thread for client "<<i<<endl;
        }
    }
    for(int i=0;i<NO_CLIENTS;i++)
    {
        pthread_join(threads[i],nullptr);
    }
    
    long long execution_time=0;
    for(int i=0;i<NO_CLIENTS;i++) execution_time+=execution_times[i];
    std::ofstream file2("times.txt",std::ios::app);
    if(file2.is_open()) file2<<((execution_time)/NO_CLIENTS)<<endl;

    // cout<<"finished all clients with execution time "<<execution_time<<endl;
}

