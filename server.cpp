#include "server.h"
#include "utils.h"

ServerResponse::ServerResponse(const std::string & response_msg) {
  message = response_msg;
  std::vector<std::string> lines = split_response(response_msg, "\r\n");
  if (lines.empty()) return;
  std::vector<std::string> status_line = split_response(lines[0], " ");
  if (status_line.size() < 2) return;
  status_code = std::stoi(status_line[1]);
  int i = 1;
  while (i < lines.size() && lines[i] != "") {
    std::vector<std::string> header = split_response(lines[i], ": ");
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

std::vector<std::string> ServerResponse::split_response(const std::string & s, const std::string & delim) {
  std::vector<std::string> tokens;
  size_t start = 0, end = 0;
  while ((end = s.find(delim, start)) != std::string::npos) {
    tokens.push_back(s.substr(start, end - start));
    start = end + delim.length();
  }
  tokens.push_back(s.substr(start));
  return tokens;
}

bool ServerResponse::isCacheable() {
  if (status_code != 200) {
    return false;
  }
  std::map<std::string, std::string>::iterator it;
  it = headers.find("Cache-Control");
  if (it != headers.end()) {
    if (it->second == "no-store") return false;
    if (it->second.find("max-age=") != std::string::npos) {
      size_t eq_pos = it->second.find('=');
      std::string max_age = it->second.substr(eq_pos + 1);
      if (max_age == "0") return false;
    }
  }
  it = headers.find("Expires");
  if (it != headers.end()) {
    std::time_t expiration_time = parse_date(it->second);
    if (expiration_time <= std::time(NULL)) {
      return false;
    }
  }
  return true;
}

std::time_t ServerResponse::parse_date(const std::string& date_str) {
  struct tm tm;
  if (strptime(date_str.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &tm) == NULL) {
    return -1;
  } else {
    return std::mktime(&tm);
  }
}

std::time_t ServerResponse::parse_max_age() {
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