#include <bits/stdc++.h> // Basic functions
#include <sys/socket.h> // Socket
#include <netinet/in.h> // sockarr_in

#include "http_server.hh"
using namespace std;

#define lli long long int
#define MAX_CONN 100
#define THREAD_POOL_SIZE 4
#define MAX_WAITING_QUEUE_LENGTH 10000

int CLIENT_WAITING_QUEUE[MAX_WAITING_QUEUE_LENGTH];

// CRITICAL SECTION VARIABLES
int CURRENT_QUEUE_PTR = 0;  // Pointer points to empty position in queue
int CURRENT_DEQUE_PTR = 0;  // Pointer points to oldest filled position
int CURRENT_QUEUE_LENGTH = 0;   // Current queue length

// THREAD VARIABLES
pthread_mutex_t THREAD_LOCK = PTHREAD_MUTEX_INITIALIZER; // Thread lock
pthread_cond_t WORKER_CV = PTHREAD_COND_INITIALIZER;    // Conditional variable for workers
pthread_cond_t PARENT_CV = PTHREAD_COND_INITIALIZER;    // Conditional variable for parent

// APPLICATION VARIABLES
pthread_t THREAD_IDS[THREAD_POOL_SIZE]; // Stores unique thread ids 
int IS_APP_ACTIVE = 1;  // Stores if the app is active or not

void program_exit(int signum) {
    IS_APP_ACTIVE = 0;
    pthread_cond_broadcast(&WORKER_CV);
    for (lli i=0;i<THREAD_POOL_SIZE;i++) {
        pthread_join(THREAD_IDS[i], NULL);
    }
    exit(0);
}

void* serve_client(void* arg) {
    lli client_ptr = 0;
    HTTP_REQUEST request_obj;
    HTTP_RESPONSE response_obj;

    char buffer[1024];

    while (1) { // Run till stopped externally
        // CRITICAL SECTION
        pthread_mutex_lock(&THREAD_LOCK);
        while (CLIENT_WAITING_QUEUE[CURRENT_DEQUE_PTR] == -1) { // No work is available now
            pthread_cond_wait(&WORKER_CV, &THREAD_LOCK);    // Wait for signal
            if (IS_APP_ACTIVE == 0) {
                pthread_mutex_unlock(&THREAD_LOCK);
                pthread_exit(NULL);
            }
        }

        // Fetch new work from the queue
        client_ptr = CLIENT_WAITING_QUEUE[CURRENT_DEQUE_PTR];
        CLIENT_WAITING_QUEUE[CURRENT_DEQUE_PTR] = -1;
        CURRENT_DEQUE_PTR = (CURRENT_DEQUE_PTR + 1) % MAX_WAITING_QUEUE_LENGTH;
        CURRENT_QUEUE_LENGTH --;

        // Signal parent if the queue was full
        if (CURRENT_QUEUE_LENGTH == MAX_WAITING_QUEUE_LENGTH - 1) {
            pthread_cond_signal(&PARENT_CV);
        }
        pthread_mutex_unlock(&THREAD_LOCK);
        
        // Generate response for the request
        lli bytes_read = read(client_ptr, buffer, 1024);

        request_obj.update_request(buffer);
        response_obj.update_response(&request_obj);
        string response_msg = response_obj.get_string();

        write (client_ptr, &response_msg[0], response_msg.length());
        close(client_ptr);
        // cout<<"End Connection : "<<client_ptr<<endl;

        bzero(buffer, 1024);
    }
}

int main(int argc, char const* argv[]) {
    signal(SIGINT, program_exit);

    // Check for parameters
    if (argc != 2) {
        perror("Invalid Command. Usage : ./server <PORT_NUMBER>\n");
        exit(EXIT_FAILURE);
    }

    // Setup default values
    for (lli i=0;i<MAX_WAITING_QUEUE_LENGTH;i++) {
        CLIENT_WAITING_QUEUE[i] = -1;   // -1 represents empty
    }

    // Setup listening socket
    lli port = stoi(argv[1]);
    lli socket_ptr = socket(AF_INET, SOCK_STREAM, 0);
    int opt_flag = 1;
    struct sockaddr_in socket_address;
    lli addrlen = sizeof(socket_address);

    setsockopt(socket_ptr, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_flag, sizeof(opt_flag));
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);
    bind(socket_ptr, (struct sockaddr*)&socket_address, sizeof(socket_address));
    listen(socket_ptr, MAX_CONN);


    // Start worker threads
    for(lli i=0;i<THREAD_POOL_SIZE;i++) {
        pthread_create(&THREAD_IDS[i], NULL, &serve_client, NULL);
    }

    cout<<"=============== SERVER ACTIVE ================\n";

    // Wait for incoming connections
    while (1) {
        lli client_ptr = accept(socket_ptr, (struct sockaddr*)&socket_address, (socklen_t*)&addrlen);
        // cout<<"New Connection : "<<client_ptr<<endl;

        pthread_mutex_lock(&THREAD_LOCK);
        if (CURRENT_QUEUE_LENGTH != MAX_WAITING_QUEUE_LENGTH) { // Empty Queue
            CLIENT_WAITING_QUEUE[CURRENT_QUEUE_PTR] = client_ptr;
            CURRENT_QUEUE_PTR = (CURRENT_QUEUE_PTR + 1) % MAX_WAITING_QUEUE_LENGTH;
            CURRENT_QUEUE_LENGTH ++;
            pthread_cond_signal(&WORKER_CV);    // Start a worker to handle the request
        } else {    // Full Queue
            pthread_cond_wait(&PARENT_CV, &THREAD_LOCK);
        }
        pthread_mutex_unlock(&THREAD_LOCK);
    }
    shutdown(socket_ptr, SHUT_RDWR);
    cout<<"=============== SERVER CLOSED ================\n";
}