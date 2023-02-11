#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sstream> 
#include <vector>
#include <iterator>
using namespace std;

int main() {

    //Variables
    int listening;
    int Port = 8080;
    int bufsize = 4096;
    char buffer[bufsize];

    //Create socket
    listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1)
    {
        cerr <<"Can't create a Socket:" << endl;
        return -1;
    }

    cout << "\n=> Socket server has been created..." << endl;

    //Bind the socket to a IP/Port
    struct sockaddr_in server_addr;
    socklen_t serverSize = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Port);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (bind(listening, (struct sockaddr*)&server_addr, serverSize) == -1)
    {
        cerr <<"Can't bind to IP/Port" << endl;
        return -2;
    }

    cout << "=> Looking for clients..." << endl;
    
    //Listening 
    if (listen(listening, SOMAXCONN) == -1)
    {
        cerr <<"Can't listen" << endl;
    }

    //Accepting clients
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    while (1)
    {
        int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
        if (clientSocket == -1)
        {
            cerr <<"=> Error on accepting" << endl;
        }
     
        // //Close the listening socket
        // close(listening);
        memset(host, 0, NI_MAXHOST);    
        memset(service, 0, NI_MAXSERV);    

        int result = getnameinfo((sockaddr*)&client, clientSize, host, NI_MAXHOST, service, NI_MAXSERV, 0);
        if (result)
        {
            cout << host << " connected on " << service << endl;
        }
        else
        {
            inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            cout << host << " connected on " << ntohs(client.sin_port) << endl;
        }
        
         //clear buffer
        memset(buffer, 0, bufsize);

        //wait for message
        int byteRecv = recv(clientSocket, buffer, bufsize, 0);

        if (byteRecv == -1)
        {
            cerr <<"Connection issue" << endl;
            return 0;
        }
     
        istringstream iss(buffer);
        vector<string> parsed((istream_iterator<string>(iss)), istream_iterator<string>());
        string url, content;

        if (parsed.size() >= 3 && parsed[0] == "GET")
        {
            url = parsed[1];
            if (url == "/")
            {

                content = "<h1>Index page</h1></br></br><a href=\"about.html\">About Page</a>";
                
            }
            else if (url == "/about.html")
            {
                content = "<h1>About page</h1>";
            }
            else
            {
                content = "<h1>404 page</h1>";
            }

            cout <<"Parsed: " << url << " This is: " << content << " " << content.size()  << endl;
            
        }

        ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n";
        oss << "Cache-Control: no-cache, private\r\n";
        oss << "Content-Type: text/html\r\n";
        oss << "Content-Length: " << content.size() << "\r\n";
        oss << "\r\n";
        oss << content;

        string output = oss.str();
        int size = output.size() + 1;

        //send message
        send(clientSocket, output.c_str(), size, 0);

        //Close client socket
        close(clientSocket);
    }
    
    return 0;
    
}