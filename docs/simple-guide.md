
# Deploying a Lightweight C++ Web Service (cpp-httplib) with nginx on macOS

This guide walks you through deploying a minimal C++ web service using the lightweight **cpp-httplib** framework, behind an **nginx** reverse proxy, on macOS.

Written by ChatGPT with corrections by jhrg, 5/10/25

## Table of Contents
- [Install Dependencies](#install-dependencies)
- [Set Up C++ Environment](#set-up-c-environment)
- [Write the C++ Web Service](#write-the-c-web-service)
- [Compile and Run the Service](#compile-and-run-the-service)
- [Configure nginx as a Reverse Proxy](#configure-nginx-as-a-reverse-proxy)
- [Manage Your Service](#manage-your-service)
- [Optional: Configure HTTPS](#optional-configure-https)

---

## Install Dependencies

### Xcode Command-line Tools

Ensure Xcode command-line tools are installed:

```bash
xcode-select --install
````

### nginx (via Homebrew)

Install **nginx**:

```bash
brew install nginx
```

---

## Set Up C++ Environment

Create a project directory:

```bash
mkdir cpp_web_service
cd cpp_web_service
```

Download the single-header **cpp-httplib** library:

```bash
curl -O https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
```

---

## Write the C++ Web Service

Create `main.cpp`:

```cpp
#include "httplib.h"

int main(void) {
  httplib::Server svr;

  // NB: I think this should be just '/'. See the "... Reverse Proxy"
  // section below. The cpp-httplib 'endpoint' is going to be accessed as
  // http://localhost/api/endpoint. jhrg 5/10/25
  
  svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
    res.set_content("Hello from C++ backend!", "text/plain");
  });

  // Listen on localhost, port 8080
  svr.listen("localhost", 8080);
}
```

---

## Compile and Run the Service

Compile using `clang++` (g++ is fine, too. jhrg, 5/10/25):

```bash
clang++ -std=c++14 -O2 main.cpp -o cpp_service
```

Run your service:

```bash
./cpp_service
```

In another terminal, verify it's running:

```bash
curl http://localhost:8080/		# Fixed teh URL. jhrg 5/10/25
# Output: Hello from C++ backend!
```

---

## Configure nginx as a Reverse Proxy

Edit nginx configuration (`/usr/local/etc/nginx/nginx.conf`):

NB: This is in `/opt/homebrew/etc/nginx/` on an M4. jhrg 5/10/25

```nginx
http {
    server {
        listen 80;
        server_name localhost;

        location /api/endpoint {
            proxy_pass http://localhost:8080/;
        }
    }
}
```

Restart nginx to apply the changes:

```bash
brew services restart nginx
```

Verify nginx is correctly proxying:

```bash
curl http://localhost/api/endpoint
# Output: Hello from C++ backend!
```

---

## Manage Your Service

NB: I have not worked through this part yet, YMMV. jhrg 510/25

To simplify running your service, create a `start_service.sh` script:

```bash
#!/bin/bash
cd "$(dirname "$0")"
./cpp_service
```

Make it executable:

```bash
chmod +x start_service.sh
```

### Using Supervisor (Recommended)

Install Supervisor for automated service management:

```bash
brew install supervisor
```

Edit `/usr/local/etc/supervisord.ini` and add:

```ini
[program:cpp_service]
command=/path/to/cpp_web_service/cpp_service
directory=/path/to/cpp_web_service
autostart=true
autorestart=true
stdout_logfile=/tmp/cpp_service.log
stderr_logfile=/tmp/cpp_service_error.log
```

Start Supervisor:

```bash
brew services start supervisor
```

Your C++ web service will now run continuously and restart automatically if needed.

---

## Optional: Configure HTTPS

Generate a self-signed certificate (for development):

```bash
openssl req -x509 -newkey rsa:4096 -nodes \
  -keyout /path/to/key.pem \
  -out /path/to/cert.pem \
  -days 365
```

Modify your nginx configuration to enable HTTPS (`/usr/local/etc/nginx/nginx.conf`):

```nginx
http {
    server {
        listen 443 ssl;
        server_name localhost;

        ssl_certificate     /path/to/cert.pem;
        ssl_certificate_key /path/to/key.pem;

        location /api/ {
            proxy_pass http://localhost:8080/;
        }
    }
}
```

Restart nginx:

```bash
brew services restart nginx
```

Verify HTTPS setup:

```bash
curl -k https://localhost/api/endpoint
# Output: Hello from C++ backend!
```

(*Note: For production deployments, use certificates from Let's Encrypt or another trusted CA.*)

---

## Summary

Your deployment now follows this architecture:

```
[Client Browser] ↔ [nginx Reverse Proxy (80/443)] ↔ [cpp-httplib Service (localhost:8080)]
```

This setup gives you a minimal, high-performance, and easy-to-maintain web service stack leveraging existing C++ code.
