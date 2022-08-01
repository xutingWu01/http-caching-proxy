#include "request.hpp"

void Request::ParseRequestLine(){
    try{
        std::size_t f_CRLF = origi_rqst.find("\r\n");
        rqst_line = origi_rqst.substr(0, f_CRLF);
        msg_withoutline = origi_rqst.substr(f_CRLF + 5, origi_rqst.size() - f_CRLF - 5);
    }
    catch(std::exception & exp){
        rqst_line = "";
        msg_withoutline = "";
    }
    //处理错误
}

void Request::ParseHeader(){
    try{
        std::size_t f_head = origi_rqst.find("\r\n\r\n");
        header = origi_rqst.substr(0, f_head);
    }
    catch(std::exception & exp){
        header = "";
    }
}

void Request::ParseMethod(){
    try{
        std::size_t f_SP = rqst_line.find(' ');
        method = rqst_line.substr(0, f_SP);
    }
    catch(std::exception & exp){
        method = "";
    }
}

void Request::ParseURL(){
    try{
        std::size_t f_space1 = rqst_line.find(' ');
        std::size_t f_space2 = rqst_line.find(' ', f_space1 + 1);
        url = rqst_line.substr(f_space1 + 1, f_space2 - f_space1 - 1);
    }
    catch(std::exception & exp){
        url = "";
    }

    //parse host and port
    try{
        std::size_t f_dslash = url.find("//");
        if(f_dslash != std::string::npos){
            f_dslash += 2;
        }
        else{
            f_dslash = 0;
        }
        std::size_t f_colon = url.find(':', f_dslash);
        std::size_t f_slash;
        if(f_colon != std::string::npos){
            domain = url.substr(f_dslash, f_colon - f_dslash);
            f_slash = url.find('/', f_colon + 1);
            if(f_slash != std::string::npos){
                port = url.substr(f_colon + 1, f_slash - f_colon - 1);
            }
            else{
                port = url.substr(f_colon + 1, url.size() - f_colon - 1);
            }
        }
        else{
            f_slash = url.find('/', f_dslash);
            if(f_slash != std::string::npos){
                domain = url.substr(f_dslash, f_slash - f_dslash);
            }
            else{
                domain = url.substr(f_dslash, url.size() - f_dslash);
            }
            port = "80";
        }
    }
    catch(std::exception & exp){
        std::cerr << "Must have host!" << std::endl;
        domain = "";
        port = "";
        return;
    }
}

int Request::getLength(){
    std::size_t f_cl = origi_rqst.find("Content-Length: ");
    if (f_cl != std::string::npos) {
        std::size_t f_crlf = origi_rqst.find("\r\n\r\n");
        int body_len = origi_rqst.size() - static_cast<int>(f_crlf) - 8;
        size_t f_end = origi_rqst.find("\r\n", f_cl);
        std::string content_len = origi_rqst.substr(f_cl + 16, f_end - f_cl - 16);
        std::stringstream ss; 
        int length;
        ss << content_len;
        ss >> length;
        return length - body_len - 4;
    }
    else{
        return -1;
    }
}

bool Request::isChunked(){
    std::size_t f_chunk = msg_withoutline.find("chunked");
    //std::cout << "finding chunk in: " << f_chunk << std::endl;
    if(f_chunk != std::string::npos){
        return true;
    }
    else{
        return false;
    }
    return false;
}

