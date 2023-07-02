# HTTP Caching Proxy

This repository contains the source code for a robust HTTP caching proxy server. This proxy server is designed to forward requests to the origin server on behalf of the client. It has the ability to cache responses, and when appropriate, respond with the cached copy of a resource rather than re-fetching it.

## Features

1. Handles GET, POST, and CONNECT requests.
2. Caches responses (when they are 200-OK) to GET requests.
3. Handles multiple concurrent requests effectively using multithreading.
4. Shared cache between all connections, with proper synchronization.
5. Log generation for each request in /var/log/erss/proxy.log.
6. Provides comprehensive logging for cache hit/miss, request/response from the origin server, cacheability, errors and more.

## Usage

To run the proxy server:

```bash
Copy code
sudo docker-compose up
```

This will start the proxy server inside a Docker container. The host computer's port 12345 will be connected to the proxy server.

The proxy server writes its logs to `/var/log/erss/proxy.log` inside the container. The directory `logs` in the same directory as the `docker-compose.yml` file is mounted to `/var/log/erss` in the container, allowing access to the logs from the host machine.

## Testing

A set of test cases are provided to demonstrate the functionality of the proxy in common cases as well as error/unusual situations. Tools like `netcat` and `wget` can be used to send malformed requests or responses, and generate standard web requests respectively.