For the requirements—high performance, lightweight dispatch, leveraging existing C++ code, and comfort with standard web servers (Apache httpd, nginx)—there are several good options. Here's an overview of the top choices:

### 1. **Crow**

* **Description:** A lightweight, header-only C++ micro-web framework designed for simplicity and high performance. Inspired by Python's Flask.
* **Pros:**

    * Extremely simple and straightforward syntax.
    * Minimal dependencies; integrates easily with existing C++ code.
    * Excellent performance; minimal overhead.
* **Cons:**

    * Limited built-in functionality (which aligns with your requirement).
    * Less active than some competitors.
* **Web Server Integration:** Typically standalone or behind nginx.
* **Website:** [Crow GitHub](https://github.com/CrowCpp/Crow)

---

### 2. **Drogon**

* **Description:** A modern asynchronous HTTP application framework, extremely fast and scalable.
* **Pros:**

    * Highly performant; leverages asynchronous non-blocking I/O.
    * Good documentation, active development.
    * Good middleware support.
* **Cons:**

    * Slightly higher complexity compared to Crow.
* **Web Server Integration:** Can serve standalone, typically used with reverse proxies like nginx.
* **Website:** [Drogon GitHub](https://github.com/drogonframework/drogon)

---

### 3. **Cpp-httplib**

* **Description:** Single-header HTTP/HTTPS library providing an extremely lightweight HTTP server/client implementation.
* **Pros:**

    * Very small footprint, easy integration.
    * No external dependencies.
    * Ideal if you just need routing endpoints to existing logic.
* **Cons:**

    * Minimal features; no built-in high-level abstractions beyond basic HTTP dispatch.
* **Web Server Integration:** Usually behind nginx or Apache reverse proxy.
* **Website:** [Cpp-httplib GitHub](https://github.com/yhirose/cpp-httplib)

---

### 4. **Pistache**

* **Description:** Modern C++ REST framework providing HTTP and REST API building blocks.
* **Pros:**

    * Fast, lightweight, and actively maintained.
    * Strong community and support.
* **Cons:**

    * Less mature than some other projects.
    * Dependency management can be slightly more involved.
* **Web Server Integration:** Typically used standalone or behind nginx.
* **Website:** [Pistache GitHub](https://github.com/pistacheio/pistache)

---

### 5. **Restinio**

* **Description:** C++ REST framework built on Boost.Asio, focusing heavily on asynchronous IO for high scalability.
* **Pros:**

    * Asynchronous event-driven architecture.
    * Fast and highly scalable.
    * Good documentation and stable.
* **Cons:**

    * Requires familiarity with Boost (or standalone Asio).
* **Web Server Integration:** Can run standalone or proxied via nginx.
* **Website:** [Restinio GitHub](https://github.com/Stiffstream/restinio)

---

### Comparison & Recommendations

| Framework       | Performance | Complexity | Dependencies | Maturity/Community |
| --------------- | ----------- | ---------- | ------------ | ------------------ |
| **Crow**        | High        | Simple     | Minimal      | Moderate           |
| **Drogon**      | Very High   | Moderate   | Few          | High               |
| **Cpp-httplib** | High        | Simple     | None         | High               |
| **Pistache**    | High        | Moderate   | Few          | Moderate           |
| **Restinio**    | Very High   | Moderate   | Boost/Asio   | Moderate           |

Given your stated preferences:

* **Minimal features (just HTTP dispatching)**
* **High performance, lightweight**
* **Easy integration with existing C++ code**
* **Comfortable with Apache/nginx as reverse proxies**

### **Top Recommendations:**

* **Cpp-httplib:** Ideal if you want absolute minimalism and simplicity; easy to integrate behind nginx/apache.
* **Crow:** Very simple, slightly richer than httplib, excellent ease-of-use.
* **Drogon:** If you foresee growing requirements for scalability or asynchronous processing.

### Suggested Stack Setup:

* **Use nginx (or Apache httpd)** as a lightweight reverse proxy in front of your C++ server.
* Your C++ application runs standalone on localhost, nginx proxies requests, managing SSL/TLS, static assets, caching, load-balancing, etc.

### Example Lightweight Setup (nginx + Cpp-httplib):

```cpp
// Simple Cpp-httplib server
#include "httplib.h"

int main(void) {
  httplib::Server svr;

  svr.Get("/api/endpoint", [](const httplib::Request&, httplib::Response& res) {
    res.set_content("Hello from C++ backend!", "text/plain");
  });

  svr.listen("localhost", 8080);
}
```

Then, nginx reverse proxy config:

```nginx
server {
  listen 80;
  server_name example.com;

  location /api/ {
    proxy_pass http://localhost:8080/;
  }
}
```

---

### Conclusion:

For your particular scenario—highly performant, minimal HTTP dispatching, easily integrated into existing C++ software, and comfortable integration with web servers—the best immediate candidates are:

* **Cpp-httplib** (minimal, easy, no dependencies)
* **Crow** (straightforward micro-framework)
* **Drogon** (powerful, scalable async handling if you might expand features)

If absolute simplicity and low-overhead performance are paramount, **Cpp-httplib** or **Crow** would be your best starting points.
