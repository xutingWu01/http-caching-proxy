#include "response.hpp"

void Response::ParseStatusline(){
    try{
        std::size_t f_crlf = response.find("\r\n");
        status_line = response.substr(0, f_crlf);
        msg_withoutline = response.substr(f_crlf + 5, response.size() - f_crlf - 5);
        //std::cout << "status line:" << status_line << std::endl;
    }
    catch(std::exception & exp){
        status_line = "";
        msg_withoutline = "";
    }
}

void Response::ParseCode(){
    try{
        std::size_t f_sp1 = response.find(' ');
        std::size_t f_sp2 = response.find(' ', f_sp1 + 1);
        code = response.substr(f_sp1 + 1, f_sp2 - f_sp1 - 1);
    }
    catch(std::exception & exp){
        code = "";
    }
}

std::string Response::ParseHelper(std::string target, std::string find_start, std::string find_end){
    std::string find_str;
    std::size_t f = target.find(find_start);
    std::size_t f_end;
    if(f != std::string::npos){
        f_end = target.find(find_end, f);
        find_str = target.substr(f + find_start.size(), f_end - f - find_start.size());
        return find_str;
    }
    else{
        return "";
    }
}

void Response::ParseHeader(){
        etag = ParseHelper(response, "Etag: ", "\r\n");
        
        // std::size_t f_et = response.find("Etag: ");
        // std::size_t f_etend;
        // if(f_et != std::string::npos){
        //     f_etend = response.find("\r\n", f_et + 6);
        //     etag = response.substr(f_et + 6, f_etend - f_et - 6); 
        // }

        //extract expire string 怎么处理时间？GMT->UTC
        expire = ParseHelper(response, "Expires: ", " GMT");
        last_modify = ParseHelper(response, "Last-Modified: ", " GMT");
        //没有date header的话需要做处理
        date = ParseHelper(response, "Date: ", " GMT");
        //14.9.3 Alternatively, it MAY be specified using the max-age directive in a response. When
        // the max-age cache-control directive is present in a cached response,
        // the response is stale if its current age is greater than the age
        // value given (in seconds) at the time of a new request for that resource.
        cache_control = ParseHelper(response, "Cache-Control: ", "\r\n");
        //std::cout << "cache control is: " << cache_control << std::endl;
        if(cache_control != ""){
            max_age = ParseHelper(cache_control, "max-age=", "\r\n");
            std::size_t f_nocache = cache_control.find("no-cache");
            if(f_nocache != std::string::npos){
                is_nocache = true;
            }
            else{
                is_nocache = false;
            }
            std::size_t f_nostore = cache_control.find("no-store");
            if(f_nostore != std::string::npos){
                is_nostore = true;
            }
            else{
                is_nostore = false;
            }
            std::size_t f_revali = cache_control.find("must-revalidate");
            if(f_revali != std::string::npos){
                is_revalidate = true;
            }
            else{
                is_revalidate = false;
            }
        }
        // std::size_t f_exp = reponse.find("Expires: ");
        // std::size_t f_gmt;
        // if{f_gmt != std::string::npos}{
        //     f_gmt = response.find("GMT", f_exp);
        //     expire = response.substr(f_exp + 9, f_gmt - f_exp - 9);
        // }
        // std::size_t f_lm = response.find("Last-Modified: ");
        // std::size_t f_lmgmt;
        // if(f_lm != std::strind::npos){
        //     f_lmgmt = reponse.find("GMT", f_lmend);
        //     last_modify = reponse.substr(f_lm + 15, f_lmend - f_lm - 15);
        // }

}

time_t Response::getTime(std::string time_str){
    struct std::tm gmttime;
    //std::istringstream ss(time_str);
    strptime(time_str.c_str(), "%a, %d %b %Y %X", &gmttime);
    time_t utctime = mktime(&gmttime);
    return utctime;
}

double Response::getLifespan(){
    time_t origin_date = getTime(date);
    time_t now = time(NULL);
    //std::cout << "now time: " <<now<<std::endl;
    time_t now_utc = now - 28800; //unit: second
    //std::cout << "now utc time: "<<now_utc<<std::endl;
    return difftime(now_utc, origin_date);
}

int Response::getLength(){
    std::string content_len = ParseHelper(response, "Content-Length: ", "\r\n");
    int len;
    if(content_len != ""){
        len = stoi(content_len);
        return len;
    }
    else{
        return -1;
    }
}

bool Response::isChunked(){
    std::string trans_encode = ParseHelper(response, "Transfer-Encoding: ", "\r\n");
    if(trans_encode != ""){
        std::size_t f_chunk = trans_encode.find("chunked");
        if(f_chunk != std::string::npos){
            return true;
        }
        else{
            return false;
        }
    }
    return false;
}

bool Response::isFresh(){
    // find the max-age, compare with the age of this response
    if(max_age != ""){
        long maxage = stol(max_age);
        // get the age of the response
        double currentage = getLifespan();
        if(currentage > maxage){
            return false;
        }
        else{
            return true;
        }
    }
    else if(expire != ""){
        time_t exp_time = getTime(expire);
        time_t now = time(NULL);
        time_t now_utc = now - 28800;
        if(now_utc > exp_time){
            return false;
        }
        else{
            return true;
        }
    }
    // calculate the freshness time if no max-age and Expires
    else if(last_modify != ""){
        time_t lstmodtime = getTime(last_modify);
        time_t datetime = getTime(date);
        double freshtime = difftime(lstmodtime, datetime)/10.0;
        double currentage = getLifespan();
        if(currentage >  freshtime){
            return false;
        }
        else{
            return true;
        }
    }
    else{
        return true;
    }
        
}

void Response::getExpiretime(){
    // find the max-age, compare with the age of this response
    if(max_age != ""){
        long maxage = stol(max_age);
        // get the age of the response
        double currentage = getLifespan();
        //double expireage = currentage + maxage;
        time_t expireage = (time_t)(currentage + maxage);
        time(&expireage);
        char* expirechar = ctime(&expireage);
        std::string expire_str(expirechar);
        expire_date=expire_str;  
    }
    else if(expire != ""){
        time_t exp_time = getTime(expire);
        time(&exp_time);
        char* expirechar = ctime(&exp_time);
        std::string expire_str(expirechar);
        expire_date=expire_str; 
    }
    // calculate the freshness time if no max-age and Expires
    else if(last_modify != ""){
        time_t lstmodtime = getTime(last_modify);
        time_t datetime = getTime(date);
        double freshtime = difftime(lstmodtime, datetime)/10.0;
        time_t expireage = (time_t)(lstmodtime + freshtime);
        time(&expireage);
        char* expirechar = ctime(&expireage);
        std::string expire_str(expirechar);
        expire_date=expire_str; 
        
    }
    else{
        expire_date = "";
    }
        
}

