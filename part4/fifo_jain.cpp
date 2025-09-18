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

// #define PORT 8080
#define Bufferlen 1024
int num_clients = 0;
int original_clients = 32;
int k = 10;
int p = 3;
vector<string> v = {};
int higher = 0;

// mutex mtx;
pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t init=PTHREAD_COND_INITIALIZER;
struct ClientArgs
{
    int new_socket;
};

struct client_request
{
    int socket;
    int offset;
    sockaddr_in address;
};

queue<client_request*> request_queue;
queue<client_request*>server_queue;

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
    file.close();
    words.push_back("EOF");
    return words;
}

void queue_fun(string func, client_request *c)
{
    // lock_guard<mutex> lock(mtx);
    pthread_mutex_lock(&mtx);
    if (func == "push")
    {
        request_queue.push(c);
        // cout<<"pushed into the queue "<<c->socket<<" "<<c->offset<<endl;
    }
    else if (func == "front")
    {
        // cout<<"checking for front  :"<<request_queue.size()<<endl;
        while(server_queue.empty())
        {
            pthread_cond_wait(&init,&mtx);
            while (!request_queue.empty())
            {
                server_queue.push(request_queue.front());
                request_queue.pop();
            }            
        }
        // if (request_queue.empty())
        //     return;
        // client_request *y = request_queue.front();
        // request_queue.pop();
    }
    pthread_mutex_unlock(&mtx);
    return;
}

void *service(void *arg)
{
    // cout << "enter into the service mode" << endl;
    int num_packets = (k % p) ? (k / p + 1) : (k / p);
    while (true)
    {
        // cout<<"entered while"<<endl;
        queue_fun("front", NULL);
        while (!server_queue.empty())
        {
            client_request *requests = server_queue.front();
            server_queue.pop();
            int new_socket = requests->socket;
            int offset = requests->offset;
            delete requests;
            // if (offset + k - 1 >= v.size() - 1)
            // {
            //     higher++;
            //     cout<<"higher is: "<<higher<<endl;
            // }
            // cout<<"sending "<<endl;
            // cout<<"k is"<<k<<" p is "<<p<<endl;
            // cout<<"size of v: "<<v->size()<<endl;
            if (offset >= v.size())
            {
                string res = "$$\n";
                send(new_socket, res.c_str(), res.length(), 0);
                // cout << "Response sent to client: " << res << endl;
                continue; 
            }
            // cout<<"offset is "<<offset<<" "<<new_socket<<endl;
            // cout<<"num clients "<<num_packets<<endl;
            int number = 0;
            for (int chari = 0; (chari < num_packets) && (offset < v.size()); chari++)
            {
                // cout<<"for loop"<<endl;
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
                    // cout << "Response sent to client: " << new_socket << " " << res << endl;
                }
            }
            // cout<<"done offset "<<offset<<" "<<new_socket<<endl;
        }
    }
    pthread_exit(NULL);
}

void *handle_client(void *arg)
{
    ClientArgs *args = (ClientArgs *)arg;
    int new_socket = args->new_socket;
    delete args; 

    char buffer[Bufferlen] = {0};

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(new_socket, buffer, Bufferlen);
        if (valread <= 0)
        {
            // cout << "Client disconnected!" << endl;
            break;
        }

        string off(buffer);
        if(off.back()=='t')
        {
            // cout << "Message from client: "<<new_socket<<" " << off<< endl;
        }
        int start=0;
        for(int index=0;index<off.size();index++)
        {
            if(off[index]=='\n')
            {
                string temp = off.substr(start, index-start);
                int offset = stoi(temp);
                // cout << "subOffset: " << offset << endl;

                client_request* requests=new client_request();
                requests->socket = new_socket;
                requests->offset=offset;
                // cout<<requests->k << " and "<<requests->p<<endl;   
                // cout<<"enter :"<<map_queue[new_socket]<<endl;
                // cout<<"received offset is"<<offset<<new_socket<<endl;
                queue_fun("push",requests);
                pthread_cond_signal(&init);
                start=index+1;
            }
        }
    }
    // cout << "closing client successfully :"<<new_socket << endl;
    close(new_socket);
    pthread_exit(NULL);
}

int main()
{

    ifstream json_file("config.json");

    if (!json_file.is_open())
    {
        cerr << "Could not open the file!" << endl;
        return 1;
    }

    json j;
    json_file >> j;
    k = j["k"];
    p = j["p"];
    v = read_text(j["input_file"]);
    original_clients = j["num_clients"];
    int PORT = j["server_port"];

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

    vector<pthread_t> client_threads;
    pthread_t server_processing;
    pthread_create(&server_processing, NULL, service, (void *)&v);
    // client_threads.push_back(server_processing);
    for (int ind = 0; ind < original_clients; ind++)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            cerr << "Accept failed!" << endl;
            close(server_fd);
            return -1;
        }

        // cout << "Connection established with a client! " << new_socket << endl;

        ClientArgs *args = new ClientArgs();
        args->new_socket = new_socket;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, (void *)args);
        client_threads.push_back(client_thread);
    }
    // pthread_join(server_processing, NULL);
    for (pthread_t &t : client_threads)
    {
        pthread_join(t, NULL);
    }
    // pthread_join(server_processing, NULL);
    // cout << "server closed..!" << endl;
    close(server_fd);
    return 0;
}