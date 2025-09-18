#include <bits/stdc++.h>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>  
#include <mutex>  
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

// #define PORT 8000
#define Bufferlen 1024
int num_clients=0;
int original_clients=32;

mutex mtx;
struct ClientArgs {
    int new_socket;
    int k;
    int p;
    vector<string>* v; 
};
int number_clients(string action)
{
    lock_guard<mutex> lock(mtx);
    // mtx.lock();
    int c=0;
    if(action=="inc")
    {
        num_clients++;
    }
    else if(action=="dec")
    {
        num_clients--;
    }
    else if(action=="show")
    {
        c=num_clients;
        // cout<<num_clients<<endl;
    }
    // mtx.unlock();
    return c;
}

vector<string> read_text(string name)
{
    std::ifstream file(name);

    if (!file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return {};
    }

    std::vector<std::string> words;
    std::string line;

    std::getline(file, line);

    std::stringstream ss(line);
    std::string word;

    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }
    words.push_back("EOF");
    file.close();
    return words;
}

void* handle_client(void* arg)
{
    ClientArgs* args = (ClientArgs*)arg;
    int new_socket = args->new_socket;
    int k = args->k;
    int p = args->p;
    vector<string> v = *(args->v);  
    delete args;  

    char buffer[Bufferlen] = {0};
    int num_packets = (k % p) ? (k / p + 1) : (k / p);

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(new_socket, buffer, Bufferlen);
        if (valread <= 0)
        {
            // cout << "Client disconnected!" << endl;
            break;
        }

        // cout << "Message from client: " << buffer << endl;

        string off(buffer);
        off = off.substr(0, off.find('/'));
        int offset = stoi(off);
        // cout << "Offset: " << offset << endl;

        if (offset >= v.size())
        {
            string res = "$$\n";
            send(new_socket, res.c_str(), res.length(), 0);
            // cout << "Response sent to client: " << res << endl;
            break;
        }

        int number = 0;
        for (int chari = 0;(chari<num_packets)&&(offset<v.size()); chari++)
        {
            string res = "";
            int loop_number = 0;
            while (offset < v.size() && loop_number < p && number < k)
            {
                res += v[offset];
                res += ",";
                offset++;
                loop_number++;
                number++;
            }
            if (!res.empty())
            {
                res.pop_back();
                res += '\n';
                send(new_socket, res.c_str(), res.length(), 0);
                // cout << "Response sent to client: " << res << endl;
            }
        }
    }
    // cout<<"ready to disconnect!..."<<endl;
    // number_clients("dec");
    close(new_socket);
    pthread_exit(NULL);
    // cout<<"disconnected"<<endl;
    return NULL;
}

int main()
{
    ifstream json_file("config.json");
    
    if (!json_file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return 1;
    }

    json j;
    json_file >> j;
    int k=j["k"];
    int p=j["p"];
    int PORT=j["server_port"];
    int num_packets=k/p;
    if(k%p) num_packets++;
    original_clients=j["num_clients"];
    vector<string>v=read_text(j["input_file"]);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cerr << "Socket creation failed!" << endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        cerr << "Bind failed!||" << endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0)
    {
        cerr << "Listen failed!" << endl;
        close(server_fd);
        return -1;
    }

    vector<pthread_t> client_threads;

    for(int i=0;i<original_clients;i++)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            cerr << "Accept failed!" << endl;
            close(server_fd);
            return -1;
        }

        // cout << "Connection established with a client!" << endl;

        ClientArgs* args = new ClientArgs();
        args->new_socket = new_socket;  
        args->k = k;                    
        args->p = p;                    
        args->v = &v;     

        pthread_t client_thread;
        // number_clients("inc");
        pthread_create(&client_thread, NULL, handle_client, (void*)args);
        // pthread_join(client_thread, NULL);
        client_threads.push_back(client_thread);
    }
    for (pthread_t& t : client_threads) {
        pthread_join(t, NULL);  
    }
    // cout<<"server closed..!"<<endl;
    close(server_fd);
    return 0;
}