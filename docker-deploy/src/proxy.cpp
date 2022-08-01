#include "proxy.hpp"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::fstream file;

/* get the first request */
void proxy::receiveRequestFromClient(int fd, std::string &ans){
    char ans1[65536] = {0};
    int recv_ans = recv(fd, ans1, sizeof(ans1), 0);
    if(recv_ans==-1){
        std::cout << "Error in receive first request from client" << std::endl;
        return;
    }
    // if (send(fd, ans1, strlen(ans1), 0) == -1){
    //     perror("send");
    // }
    //std::cout << "very first length: " << recv_ans;
    ans.assign(ans1, recv_ans);
}

proxy::proxy(const char* port):port_num(port){}

void proxy::start(){
    file.open("/var/log/erss/proxy.log", std::fstream::out);
    if(file.is_open()){
        std::cout << "open file success" << std::endl;
    }else{
        perror("file");
        std::cout << "open file failure" << std::endl;
    }
    //file<<"this is the file";
    server *myserver = new server();
    int server_socket = myserver->setup("12345");
    cache *mycache = new cache(100);
    std::fstream fs;
    //TODO setup server socket setup failure
    while(1){
        std::string browser_ip;
        int server_accept_socket = myserver->toAccept(server_socket, browser_ip);
        std::cout << browser_ip << std::endl;
        //create a new thread to receive the information from 
        pthread_t new_thread;
        Thread *mythread = new Thread(mycache, server_accept_socket, browser_ip, id);
        id++;
        pthread_create(&new_thread, NULL, processRequestFromClient, mythread);
    }
}


void * proxy::processRequestFromClient(void * mythread_ptr){
    //pthread_mutex_lock(&mutex);
    Thread * mythread = (Thread*)mythread_ptr;
    cache* mycache = mythread->mycache;
    int server_accept_socket = mythread->sockect_as_server;
    int id = mythread->thread_id;
    std::string ip = mythread->ip;
    std::string original_rqst;
    //int original_len;
    receiveRequestFromClient(server_accept_socket, original_rqst);
    if(original_rqst.empty()){
        return NULL;
    }
    //begin parse
    Request new_rqst(original_rqst);
    //std::cout<<"parse request start here!"<<std::endl;
    std::cout<<"the first original request: "<<new_rqst.rqst_line<<std::endl;
    //std::cout<<"request line: "<<new_rqst.rqst_line<<std::endl;
    //std::cout<<"method: "<<new_rqst.method<<std::endl;
    //std::cout<<"URL: "<<new_rqst.url<<std::endl;
    //std::cout<<"HOST name: "<<new_rqst.domain<<std::endl;
    //std::cout<<"Port: "<<new_rqst.port<<std::endl;
    //  int length = new_rqst.getLength();
    //std::cout<<"content length: "<<length<<std::endl;
    //pthread_mutex_unlock(&mutex);

    //create socket to connect to original server
    client c;
    char* domain = new char[new_rqst.domain.length() + 1];
    std::strcpy (domain, new_rqst.domain.c_str());
    char* port = new char[new_rqst.port.length() + 1];
    std::strcpy (port, new_rqst.port.c_str());
    int connectToServer_fd = c.setupClient(domain, port);
    std::cout << "creating a socket to connect to the original server " << connectToServer_fd <<std::endl;
    std::stringstream ss;
    char *time = getCurrenttime();
    ss << id <<": "<<'\"'<<new_rqst.rqst_line<<'\"'<<" from "<<ip<<" @ "<<time;
    std::string str = ss.str();
    pthread_mutex_lock(&mutex);
    writeLog(str);
    pthread_mutex_unlock(&mutex);
    //if get
    if(new_rqst.method.compare("GET")==0){
        std::cout << "start getting process" << std::endl;
        getRequest(connectToServer_fd, server_accept_socket, new_rqst, mycache, mythread->thread_id);
    }
    //if post
    if(new_rqst.method.compare("POST")==0){
        std::cout << "start posting process" << std::endl;
        //int client_fd, int server_fd, Request rqst
        postRequest(connectToServer_fd, server_accept_socket, new_rqst, id);
    }

    //if connect
    if(new_rqst.method.compare("CONNECT")==0){
        std::cout << "start connecting process" << std::endl;
        tryConnect(server_accept_socket, connectToServer_fd, id, new_rqst);
    }
    close(server_accept_socket);
    close(connectToServer_fd);
    std::cout << "finish" << std::endl;
    //pthread_mutex_unlock(&mutex);
      return NULL;
}


void proxy::tryConnect(int server_accept_socket, int connectToServer_fd, int id, Request request){
    send(server_accept_socket, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    //std::cout<<"send value here! "<<x<<std::endl;
    fd_set readfds;//set of socket destriptor
    int n = std::max(server_accept_socket, connectToServer_fd) + 1;
    int rv;
    while(1){
        FD_ZERO(&readfds);//clear the set
        //add fd to the set
        FD_SET(server_accept_socket, &readfds);
        FD_SET(connectToServer_fd, &readfds);

        rv = select(n, &readfds, NULL, NULL, NULL);
        if(rv==-1){
            //perror("waiting client");
            break;
        }
        char msg1[65536] = {0};
        char msg2[65536] = {0};
        //proxy received the data from client
        if(FD_ISSET(server_accept_socket, &readfds)){
            //std::cout << "proxy are receiving data from client" << std::endl;
            int recv_res = recv(server_accept_socket, msg1, sizeof(msg1), 0);
            if(recv_res==0){
                std::stringstream ss;
                ss << id;
                ss << ": Tunnel closed\n";
                pthread_mutex_lock(&mutex);
                writeLog(ss.str());
                pthread_mutex_unlock(&mutex);
                return;
            }else if(recv_res<0){
                perror("here");
                //std::cout << "ERROR in recv from client" << recv_res <<std::endl;
                return;
            }else{
                //std::cout << "receive bytes from client: " <<recv_res << std::endl;
            }
             //re-send the data to server
             
             int send_res = send(connectToServer_fd, msg1, recv_res, 0);

             if(send_res>0){
                 //std::cout << "Success in send to server " << send_res << std::endl;
             }
             if(send_res<=0){
                 //std::cout << "Error in send to server" << send_res << std::endl;
                 return;
                 //perror("send to client: ");
             }
        }
        if(FD_ISSET(connectToServer_fd, &readfds)){
            //std::cout << "proxy are receving data from server" << std::endl;
            int recv_res_1 = recv(connectToServer_fd, msg2, sizeof(msg2), 0);
            if(recv_res_1==0){
                std::stringstream ss;
                ss << id;
                ss << ": Tunnel closed\n";
                pthread_mutex_lock(&mutex);
                writeLog(ss.str());
                pthread_mutex_unlock(&mutex);
                return;
            }
            if(recv_res_1<0){
                //perror("data from server");
                //std::cout << "data from server";
                return;
            }
            //std::cout << "receive bytes from server: " <<recv_res_1 << std::endl;
             //re-send the data to client
             int send_res_1 = send(server_accept_socket, msg2, recv_res_1, 0);
             if(send_res_1<=0){
                 //perror("send to server");
                 //std::cout << "send to server" << std::endl;
                 return;
                 //perror("send to server: ");
             }else{
                 //std::cout << "SUCCESS in send data to client: " << send_res_1 <<std::endl;
             }
        }
    }
}


std::string proxy::receiveAllmsg(int fd, std::string msg, bool chunk, int& total){
    //std::cout << "message length is" << msg.length() << "chunk is: " << chunk << std::endl;
    if(!chunk && msg.length()<=65536){
        total+=msg.length();
        return msg;
    }
    int rev_len = 0;
    std::string total_rqst;
    total_rqst.append(msg);
    //std::cout<<"first string: "<<total_rqst<<std::endl;
    
    while(1){
        //std::cout<<"in while now"<<std::endl;
        char rev_msg[65536] = {0};
        rev_len = recv(fd, rev_msg, sizeof(rev_msg), 0);
        //std::cout<<"first receive length: "<<rev_len<<std::endl;
        if(rev_len<=0){
            break;
        }
        std::string rev_msg_str(rev_msg, rev_len);
        //std::cout<<"receive length is "<<rev_len<<" receive message is "<<rev_msg_str<<std::endl;
        if(chunk){
            //std::cout<<"it is chunked"<<std::endl;
            std::size_t f_chunked = rev_msg_str.find("0\r\n\r\n");
            if(f_chunked != std::string::npos){
                //std::cout<<"break here"<<std::endl;
                std::string rec(rev_msg, rev_len);
                total_rqst.append(rec);
                total += rev_len;
                break;
            }
        }
        else{
            //std::cout<<"it is nochunk"<<std::endl;
            if(rev_len <= 0){
                break;
            }
        }
        std::string rec(rev_msg, rev_len);
        total_rqst.append(rec);
        total += rev_len;
        //std::cout<<"chunk total length: "<<total<<std::endl;
    }

    return total_rqst;
}


/* proxy as client, connect to server*/
void proxy::postRequest(int client_fd, int server_fd, Request rqst, int id){
    //std::string origi_msg = rqst.origi_rqst;
    //int content_len = rqst.getLength();
    //std::string origi_msg = receiveAllchunks(server_fd);
    //std::cout<<"original message from client"<<origi_msg<<std::endl;
    //if(content_len != -1){
       //  char request[origi_msg.size() + 1];
       //  strcpy(request, origi_msg.c_str());
    //send to original server, make sure to receive all request
    bool rqst_chunk = rqst.isChunked();
    std::cout<<"chunk value is "<<rqst_chunk<<std::endl;
    int origi_msg_len = 0;
    std::string origi_msg = receiveAllmsg(server_fd, rqst.origi_rqst, rqst_chunk, origi_msg_len);
    std::cout << "message is: " << origi_msg;
    send(client_fd, origi_msg.data(), origi_msg_len, 0);
    Request request(origi_msg);
    pthread_mutex_lock(&mutex);
    writeLogforproxyrequest(request, id);
    pthread_mutex_unlock(&mutex);
    //receive from original server, receive the first response
        char origi_resp[65536] = {0};
        int num = recv(client_fd, origi_resp, sizeof(origi_resp), 0);
        //std::cout<<"original response firstly: "<<origi_resp<<std::endl;
        if(num > 0){
            Response response(origi_resp);
            //check chunk, receive all chunks
            bool rsps_chunk = response.isChunked();
            int all_response_len = 0;
            std::string all_response = receiveAllmsg(client_fd, response.response, rsps_chunk, all_response_len);
            Response new_rsps(all_response);
            pthread_mutex_lock(&mutex);
            writeLogforproxyresponse(all_response, request, id);
            pthread_mutex_unlock(&mutex);
            // write into log here
            send(server_fd, all_response.data(), all_response_len, 0);
            writeLogproxyserver(new_rsps);
        }
        else{
            perror("error port");
            //std::cerr<<"error port"<<std::endl;
        }

    //}
}

void proxy::revalidate(int proxy_as_client_fd, int proxy_as_server_fd, Request rqst, Response resp, cache* mycache, int id){
    // check ETag
    //std::cout<<"handle request: "<<rqst.rqst_line<<std::endl;
    //std::cout<<"handle request header: "<<rqst.header<<std::endl;
    //std::cout<<"handle response: "<<resp.response<<std::endl;
    std::string vali_rqst = rqst.header + "\r\n";
    if(resp.etag != ""){
        vali_rqst += "If-None-Match: " + resp.etag + "\r\n";
    }
    if(resp.last_modify != ""){
        vali_rqst += "If-Modified-Since: " + resp.last_modify + "\r\n";
    }
    vali_rqst += "\r\n\r\n";
    //std::cout << "new request is" << vali_rqst << std::endl;
    //std::cout << "new request length" <<vali_rqst.length()<<std::endl;
    send(proxy_as_client_fd, vali_rqst.data(), vali_rqst.size() + 1, 0);
    pthread_mutex_lock(&mutex);
    writeLogforproxyrequest(rqst, id);
    pthread_mutex_unlock(&mutex);
    char vali_resp[65536] = {0};
    int vali_resp_len = recv(proxy_as_client_fd, vali_resp, sizeof(vali_resp), 0);
    std::cout << "revalidate: LENGTH" << vali_resp_len << std::endl;
    if(vali_resp_len != 0){
        std::string reps_str(vali_resp, vali_resp_len);
        Response response(reps_str);
        pthread_mutex_lock(&mutex);
        writeLogforproxyresponse(response, rqst, id);
        pthread_mutex_unlock(&mutex);
        if(response.code == "200"){
            //update cache and response
            mycache->updateCache(rqst.rqst_line, response);
            send(proxy_as_server_fd, response.response.data(), response.response.size() + 1, 0);
            writeLogproxyserver(response);
            std::stringstream ss;
            ss << id << ": ";
            if(response.is_nostore==1){
                ss << "not cacheable because it contains no store field";
            }
            else if(response.is_nocache==1){
                ss << "cached, but requires re-validation";
            }else if(response.expire_date!=""){
                ss << "cached, but expires at " << response.expire_date;
            }
            pthread_mutex_lock(&mutex);
            writeLog(ss.str());
            pthread_mutex_unlock(&mutex);
        }
        else if(response.code == "304"){
            //response using cache
            send(proxy_as_server_fd, response.response.data(), response.response.size() + 1, 0);
            writeLogproxyserver(response);
        }
        
    }
}

void proxy::getRequest(int client_fd, int server_fd, Request rqst, cache* mycache, int id){
    // tell if the rqst is in cache or not
    //int content_len = rqst.getLength();
    //std::string origi_msg = receiveAllchunks(server_fd, rqst.origi_rqst, content_len);
    bool rqst_chunk = rqst.isChunked();
    int origi_msg_len = 0;
    //std::cout << "reererqerq: "<<rqst.origi_rqst<<std::endl;
    //std::cout << "ans length:"<<rqst.origi_rqst.length()<<std::endl;
    //std::cout << "CHUNK"<<rqst_chunk<<std::endl;
    //std::string origi_msg = rqst.origi_rqst;
    //int origi_msg_len = rqst.origi_rqst.length();
    std::string origi_msg =receiveAllmsg(server_fd, rqst.origi_rqst, rqst_chunk, origi_msg_len);

    std::cout<<"original message from client"<<origi_msg<<std::endl;

    std::string rqst_line = rqst.rqst_line;
    
    if(mycache->checkCache(rqst_line)){
        std::cout << "in cache" << std::endl;
        //if in cache
        Response response = mycache->getResponse(rqst_line);
        //std::cout << "getting response from cache: " << response.response << std::endl;
        std::cout << "mymax-age: "<<response.max_age<<std::endl;
        std::cout << "myisfresh: "<<response.isFresh()<<std::endl;
        std::cout << "mydate: "<<response.date<<std::endl;
        //std::cout << "my utc date: "<<response.getTime(response.date)<<std::endl;
        //std::cout << "my current age: "<<response.getLifespan()<<std::endl;
        //check no cache
        if(response.is_nocache){
            std::cout << "is no cache" << std::endl;
            std::stringstream ss;
            ss << id;
            ss << ": in cache, requires validation\n";
            pthread_mutex_lock(&mutex);
            writeLog(ss.str());
            pthread_mutex_unlock(&mutex);
            std::cout << "no cache" << std::endl;
            revalidate(client_fd, server_fd, rqst, response, mycache, id);
            return;
        }
        //check fresh
        if(response.isFresh()){
            std::cout << "is fresh" << std::endl;
            std::stringstream ss;
            ss << id;
            ss << ": in cache, valid\n";
            pthread_mutex_lock(&mutex);
            writeLog(ss.str());
            pthread_mutex_unlock(&mutex);
            send(server_fd, response.response.data(), response.response.size() + 1, 0);
            writeLogproxyserver(response);
            return;
        }else{
            std::cout << "is not fresh" << std::endl;
            std::stringstream ss;
            ss << id;
            ss << ": in cache, requires validation\n";
            pthread_mutex_lock(&mutex);
            writeLog(ss.str());
            pthread_mutex_unlock(&mutex);
            std::cout << "is validating" << std::endl;
            revalidate(client_fd, server_fd, rqst, response, mycache, id);
            return;
        }
    }
    // if it is not in cache
    else{
        std::cout << "not in cache" << std::endl;
        std::stringstream ss;
        ss << id;
        ss << ": not in cache\n";
        pthread_mutex_lock(&mutex);
        writeLog(ss.str());
        pthread_mutex_unlock(&mutex);
        //int content_len = rqst.getLength();
        //std::string origi_msg = receiveAllchunks(server_fd, rqst.origi_rqst, content_len);
        //std::cout<< content_len <<" bytes, original message from client"<<origi_msg<<std::endl;
        //if(content_len != -1){
       //  char request[origi_msg.size() + 1];
       //  strcpy(request, origi_msg.c_str());

        //send to original server
        int send_Res = send(client_fd, origi_msg.data(), origi_msg_len, 0);
        std::cout<<"msg length is:"<<origi_msg.length()<<"compute length is: "<<origi_msg_len<<std::endl;
        std::cout << "send " << send_Res << "to server" << std::endl;
        std::cout << "send request: " << origi_msg<< std::endl;
        //receive from original server
        char origi_resp[65536] = {0};
        int num = recv(client_fd, origi_resp, sizeof(origi_resp), 0);

        pthread_mutex_lock(&mutex);
        //std::cout<<"original response firstly: "<<origi_resp<<std::endl;
        std::cout << "receive: " << num <<std::endl;
        pthread_mutex_unlock(&mutex);
        if(num > 0){
            std::string resp_str(origi_resp, num);
            std::cout << "construct: " << resp_str.length()<<std::endl;
            Response response(resp_str);
            bool resp_chunk = response.isChunked();
            int total_response_len = 0;
            std::string total_response = receiveAllmsg(client_fd, response.response, resp_chunk, total_response_len);
            Response store_resp(total_response);
            send(server_fd, total_response.data(), total_response_len, 0);
            //std::cout<<"received response: "<<store_resp.response<<std::endl;
            std::cout << "code: " << store_resp.code << std::endl;
            if(!store_resp.is_nostore && store_resp.code.compare("206")!=0){
                mycache->addToCache(rqst_line, store_resp);
            }
            
        }
        else{
            perror("error port");
            //std::cerr<<"error port"<<std::endl;
        }
    //}
}

}

// void handle502(){

// }

void proxy::writeLog(std::string msg){
    //need to mutex
    file<<msg;
}

char* proxy::getCurrenttime(){
    time_t nowtime;
    time(&nowtime);
    char* time_str = ctime(&nowtime);
    return time_str;
}

void proxy::writeLogforproxyrequest(Request rqst_msg, int id){
    std::stringstream ss;
    ss << id<<": "<<"Requesting "<<'\"'<<rqst_msg.rqst_line<<'\"'<<" from "<<rqst_msg.domain << "\n";
    std::string str = ss.str();
    writeLog(str);
}

void proxy::writeLogforproxyresponse(Response rsps_msg, Request rqst, int id){
    std::stringstream ss;
    //std::cout << "TEST: "<<rsps_msg.status_line << std::endl;
    ss << id<<": "<<" Received "<<'\"'<<rsps_msg.status_line<<'\"'<<" from "<<rqst.domain << "\n";
    std::string str = ss.str();
    writeLog(str);
}

void proxy::writeLogproxyserver(Response new_rsps){
    std::stringstream ss;
    ss << "Responding"<<" "<<'\"'<<new_rsps.status_line<<'\"';
    std::string str = ss.str();
    writeLog(str);
}