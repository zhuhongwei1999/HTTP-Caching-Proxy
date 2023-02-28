#include "server.h"
#include "utils.h"

ServerResponse::ServerResponse(const std::string & response_msg) {
  message = response_msg;
  std::vector<std::string> lines = splitResponse(response_msg, "\r\n");
  if (lines.empty()) return;
  response_line = lines[0];
  std::vector<std::string> status_line = splitResponse(lines[0], " ");
  if (status_line.size() < 2) return;
  status_code = std::stoi(status_line[1]);
  int i = 1;
  while (i < lines.size() && lines[i] != "") {
    std::vector<std::string> header = splitResponse(lines[i], ": ");
    if (header.size() == 2) {
      headers[header[0]] = header[1];
    }
    i++;
  }
  i++;
  while (i < lines.size()) {
    body += lines[i] + "\r\n";
    i++;
  }
}

std::vector<std::string> ServerResponse::splitResponse(const std::string & s, const std::string & delim) {
  std::vector<std::string> tokens;
  size_t start = 0, end = 0;
  while ((end = s.find(delim, start)) != std::string::npos) {
    tokens.push_back(s.substr(start, end - start));
    start = end + delim.length();
  }
  tokens.push_back(s.substr(start));
  return tokens;
}

bool ServerResponse::isCacheable(int client_id, int &client_fd) {
  if (status_code != 200) {
    send(client_fd, &response_line, response_line.size(), 0);
    pthread_mutex_lock(&mutex);
    logFile<<client_id<<": Responding \""<<response_line<<"\""<<endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }
  std::map<std::string, std::string>::iterator it;
  it = headers.find("Cache-Control");
  if (it != headers.end()) {
    if (it->second == "no-store"){
      pthread_mutex_lock(&mutex);
      logFile<<client_id<<": NOTE Cache-Control: no-store"<<endl;
      logFile<<client_id<<": not cacheable because the response must not be cached in any form."<<endl;
      pthread_mutex_unlock(&mutex);
      return false;
    }
    if (it->second.find("max-age=") != std::string::npos) {
      size_t eq_pos = it->second.find('=');
      std::string max_age = it->second.substr(eq_pos + 1);
      if (max_age == "0"){
        pthread_mutex_lock(&mutex);
        logFile<<client_id<<": NOTE max-age=0"<<endl;
        logFile<<client_id<<": not cacheable because the the response should not be cached for client to ensure that it has the latest version."<<endl;
        pthread_mutex_unlock(&mutex);
        return false;
      }
    }
  }
  it = headers.find("Expires");
  if (it != headers.end()) {
    std::time_t expiration_time = parseDate(it->second);
    if (expiration_time <= std::time(NULL)) {
      pthread_mutex_lock(&mutex);
      logFile<<client_id<<": not cacheable because the the response expired."<<endl;
      pthread_mutex_unlock(&mutex);
      return false;
    }
  }
  return true;
}

std::time_t ServerResponse::parseDate(const std::string& date_str) {
  struct tm tm;
  if (strptime(date_str.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &tm) == NULL) {
    return -1;
  } 
  else {
    return std::mktime(&tm);
  }
}

std::time_t ServerResponse::parseMaxAge() {
  if (headers.count("Cache-Control")) {
    std::string cache_control = headers["Cache-Control"];
    if (cache_control.find("max-age=") != std::string::npos) {
      size_t eq_pos = cache_control.find('=');
      std::string max_age = cache_control.substr(eq_pos + 1);
      int max_age_sec = std::stoi(max_age);
      return max_age_sec;
    }
  }
  return -1;
}

std::time_t ServerResponse::parseExpiretime() {
  if (headers.count("Expires")) {
    std::string expires_str = headers["Expires"];
    return parseDate(expires_str);
  }
  return -1;
}

std::time_t ServerResponse::getExpiretime() {
  std::time_t expire_time = parseExpiretime();
  if (expire_time == -1) {
    std::time_t date = parseDate(headers["Date"]);
    std::time_t max_age = parseMaxAge();
    expire_time = date + max_age;
  }
  return expire_time;
}