#include "client.hpp"
    int client::setupClient(const char* hostname, const char * port){
            int socket_fd;
            struct addrinfo hints, *servinfo, *temp;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            if (getaddrinfo(hostname, port, &hints, &servinfo)!=0){
                //Given node and service, use getaddrinfo to get the internet address
                perror(hostname);
                return -1;
            }

            for(temp = servinfo; temp != NULL; temp = temp->ai_next) {
                if ((socket_fd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) == -1) {
                    perror("client: socket");
                    continue;
                }

                if (connect(socket_fd, temp->ai_addr, temp->ai_addrlen) == -1) {
                    close(socket_fd);
                    perror("client: connect");
                    continue;
                }

                break;
            }

            std::cout << "Client: Connect to server successfully\n";
            freeaddrinfo(servinfo);
            return socket_fd;
        }