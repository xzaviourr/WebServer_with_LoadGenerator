#ifndef _HTTP_SERVER_HH_
#define _HTTP_SERVER_HH_

#include <bits/stdc++.h>
#include <sys/stat.h>
#include <fstream>
using namespace std;


class HTTP_REQUEST {
    public:
        string HTTP_VERSION;
        string METHOD;
        string URL;
    
    HTTP_REQUEST (string version = "HTTP/1.1", string method = "GET", string url = "/") {
        HTTP_VERSION = version;
        METHOD = method;
        URL = url;
    }

    void update_request (string request) {
        try {
            stringstream ss(request);
            string header_line;

            getline(ss, header_line, '\n'); // Extract header line
            stringstream wordss(header_line);
            string word;
            int word_counter = 0;
            while (wordss >> word) {
                if (word_counter == 0) {
                    METHOD = word;
                } else if (word_counter == 1) {
                    URL = word;
                } else if (word_counter == 2) {
                    HTTP_VERSION = word;
                }
            }
        } catch (...) {
            cout << "Invalid client request\n";
            HTTP_VERSION = "HTTP/1.1";
            METHOD = "GET";
            URL = "/";
        }
    }
};


class HTTP_RESPONSE {
    public:
        string HTTP_VERSION;
        string STATUS_CODE;
        string STATUS_TEXT;
        string CONTENT_TYPE;
        string CONTENT_LENGTH;
        string BODY;
    
    HTTP_RESPONSE (string version = "HTTP/1.1", string code = "200", string s_text = "OK", string c_type = "text/html; charset=UTF-8", string c_length = "0", string body = "") {
        HTTP_VERSION = version;
        STATUS_CODE = code;
        STATUS_TEXT = s_text;
        CONTENT_TYPE = c_type;
        CONTENT_LENGTH = c_length;
        BODY = body;
    }

    void update_response (HTTP_REQUEST *request) {
        string url = "./html_files" + request->URL;
        
        struct stat buffer;
        int status = stat(&url[0], &buffer);
        if (status != 0) {  // File and directory does not exists
            BODY = "<!DOCTYPE html>\n<head>\n</head>\n<body>\n<h1>404 NOT FOUND</h1>\n</body>\n</html>";
            HTTP_VERSION = "1.1";
            STATUS_CODE = "404";
            CONTENT_TYPE = "text/html; charset=UTF-8";
            CONTENT_LENGTH = "0";
            STATUS_TEXT = "Not Found";
        } else {
            if (S_ISDIR(buffer.st_mode)) {  // DIRECTORY FOUND
                if (url[url.length() - 1] == '/') {
                    url = url + "index.html";
                } else {
                    url = url + "/index.html";
                }
            }

            ifstream fin(url);
            string file_data((std::istreambuf_iterator<char>(fin)),
                 std::istreambuf_iterator<char>());

            BODY = file_data;
            HTTP_VERSION = "1.1";
            STATUS_CODE = "200";
            CONTENT_TYPE = "text/html; charset=UTF-8";
            CONTENT_LENGTH = file_data.length();
            STATUS_TEXT = "OK";
        }
    }

    string get_string() {
        string response_text = "HTTP/" + HTTP_VERSION + " " + STATUS_CODE + " " + STATUS_TEXT;
        response_text += "\nContent-Type: " + CONTENT_TYPE;
        response_text += "\n\n" + BODY;
        return response_text;
    }
};

#endif