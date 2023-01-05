#include <bits/stdc++.h>
#include <sys/time.h>
#include <random>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
using namespace std;

#define lli long long int

string AVAILABLE_RESOURCES[] = {"/", "/apart1/", "/apart1/flat11/", "/apart1/flat12/", "/apart2/", "/apart2/", "/apar2/flat21/", "/apart3/", "/apart3/flat31/", "/apart3/flat32/"};
int IS_APPLICATION_ACTIVE = 1;

float time_diff(timeval *t2, timeval *t1) {
    return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

struct USER {
    lli PORT;
    string HOSTNAME;
    double THINK_TIME;

    lli TOTAL_REQUESTS;
    double TOTAL_RTT;
};

void* user_function(void *arg) {
    USER* user_properties = (USER*) arg;
    timeval start_time, end_time;
    lli server_ptr, socket_ptr;
    string request;
    char buffer[1024];
    int check_flag;

    while (1) {
        request = "GET " + AVAILABLE_RESOURCES[rand()%10] + " HTTP/1.1";
        gettimeofday(&start_time, NULL);

        socket_ptr = socket(AF_INET, SOCK_STREAM, 0);
        // setsockopt(socket_ptr, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tout, sizeof tout);
        if (socket_ptr < 0)
        continue;

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(user_properties->PORT);
        inet_pton(AF_INET, &user_properties->HOSTNAME[0], &server_address.sin_addr);

        server_ptr = connect(socket_ptr, (struct sockaddr*)&server_address, sizeof(server_address));
        if (server_ptr < 0) {
            close(socket_ptr);
            continue;
        }

        check_flag = write(socket_ptr, &request, request.length());
        if ( check_flag < 0 ) {
            close(socket_ptr);
            continue;
        }

        check_flag = read(socket_ptr, buffer, 1024);
        if (check_flag < 0) {
            close(socket_ptr);
            continue;
        }

        close(socket_ptr);
        gettimeofday(&end_time, NULL);

        if (IS_APPLICATION_ACTIVE == 0) {
            break;
        }

        user_properties->TOTAL_REQUESTS += 1;
        user_properties->TOTAL_RTT += time_diff(&end_time, &start_time);

        bzero(buffer, 1024);
        usleep(user_properties->THINK_TIME * 1000000);
    }
    return NULL;
}

int main (int argc, char* argv[]) {
    lli user_count, port, test_duration;
    float think_time;
    char* hostname = NULL;
    if (argc != 6) {
        cout<<"Usage: "<<argv[0]<<" <hostname> <server port> <number of concurrent users> <think time (in s)> <test duration (in s)>\n";
        exit(0);
    }

    // Data type setup
    hostname = argv[1];
    port = atoi(argv[2]);
    user_count = atoi(argv[3]);
    think_time = atof(argv[4]);
    test_duration = atoi(argv[5]);

    USER users[user_count];
    pthread_t thread_id[user_count];
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    for (int i=0;i<user_count;i++) {
        // Generate user properties
        users[i].PORT = port;
        users[i].HOSTNAME = hostname;
        users[i].THINK_TIME = think_time;
        users[i].TOTAL_REQUESTS = 0;
        users[i].TOTAL_RTT = 0;

        pthread_create(&thread_id[i], NULL, &user_function, &users[i]);
    } 

    lli time_counter = 1;
    do {
        sleep(1);
        gettimeofday(&end_time, NULL);
        // cout<<"Time : "<<time_counter++<<endl;
    } while (time_diff(&end_time, &start_time) <= test_duration);
    
    // cout<<time_diff(&end_time, &start_time)<<endl;
    IS_APPLICATION_ACTIVE = 0;

    // STOP ALL THREADS
    for(int i=0;i<user_count;i++) {
        pthread_cancel(thread_id[i]);
    }

    int total_requests = 0;
    float total_time = 0;
    for(int i=0;i<user_count;i++) {
        total_requests += users[i].TOTAL_REQUESTS;
        total_time += users[i].TOTAL_RTT;
    }
    // printf("%lld,%f,%f\n",user_count,((float)total_requests)/test_duration,total_time/total_requests);
    cout<<"Total Request Count : "<<total_requests<<endl;
    cout<<"Throughput : "<<((float)total_requests/(float)test_duration)<<endl;
    cout<<"Average Response Time : "<<total_time/total_requests<<endl;
}