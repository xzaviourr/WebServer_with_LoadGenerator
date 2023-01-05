# **Web Server with Load Generator**
Web server built from scratch in C, that handles GET requests from the users and serves them with appropriate responses. 

Load generator is built to test the performance of the server using a closed loop load testing. Final results obtained for the server throughput and response time can be seen in the results. 

Project made in order to serve the requirements of course CS 744 at IIT Bombay. 

## Run the server
- Run the makefile to generate the executable    
    
    ```make all```

- Run the server

    ```./server <hostname> <server port>```

## Run the load tester
- Run the executable for the load tester

    ```./load_generator <hostname> <server port> <number of concurrent users> <think time  in seconds)> <test duration (in seconds)```

## Results
![Load Generator result](https://raw.githubusercontent.com/xzaviourr/WebServer_with_LoadGenerator/master/load-gen-output.png)