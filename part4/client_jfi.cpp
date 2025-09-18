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
#include <mutex>
using namespace std;
using json = nlohmann::json;
long long sum=0;
long long square=0;
mutex mtx;

int PORT=10;
int k=10;
int NO_CLIENTS=0;
string ip="";
#define Bufferlen 1024
// vector<long long>times;
void change_timer(long long time)
{
    mtx.lock();
    sum+=time;
    square+=(time*time);
    // times.push_back(time);
    mtx.unlock();
}

bool check_k_comma(string csv,int req)
{
    int count=0;
    for(int i=0;i<csv.size();i++) if(csv[i]==',' || csv[i]=='\n') count++;
    return (count==req);
}
void* rogue_client(void* arg)
{
    long long start_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    int client_id=*((int*)arg);

    int sock = 0;
    struct sockaddr_in serv_addr;
    std::string message = "Hello from client siddhu roy";
    char buffer[Bufferlen] = {0};
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr <<"Socket creation failed for client: "<<client_id<<endl;
        pthread_exit(nullptr);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0)
    {
        cerr<<"Invalid address or address not supported for client" <<client_id<<endl;
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
        for(int g=0;g<5;g++)
        {
            offset+=k;
            std::string modified_message = std::to_string(offset);
            modified_message+='\n';
            
            // std::cout << "Message sent to server for client:"<<client_id<<" and offset: "<<modified_message << std::endl;

            send(sock, modified_message.c_str(), modified_message.length(), 0);
        }

        string csv="";
        while(true)
        {
            memset(buffer, 0, sizeof(buffer));

            read(sock, buffer, Bufferlen);
            // cout<<"buffer is "<<buffer<<endl;
            csv+=buffer;
            if(csv.find("$$\n")!=string::npos)
            {
                // cout<<"indollar"<<endl;
                eof=true;break;
            }
            
            // cout<<"csv is "<<csv<<endl;
            if(eof) break;
            // cout<<"for client:"<<client_id<<" csv: "<<csv<<endl;
            if(csv.find("EOF")!=string::npos) break;
            if(check_k_comma(csv,5*k)) break;
            // cout<<"didnt break"<<endl;
        }
        string curr="";
        for(int i=0;i<csv.size();i++)
        {
            if(csv[i]==',' || csv[i]=='\n')
            {
                words[curr]++;curr="";
            }
            else curr+=csv[i];
        }
        words.erase("$$");
        if(words["EOF"]>0) eof=true;

    }
    long long end_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    
    change_timer(end_time-start_time);
    words.erase("EOF");
    string filename="output_"+to_string(client_id)+".txt";
    std::ofstream file(filename);
    if(file.is_open())
    {
        for(auto it:words) file<<it.first<<", "<<it.second<<endl;
    }
    file.close();
    // cout<<"closing socket for:"<<client_id<<endl;
    close(sock);
    pthread_exit(nullptr);
}
void* handle_clients(void* arg)
{
    long long start_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    int client_id=*((int*)arg);
    
    int sock = 0;
    struct sockaddr_in serv_addr;
    std::string message = "Hello from client siddhu roy";
    char buffer[Bufferlen] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        cerr<<"Socket creation failed for client: "<<client_id<<endl;
        pthread_exit(nullptr);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0)
    {
        cerr<<"Invalid address or address not supported for client" <<client_id<<endl;
        pthread_exit(nullptr);
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
                // cout<<"indollar for client "<<client_id<<endl;
                eof=true;break;
            }
            csv+=buffer;
            // cout<<"for client:"<<client_id<<" csv: "<<csv<<endl;
            if(csv.find("EOF")!=string::npos) break;
            if(check_k_comma(csv,k)) break;
        }
        
        // cout<<"processing words for client "<<client_id<<endl;
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
    long long end_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    
    change_timer(end_time-start_time);
    words.erase("EOF");
    string filename="output_"+to_string(client_id)+".txt";
    std::ofstream file(filename);
    if(file.is_open())
    {
        for(auto it:words) file<<it.first<<", "<<it.second<<endl;
        file<<words.size()<<endl;
    }
    file.close();
    close(sock);
    pthread_exit(nullptr);
}
int main()
{
    std::ifstream filejson("config.json");
    if (!filejson.is_open())
    {
        cerr<<"Unable to open the config file!"<<endl;
        return 1;
    }
    json j;filejson >> j;
    k=j["k"];
    ip=j["server_ip"];
    string ip=j["server_ip"];
    PORT=j["server_port"];
    NO_CLIENTS=j["num_clients"];
    
    pthread_t threads[NO_CLIENTS];
    
    for(int i=0;i<NO_CLIENTS;i++)
    {
        int* client_id=new int (i);
        if(i==0)
        {
            if(pthread_create(&threads[i],nullptr,rogue_client,(void*)client_id)!=0)
            {
                cerr<<"Failed to create thread for rogue client "<<i<<endl;
            }
        }
        else if(pthread_create(&threads[i],nullptr,handle_clients,(void*)client_id)!=0)
        {
            cerr<<"Failed to create thread for client "<<i<<endl;
        }
    }
    
    long long start_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    
    for(int i=0;i<NO_CLIENTS;i++)
    {
        pthread_join(threads[i],nullptr);
    }
    long long end_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    
    long long execution_time=end_time-start_time;

    std::ofstream file2("times.txt",std::ios::app);
    double jfi=static_cast<double>(sum*sum)/static_cast<double>(NO_CLIENTS*square);
    cout<<"average running time "<<(sum/NO_CLIENTS)<<", JFI is "<<jfi<<endl;
    if(file2.is_open()) file2<<((execution_time)/NO_CLIENTS)<<endl;
    
}

