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
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

int PORT=10;
int k=10;
int NO_CLIENTS=0;
string ip="";
int T=100;
double prb=0.05;
vector<long long> execution_times;
#define Bufferlen 1024
// mutex mtx;

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

    if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0)
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
    
    srand(time(0));

    bool eof=false;
    map<string,int> words;
    int offset=-k;

    // long long current_time_start = chrono::duration_cast<chrono::milliseconds>(
    //         chrono::system_clock::now().time_since_epoch()
    //     ).count();
    // cout<<"entering while eof"<<endl;

    while(!eof)
    {
        offset+=k;
        // cout<<"offset is "<<offset<<endl;
        string modified_message=to_string(offset);
        modified_message+='\n';

        long long current_time_ms = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();

        // int some_rand_num=(rand()%5)+1;
        // int jitter=rand()%(T/10);
        long long wait_time = T - (current_time_ms % T);
        //cout<<"\rsleep time for client :"<<client_id<<" "<<(wait_time)*1000<<endl;
        usleep((wait_time)*1000); //convert milliseconds to microseconds
        //cout<<"\rwoke up for client :"<<client_id<<endl;

        double rand_prob = (double)rand() / RAND_MAX;
        bool message_sent=false;
        if(rand_prob<prb)
        {
            long long current_time_send = chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch()
            ).count();
            //cout<<"sending offset for client:"<<client_id<<" offset:"<<offset<<endl;
            send(sock, modified_message.c_str(), modified_message.length(), 0);
            // cout<<"sending offset of "<<offset<<" for client:"<<client_id<<" at slot "<<(current_time_send-current_time_start)/100<<endl;
            message_sent=true;
        }
        else offset-=k;

        string csv="";bool huh=false;
        while(true && message_sent)
        {
            memset(buffer, 0, sizeof(buffer));
            
            read(sock, buffer, Bufferlen);
            if(string(buffer).find("HUH!")!=string::npos) 
            {
                // cout<<"HUH! received. Retrying in the next slot for client:"<<client_id<<endl;
                // cout<<"and for offset: "<<offset<<endl;
                huh=true;offset-=k;csv="";
                // cout<<"so changing offset to:"<<offset<<endl;
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
        
        if((!huh)&&message_sent)
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
            // cout<<"eofff"<<client_id<<" "<<eof<<endl;
        }
    }
    // cout<<"exited the while main loop for client"<<client_id<<endl;
    words.erase("EOF");
    string filename="output_"+to_string(client_id)+".txt";
    std::ofstream file(filename);
    if(file.is_open())
    {
        // cout<<"file opened for client "<<client_id<<endl;
        for(auto it:words) file<<it.first<<", "<<it.second<<endl;
    }
    // cout<<"done writing into files for client "<<client_id<<endl;
    file.close();
    // Close the socket
    close(sock);
    long long client_end_time = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();

    long long execution_time=client_end_time-client_start_time;
    execution_times[client_id]=execution_time;
    //cout<<"closing socket:"<<client_id<<endl;
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
    prb=(1.0/NO_CLIENTS);
    execution_times.resize(NO_CLIENTS);
    // prb=1;


    // cout<<"ip is "<<ip<<endl;
    // cout<<"PORT is "<<PORT<<endl;
    // cout<<"num clietns is "<<NO_CLIENTS<<endl;
    // cout<<"prb is "<<prb<<endl;


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

    std::ofstream file2("times.txt",std::ios::app);
    long long execution_time=0;
    for(int i=0;i<NO_CLIENTS;i++) execution_time+=execution_times[i];
    if(file2.is_open()) file2<<(execution_time/NO_CLIENTS)<<endl;
    
    // cout<<"finished all clients and execution time "<<execution_time<<endl;
}
