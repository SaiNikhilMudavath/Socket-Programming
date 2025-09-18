#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <random>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;

// #define PORT 8080
#define Bufferlen 1024
int num_clients=0;
int original_clients=32;
int k = 10;
int p = 3;
vector<string>v={};

mutex mtx;    
struct ClientArgs {
    int new_socket;
};

int active_clients = 0;    
int current_clients=0;

void increment_active_clients() {
    lock_guard<mutex> lock(mtx);  
    active_clients++;
    current_clients++;
}

void decrement_active_clients() {
    // lock_guard<mutex> lock(mtx);  
    current_clients--;
    if(current_clients==0)
    {
        active_clients=0;
    }
}

int get_active_clients() {
    // lock_guard<mutex> lock(mtx);  
    return active_clients;  
}

int clients(string action)
{
    lock_guard<mutex> lock(mtx);
    // mtx.lock();
    int c=0;
    if(action=="inc")
    {
        active_clients++;
        current_clients++;
    }
    else if(action=="dec")
    {
        current_clients--;
        if(current_clients==0)
        {
            active_clients=0;
        }
    }
    else if(action=="show")
    {
        c=active_clients;
        // cout<<num_clients<<endl;
    }
    return c;
}

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
    ifstream file(name);

    if (!file.is_open()) 
    {
        cerr << "Could not open the file!" << endl;
        return {};
    }

    vector<string> words;
    string line;

    getline(file, line);

    stringstream ss(line);
    string word;

    while (getline(ss, word, ',')) 
    {
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
    delete args;
    //cout<<"entered for this client: "<<new_socket<<endl;

    char buffer[Bufferlen] = {0};
    int num_packets = (k % p) ? (k / p + 1) : (k / p);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(new_socket, buffer, Bufferlen);
        
        if (valread <= 0) {
            //cout << "Client disconnected!" << endl;
            break;
        }

        // cout << "Message from client: " << buffer << endl;
        // cout<<"entered"<<endl;
        // increment_active_clients();
        // mtx.lock();
        // active_clients++;
        // current_clients++;
        // mtx.unlock();
        clients("inc");
        string off(buffer);
        off = off.substr(0, off.find('/'));
        int offset = stoi(off);
        // cout << "Offset: " << off << endl;

        if (offset >= v.size())
        {
            string res = "$$\n";
            send(new_socket, res.c_str(), res.length(), 0);
            // cout << "Response sent to client: " << res << endl;
            break;
        }

        // lock_guard<mutex> lock(mtx); 
        // cout<<"number of active clients: "<<get_active_clients()<<endl;
        // mtx.lock();int num_clients=active_clients; mtx.unlock();
        int q=clients("show");
        if (q>1) {
            // lock_guard<mutex> lock(mtx);
            send(new_socket, "HUH!\n", 5, 0);
            // cout << "Server is busy! Sent HUH! to client." << endl;
            // decrement_active_clients();
            clients("dec");
        }
        else if(q==1)
        {
            // cout<<"entered inner"<<endl;
            // lock_guard<mutex> lock(mtx);
            int number = 0;
            for (int chari = 0; (chari<num_packets)&&(offset<v.size()); chari++)
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
                    // cout<<"result is: "<<res<<endl;
                    // cout<<get_active_clients()<<endl;
                    // cout<<(get_active_clients()>1)<<endl;
                    
                    // has to change active_clients to clients("show")
                    if (clients("show")>1) {
                        // cout<<"entered if"<<endl;
                        send(new_socket, "HUH!\n", 5, 0);
                        // cout << "Server is busy! Sent HUH! to client." << endl;
                        break;
                    } 
                    // cout<<"entered other side"<<endl;
                    send(new_socket, res.c_str(), res.length(), 0);
                    // cout << "Response sent to client: " << res << endl;
                }
            }
            // decrement_active_clients();
            clients("dec");
        }
       
    }
    close(new_socket);  
    pthread_exit(NULL);
}

int main() {
    ifstream json_file("config.json");
    
    if (!json_file.is_open()) 
    {
        cerr << "Could not open the file!" << endl;
        return 1;
    }

    json j;
    json_file >> j;
    k=j["k"];
    p=j["p"];
    v=read_text(j["input_file"]);
    original_clients=j["num_clients"];
    int PORT=j["server_port"];

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
        cerr << "Bind failed!" << endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0)
    {
        cerr << "Listen failed!" << endl;
        close(server_fd);
        return -1;
    }

    // cout << "Server is waiting for connections..." << endl;
    vector<pthread_t> client_threads;

    while (true && (number_clients("show")<original_clients))
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

        pthread_t client_thread;
        number_clients("inc");
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