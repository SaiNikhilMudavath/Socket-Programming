#include <iostream>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
#define Bufferlen 1024

vector<string> read_text(string name)
{
    std::ifstream file(name);

    if (!file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return {};
    }

    vector<string> words;
    string line;
    getline(file, line);
    stringstream ss(line);
    string word;

    while(getline(ss,word,','))
    {
        words.push_back(word);
    }
    words.push_back("EOF");
    // Close the file
    file.close();
    return words;
}

int main() {
    std::ifstream json_file("config.json");
    
    if(!json_file.is_open())
    {
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
    vector<string>v=read_text(j["input_file"]);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[Bufferlen] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT);       

    // Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed!" << std::endl;
        close(server_fd);
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed!" << std::endl;
        close(server_fd);
        return -1;
    }

    //::cout << "Server is waiting for connections..." << std::endl;

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        std::cerr << "Accept failed!" << std::endl;
        close(server_fd);
        return -1;
    }

    //std::cout << "Connection established with the client!" << std::endl;
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));  // Clear buffer

        // Read message from client
        int valread = read(new_socket, buffer, Bufferlen);
        if (valread <= 0) {
            // cout << "Client disconnected!" <<endl;
            break;  // Exit loop if client disconnects
        }

        //cout << "Message from client: " << buffer <<endl;
        int i=0;string off="";
        while(i<Bufferlen && buffer[i]!='/')
        {
            off+=buffer[i];
            i++;
        }
        int offset=stoi(off);
        // cout<<off<<endl;
        if(offset>=v.size())
        {
            string res="$$\n";
            send(new_socket, res.c_str(), res.length(), 0);
            std::cout << "Response sent to client: "<<res<< std::endl;
            break;
        }

        if (strcmp(buffer, "exit") == 0) {
            std::cout << "Client requested to terminate the connection." << std::endl;
            break;
        }
        //cout<<"num packets is "<<num_packets<<endl;
        int number=0;
        //cout<<"offset is "<<offset<<endl;
        for(int chari=0;(chari<num_packets)&&(offset<v.size());chari++)
        {
            string res="";
            int loop_number=0;
            while(offset<v.size() && loop_number<p && number<k)
            {
                res+=v[offset];res+=",";
                offset++;loop_number++;number++;
            }
            res.pop_back();res+='\n';
            send(new_socket, res.c_str(), res.length(), 0);
            //std::cout << "Response sent to client" << res<<std::endl;
        }
    }
    close(new_socket);
    close(server_fd);

    return 0;
}