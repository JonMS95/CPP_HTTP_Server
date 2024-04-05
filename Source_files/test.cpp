#include "test_api.hpp"


/************************************/
/******** Include statements ********/
/************************************/

#include <netinet/in.h>     // INET_ADDRSTRLEN.
#include <fcntl.h>          // Set socket flags.
#include <openssl/ssl.h>
#include <arpa/inet.h>      // sockaddr_in, inet_addr
#include <unistd.h>
#include "SeverityLog_api.h"

#include <string>
#include <fstream>          // Read from file to string

/*************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_LEN_RX_BUFFER             256     // RX buffer size.
#define SERVER_SOCKET_LEN_TX_BUFFER             256     // TX buffer size.

#define SERVER_SOCKET_MSG_ERR_GET_SOCKET_FLAGS  "Error while getting socket flags."
#define SERVER_SOCKET_MSG_ERR_SET_SOCKET_FLAGS  "Error while setting O_NONBLOCK flag."

#define SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT "Data read from client: <%s>"

#define SERVER_SOCKET_MSG_CLIENT_DISCONNECTED   "Client with IP <%s> disconnected."

/************************************/

/************************************/
/********* Private variables ********/
/************************************/

std::string path_to_resources;

/************************************/

/// @brief Get client's IP address.
/// @param client_socket Client socket.
/// @param client_IPv4 Target string to which client IPv4 address is meant to be copied.
static void ServerSocketGetClientIPv4(int client_socket, char* client_IPv4)
{
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    getpeername(client_socket, (struct sockaddr*)&client, &client_len);
    inet_ntop(AF_INET, &client.sin_addr, client_IPv4, INET_ADDRSTRLEN);
}

/// @brief Set the socket as non-blocking.
/// @param client_socket Client socket.
/// @return < 0 is getting / settings flags failed 0, otherwise.
static int ServerSocketSetNonBlocking(int client_socket)
{
    // First, try to get socket's current flag set.
    int flags = fcntl(client_socket, F_GETFL, 0);
    if(flags < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_ERR_GET_SOCKET_FLAGS);
        return flags;
    }

    // Then, set the O_NONBLOCK flag (which, as the name suggests, makes the socket non-blocking).
    flags |= O_NONBLOCK;
    if(fcntl(client_socket, F_SETFL, flags) < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_ERR_SET_SOCKET_FLAGS);
        return flags;
    }

    return 0;
}

void HttpSetPathToResources(char* path)
{
    path_to_resources = path;
}

std::string HttpGetPathToResources(void)
{
    return path_to_resources;
}

int HttpCopyHTMLToString(std::string& requested_resource, std::string& dest)
{
    if(requested_resource == "")
        requested_resource = "index.html";
    
    std::string path_to_requested_resource = HttpGetPathToResources() + '/' + requested_resource;

    std::ifstream file(path_to_requested_resource); // Open the file

    if (!file.is_open())
    {
        LOG_ERR("Error opening file.");
        return -101;
    }

    // Read the file content into an std::string
    dest.assign((   std::istreambuf_iterator<char>(file))   ,
                    std::istreambuf_iterator<char>())       ;

    // Close the file
    file.close();

    return 0;
}

/// @brief Reads from client, then sends a response.
/// @param client_socket Client socket.
/// @param secure True if TLS security is wanted, false otherwise.
/// @param ssl SSL data.
/// @return < 0 if any error happened, 0 otherwise.
int HttpServerDefaultResponse(int client_socket, bool secure, SSL** ssl)
{
    //////////////////////////////////////////////////////////////////////
    // START CUSTOM INTERACT FUNCTION
    //////////////////////////////////////////////////////////////////////

    ////////////////////////////////
    // GET THE CLIENT IP ADDR FIRST.
    ////////////////////////////////

    // Get client IP first, then Log it's IP address.
    char client_IP_addr[INET_ADDRSTRLEN] = {};
    ServerSocketGetClientIPv4(client_socket, client_IP_addr);

    // Set socket as non-blocking.
    int set_non_blocking = ServerSocketSetNonBlocking(client_socket);
    if(set_non_blocking < 0)
        return set_non_blocking;

    char rx_buffer[SERVER_SOCKET_LEN_RX_BUFFER];
    memset(rx_buffer, 0, sizeof(rx_buffer));

    ssize_t read_from_socket = -1;
    bool something_read = false;

    while( (read_from_socket >= 0) || (something_read == false) )
    {
        if(!secure)
            read_from_socket = read(client_socket, rx_buffer, sizeof(rx_buffer));
        else
            read_from_socket = SSL_read(*ssl, rx_buffer, sizeof(rx_buffer));

        // Check if the client is still connected, or if no data has been received.
        if(read_from_socket == 0)
        {
            LOG_WNG(SERVER_SOCKET_MSG_CLIENT_DISCONNECTED, client_IP_addr);
            break;
        }

        // If no data has been found in the rx buffer but it has been found previously, then exit.
        // It is assumed that all the data that was meant to be read has already been read.
        if(read_from_socket < 0)
        {
            if(something_read)
                break;
            
            continue;
        }

        // If any data has been received, then set the something_read flag and display it on console.
        // Remove possible ending new line and carriage return characters first.
        something_read = true;
        
        if(strlen(rx_buffer) > 0 && rx_buffer[strlen(rx_buffer) - 1] == '\n')
            rx_buffer[strlen(rx_buffer) - 1] = 0;

        if(strlen(rx_buffer) > 0 && rx_buffer[strlen(rx_buffer) - 1] == '\r')
            rx_buffer[strlen(rx_buffer) - 1] = 0;

        if(strlen(rx_buffer) > 0)
        {
            LOG_INF(SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT, rx_buffer);
        }

        // Clean the buffer after reading.
        memset(rx_buffer, 0, read_from_socket);

        // Wait for a millisecond before trying to read again.
        usleep(1000);
    }

    std::string html_page;
    std::string requested_resource = "";

    int get_html = HttpCopyHTMLToString(requested_resource, html_page);
    if(get_html < 0)
        return get_html;

    // Construct HTTP response with headers and HTML content
    std::string httpResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: " +  std::to_string(html_page.size()) + "\r\n"
                               "\r\n" +
                               html_page;

    // Send a message to the client as soon as it is accepted.
    char tx_buffer[SERVER_SOCKET_LEN_TX_BUFFER];
    memset(tx_buffer, 0, sizeof(tx_buffer));
    sprintf(tx_buffer, httpResponse.c_str(), client_IP_addr);

    if(!secure)
        write(client_socket, tx_buffer, strlen(tx_buffer));
    else
        SSL_write(*ssl, tx_buffer, strlen(tx_buffer));

    //////////////////////////////////////////////////////////////////////
    // END CUSTOM INTERACT FUNCTION
    //////////////////////////////////////////////////////////////////////

    return 0;
}
