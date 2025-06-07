# Basic use of *_cpp-httplib_* to build web services.

---

## ✅ **Extracting Path and Query Strings**

When handling requests, you typically have access to the path and query parameters through the `Request` object.

### Example:

```cpp
#include "httplib.h"
#include <iostream>

int main() {
  httplib::Server svr;

  svr.Get(R"(/api/users/(\d+))", [](const httplib::Request &req, httplib::Response &res) {
    // Extracting path parameter
    auto user_id = req.matches[1];
    res.set_content("User ID: " + user_id.str(), "text/plain");
  });

  svr.Get("/search", [](const httplib::Request &req, httplib::Response &res) {
    // Extracting query parameters
    auto term = req.get_param_value("term");
    auto page = req.get_param_value("page");

    res.set_content("Search term: " + term + ", Page: " + page, "text/plain");
  });

  svr.listen("localhost", 8080);
}
```

### Calling these endpoints:

* **Path parameter example:**

  ```bash
  curl http://localhost:8080/api/users/42
  # Output: User ID: 42
  ```

* **Query parameter example:**

  ```bash
  curl "http://localhost:8080/search?term=ocean&page=2"
  # Output: Search term: ocean, Page: 2
  ```

---

## ✅ Path Parameters and Query-string parameters

In `cpp-httplib`, `req.path_params` and `req.params` serve different purposes 
related to how the server interprets the incoming URL. Here’s a detailed breakdown:

---

## 🧭 `req.params`: **Query String Parameters**

These are the parameters passed **after the `?` in a URL**, using standard `key=value`
format, often used in web APIs to pass filters or options.

### Example URL:

```
GET /api/data?ce=123&function=echo
```

### In this case:

* `req.params["ce"]` → `"123"`
* `req.params["function"]` → `"echo"`

### Summary:

* Extracted from the **query string**
* Automatically parsed by `cpp-httplib`
* Available regardless of route patterns

---

## 🧩 `req.path_params`: **Path Pattern Captures**

These are values extracted from the **URL path** itself using **named capture groups** in the route definition.

### Example Route:

```cpp
svr.Get(R"(/users/(?<userid>\d+))", handler);
```

### Incoming Request:

```
GET /users/42
```

### Then:

* `req.path_params["userid"]` → `"42"`

### Notes:

* Requires use of **named capture groups** in the regex pattern: `(?<name>...)`
* Only populated if the route uses such a pattern

---

## 🧠 Key Differences

| Feature        | `req.params`                         | `req.path_params`                                              |
| -------------- | ------------------------------------ | -------------------------------------------------------------- |
| Comes from     | **Query string** (after `?`)         | **Path capture groups** in regex pattern                       |
| Requires regex | ❌ No                                 | ✅ Yes (`(?<name>pattern)`)                                     |
| Example        | `/api/foo?bar=baz` → `params["bar"]` | `/user/42` matched by `/user/(?<id>\d+)` → `path_params["id"]` |
| Common use     | Filtering, options, extra inputs     | Dynamic route variables (e.g. user ID)                         |

---

## ✅ Summary

* Use `**req.params**` for standard query strings (`?key=value`).
* Use `**req.path_params**` when extracting values from the **URL path** itself via named regex captures.

## ✅ **Writing Different Types of Responses**

### 1. **String Responses**

Straightforward using `set_content`:

```cpp
svr.Get("/hello", [](const httplib::Request&, httplib::Response &res) {
  res.set_content("Hello, world!", "text/plain");
});
```

---

### 2. **Serving Files (with known size)**

When serving a file, you can use the built-in helper `set_content_provider` for efficiency:

```cpp
#include <fstream>

svr.Get("/download/report.txt", [](const httplib::Request&, httplib::Response &res) {
  std::ifstream file("report.txt", std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    res.status = 404;
    res.set_content("File not found", "text/plain");
    return;
  }

  auto size = file.tellg();
  file.seekg(0);

  res.set_content_provider(
    static_cast<size_t>(size),
    "text/plain",
    [file = std::move(file)](size_t offset, size_t length, httplib::DataSink &sink) mutable {
      char buffer[4096];
      file.seekg(offset);
      file.read(buffer, std::min(sizeof(buffer), length));
      sink.write(buffer, file.gcount());
      return true; // continue sending
    }
  );
});
```

---

### 3. **Streaming Responses**

Useful for continuously generated or large data:

```cpp
svr.Get("/stream", [](const httplib::Request&, httplib::Response &res) {
  res.set_chunked_content_provider(
    "text/plain",
    [](size_t offset, httplib::DataSink &sink) {
      // Example: Streaming numbers 0 to 9
      if (offset >= 10) {
        sink.done();  // End the stream
        return false; // Stop provider
      }

      sink.write(std::to_string(offset) + "\n");
      return true; // Continue streaming
    }
  );
});
```

To test streaming endpoint:

```bash
curl http://localhost:8080/stream
# Output:
# 0
# 1
# 2
# ...
# 9
```

---

## 🗒 **Summary of Provided Examples:**

| Feature                        | Path                      |
| ------------------------------ | ------------------------- |
| **Path Parameter Extraction**  | `/api/users/{user_id}`    |
| **Query Parameter Extraction** | `/search?term={}&page={}` |
| **Simple String Response**     | `/hello`                  |
| **Serving Files (known size)** | `/download/report.txt`    |
| **Streaming Response**         | `/stream`                 |
