#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <pthread.h>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <mutex>
#include <algorithm>
#include "json.hpp"
using namespace std;
using json=nlohmann::json;

int PORT=10;
int k=10;
int NO_CLIENTS=0;
string ip="";
int T=100;
double prb=0.05;
vector<long long> execution_times;
#define Bufferlen 1024
// mutex mtx;

long long power(int a,int b)
{
    if(b==0) return 1;
    else if(b%2==0) return power(a*a,b/2);
    else return a*power(a,b-1);
}
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
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "Socket creation failed for client: "<<client_id<<endl;
        pthread_exit(nullptr);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        cerr << "Invalid address or address not supported for client"<<client_id<<endl;
        pthread_exit(nullptr);
    }
    
    while(true)
    {
        if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            usleep(100*1000);
        }
        else break;
    }
    // cout<<"starting for client "<<client_id<<endl;
    srand(time(0));

    bool eof=false;
    map<string,int> words;
    int offset=-k;
    
    int attempt=0;
    while(!eof)
    {
        //binary exponential backoff
        int some_rand_num=0;
        if(attempt==0) some_rand_num=0;
        else if(attempt==1) some_rand_num=1;
        else some_rand_num=(rand()%(power(2,attempt)-1))+1;
        
        long long wait_time = some_rand_num*T;
        usleep((wait_time)*1000); //convert milliseconds to microseconds

        //sensing
        string control_message="BUSY?\n";
        while(true)
        {
            send(sock, control_message.c_str(), control_message.length(), 0);
            memset(buffer, 0, sizeof(buffer));
            read(sock, buffer, Bufferlen);
            if(strcmp(buffer,"IDLE\n")==0) break;
            usleep((T)*1000);
            // cout<<"waiting for client for idle: "<<client_id<<endl;
        }

        //sending the offset
        offset+=k;
        string modified_message=to_string(offset);
        modified_message+='\n';
        send(sock, modified_message.c_str(), modified_message.length(), 0);

        string csv="";bool huh=false;
        while(true)
        {
            memset(buffer, 0, sizeof(buffer));
            read(sock, buffer, Bufferlen);
            if(string(buffer).find("HUH!")!=string::npos) 
            {
                huh=true;offset-=k;csv="";
                attempt++;
                // cout<<"received huh for client "<<client_id<<endl;
                break;
            }
            if(strcmp(buffer,"$$\n")==0)
            {
                eof=true;csv="";break;
            }
            csv+=buffer;
            if(csv.find("EOF")!=string::npos) break;
            if(check_k_comma(csv,k)) break;
        }
        
        if((!huh))
        {
            // lock_guard<mutex> lock(mtx);
            string curr="";
            for(int i=0;i<csv.size();i++)
            {
                if(csv[i]==',' || csv[i]=='\n')
                {
                    words[curr]++;curr="";
                }
                else curr+=csv[i];
            }
            if(csv.find("EOF")!=string::npos) eof=true;
            if(words["EOF"]>0) eof=true;
            attempt=0;
        }
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
    // cout<<"closing socket:"<<client_id<<endl;
    pthread_exit(nullptr);
}
int main()
{
    std::ifstream filejson("config.json");
    if (!filejson.is_open())
    {
        cerr << "Unable to open the config file!" <<endl;
        return 1;
    }
    json j;filejson >> j;
    k=j["k"];
    ip=j["server_ip"];
    string ip=j["server_ip"];
    PORT=j["server_port"];
    NO_CLIENTS=j["num_clients"];
    prb=(1.0/NO_CLIENTS);
    execution_times.resize(NO_CLIENTS);
    
    pthread_t threads[NO_CLIENTS];
    for(int i=0;i<NO_CLIENTS;i++)
    {
        int* client_id=new int (i);
        if(pthread_create(&threads[i],nullptr,handle_clients,(void*)client_id)!=0)
        {
            cerr<<"Failed to create thread for client "<<i<<endl;
        }
    }
    for(int i=0;i<NO_CLIENTS;i++)
    {
        pthread_join(threads[i],nullptr);
    }

    long long execution_time=0;
    
    std::ofstream file2("times.txt",std::ios::app);
    for(int i=0;i<NO_CLIENTS;i++) execution_time+=execution_times[i];
    if(file2.is_open()) file2<<(execution_time/NO_CLIENTS)<<endl;

    cout<<"finished all clients and execution time "<<execution_time<<endl;
}
