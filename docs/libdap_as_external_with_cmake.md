
**Where will this library be installed (e.g., given: install(FILES ${DAP_HDR} ${DAP4_ONLY_HDR} ${CLIENT_HDR} ${SERVER_HDR}
DESTINATION include/libdap))?**

The CMake command you highlighted:

```cmake
install(FILES ${DAP_HDR} ${DAP4_ONLY_HDR} ${CLIENT_HDR} ${SERVER_HDR}
        DESTINATION include/libdap)
```

installs the listed header files into a subdirectory named:

```
<install-prefix>/include/libdap
```

---

### **What is `<install-prefix>`?**

The installation prefix is typically set by the CMake variable `CMAKE_INSTALL_PREFIX`. By default, this is usually:

* `/usr/local` on Unix-like systems.
* `C:\Program Files\<project>` on Windows.

---

### **For Example:**

* If `CMAKE_INSTALL_PREFIX` is `/usr/local` (the default), the files go to:

```
/usr/local/include/libdap
```

* If configured with a custom prefix (e.g., `/opt/libdap`):

```
/opt/libdap/include/libdap
```

---

### **How to Change the Prefix:**

You set it explicitly when running CMake configuration:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/desired/path ..
```

---

### **Summary of Installation Directories (typical defaults):**

| CMake Command                                  | Default install location |
| ---------------------------------------------- | ------------------------ |
| `install(TARGETS ... LIBRARY DESTINATION lib)` | `/usr/local/lib`         |
| `install(TARGETS ... RUNTIME DESTINATION bin)` | `/usr/local/bin`         |
| `install(FILES ... DESTINATION include/...)`   | `/usr/local/include/...` |

---

### **Quick Check:**

If unsure, run CMake with your custom prefix explicitly set:

```bash
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/my/prefix ..
make
make install
```

Then you'll find your files under `/my/prefix/include/libdap`.


**I'm adding this library into a new more complex github repo as an external library. I want to build it separately, once,
or when updated, but I don't want to install it outside my new repo/project. What's the best way to do that and be able
to easily use the library in the new repo/project?**

The cleanest, easiest way to do this is:

* **Build your external library separately within your new repository** using a consistent build directory.
* **Use CMake’s built-in support for imported targets or `add_subdirectory()`** to link the built library easily without
  a global installation.

---

## Recommended Approach: Using an External Build with Imported Targets

This is a good workflow:

### **Step 1: Directory structure**

Organize like this in your repo:

```
new-project/
├── external/
│   └── libdap/            # external project source (via git submodule or subtree)
├── build/
│   ├── libdap/            # build output from libdap (separate)
│   └── my-app/            # your project's own build
├── src/
├── CMakeLists.txt
└── README.md
```

---

### **Step 2: Building external library (`libdap`) once**

Perform a separate build in your main repo, targeting a custom local installation within your repo:

```bash
mkdir -p build/libdap
cd build/libdap
cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/install ../../external/libdap
make -j
make install
```

Now your `libdap` library, headers, and config files are installed only under:

```
new-project/build/libdap/install/
```

---

### **Step 3: Easily Using the Installed libdap from your new project**

In your main project's `CMakeLists.txt`, add this clean find logic:

```cmake
cmake_minimum_required(VERSION 3.10)
project(my-app)

# Find the locally built libdap
set(libdap_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/build/libdap/install")

# Tell CMake where to find config files
list(APPEND CMAKE_PREFIX_PATH "${libdap_ROOT}")

find_package(libdap REQUIRED CONFIG)

# Now link libdap easily in your targets
add_executable(my-app src/main.cpp)
target_link_libraries(my-app PRIVATE libdap::libdap)
```

Note:

* Assumes `libdap` has been set up properly for CMake install with `libdapConfig.cmake` or uses exported targets.

---

## Alternative: Using `add_subdirectory()` (Less recommended if external)

If you prefer everything to build together each time (less optimal), you can directly include the external library via `add_subdirectory()`:

```cmake
add_subdirectory(external/libdap)
target_link_libraries(my-app PRIVATE libdap)
```

But this re-builds every time. Usually better for small dependencies, not larger external libraries.

---

## Recommended for Stability and Speed

* **Preferred:**
  **Build once** separately within your repo, then reference via imported targets (`find_package` and installed config files).

* **Second best:**
  `add_subdirectory()` directly (useful during development but not efficient for large external libraries).

This structure keeps the external dependency neatly encapsulated and easily updated when needed.
