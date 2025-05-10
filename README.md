
# A New OPeNDAP Service 
This server/service is designed to use data access and processing
components of the existing Hyrax 1.x server, but without the overhead
of a more general orchestration framework or an internal IPC layer
between the Web API implementation and the core dispatch logic.

The code uses nginx as the web interface and cpp-httplib to implement the
web API service endpoints.

## Some useful reference information

NGINX Admin guide: https://docs.nginx.com/nginx/admin-guide/

cpp-httplib: https://github.com/yhirose/cpp-httplib

Something more advanced (re: AWS): https://docs.nginx.com/nginx/deployment-guides/amazon-web-services/high-availability-network-load-balancer/.

## How to build the code

Get nginx using homebrew, etc. You must understand how/where it is
installed. 

Get httplib.h from https://github.com/yhirose/cpp-httplib.

### Build the service endpoint

In cpp_web_service, `clang++ -std=c++14 -O2 main.cpp -o cpp_service`

Start it, `./cpp_service`

Test it, `curl -i http://localhost:8080/`

Should get:
```
HTTP/1.1 200 OK
Content-Type: text/plain
Keep-Alive: timeout=5, max=100
Content-Length: 28

Hello from the C++ backend!
```

### Configure and start the nginx server

Copy the nginx.conf file from this repo, `cp nginx.conf
/opt/homebrew/etc/nginx/`

Start nginx, `brew services start nginx`

Test it, `curl -i http://localhost/api/endpoint`

Note that it's now port 80 and references `/api/endpoint`

You should get:
```
HTTP/1.1 200 OK
Server: nginx/1.27.5
Date: Sat, 10 May 2025 16:14:33 GMT
Content-Type: text/plain
Content-Length: 28
Connection: keep-alive

Hello from the C++ backend!
```

### Problems?
Check the logs in `/opt/homebrew/Cellar/nginx/1.27.5/logs`


