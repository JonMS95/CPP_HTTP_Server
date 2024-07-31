# CPP_HTTP_Server: an HTTP(S) server written in C++ 🌐
This project aims to learn about HTTP protocol and build an HTTP(S) server, so it has been created for both educational and practical purposes.
Although it is still in its early stages, it can already be used to serve simple websites.

## Table of contents 🗂️
* [**Introduction** 📑](#introduction)
* [**Features** 🌟](#features)
* [**Prerequisites** 🧱](#prerequisites)
* [**Installation instructions** 📓](#installation-instructions)
  * [**Download and compile** ⚙️](#download-and-compile)
  * [**Create certificate and private key** 🔐](#create-certificate-and-private-key)
  * [**Create sample webpage** 📃](#create-sample-webpage)
  * [**Compile and run test** 🧪](#compile-and-run-test)
* [**Usage** 🖱️](#usage)
* [**To do** ☑️](#to-do)
* [**Related documents** 🗄️](#related-documents)


## Introduction <a id="introduction"></a> 📑
The initial aim of this project was to have a deeper dive into some concepts about the HTTP protocol. It's pretty basic by now, but it supports the minimum required services for a compliant HTTP server
as stated in [RFC 9110](https://datatracker.ietf.org/doc/html/rfc9110#name-methods). As HTTP protocol runs over TCP/TLS, it depends on a TCP server socket, so [C_Server_Socket](https://github.com/JonMS95/C_Server_Socket)
to establish the TCP connection. In fact, this library is nothing but a [TCP server socket interaction function](https://github.com/JonMS95/C_Server_Socket?tab=readme-ov-file#usage).

Building this project has involved a deep understanding of:
* C/C++ programming
* Bash scripting
* TCP/IP protocol stack
* HTTP(S) protocol
* HTML
* Usage of tools such as Postman and Wireshark
* Dependency management
* Parallel-running processes
* Tests
* Linux OS operation

and the list goes on and on.

As the project still is in its first stages, many things are yet to be improved, and some methods still need to be added (like PUT, POST and DELETE among others).


## Features <a id="features"></a> 🌟
The current list of supported methods is the following:
* GET
* HEAD
* TRACE

The aim is to lengthen this list soon with methods that could allow users to modify server-hosted resources (PUT, DELETE, POST and so on).

On top of the mentioned above, this server allows the user to choose a website of its own choice to host. The only thing that should be done is to provide
the path to the directory in which the resources to be served are hosted.

As far as security is concerned, TLS (and HTTPS as a consequence) can be activated by simply providing a certificate and a private key.

In order to get some knowledge about how to use the library alongside its options, go to [Usage](#usage).


## Prerequisites <a id="prerequisites"></a> 🧱
By now, the application has only been tested in POSIX-compliant Linux distros. In these, many of the dependencies below may already come installed in the OS.
In the following list, the minimum versions required (if any) by the library are listed.

| Dependency                   | Purpose                                 | Minimum version |
| :--------------------------- | :-------------------------------------- |:-------------: |
| [gcc][gcc-link]              | Compile                                 |11.4            |
| [Bash][bash-link]            | Execute Bash/Shell scripts              |4.4             |
| [Make][make-link]            | Execute make file                       |4.1             |
| [Git][git-link]              | Download GitHub dependencies            |2.34.1          |
| [Xmlstarlet][xmlstarlet-link]| Parse [configuration file](config.xml)  |1.6.1           |
| [OpenSSL][openssl-link]      | Allow TLS                               |3.0.2           |

[gcc-link]:        https://gcc.gnu.org/
[bash-link]:       https://www.gnu.org/software/bash/
[make-link]:       https://www.gnu.org/software/make/
[git-link]:        https://git-scm.com/
[xmlstarlet-link]: https://xmlstar.sourceforge.net/
[openssl-link]:    https://www.openssl.org/

Except for Make, Bash and OpenSSL, the latest version of each of the remaining dependencies will be installed automatically if they have not been found beforehand. 

In any case, installing **_Xmlstarlet_** before executing any of the commands below is strongly recommended. Otherwise, it can lead to error since make file
contains some calls to it at the top. If that happens, just repeat the process (Xmlstarlet would have been already installed).

On top of the ones listed above, there are some *JMS* dependencies (libraries that were also made by myself) that are required for both the library and the test executable to be built,
(although these are managed by the library itself, so there is no need to download them manually). The required version for each of them is specified by the [config.xml](config.xml) file.

| Dependency                                                              | Purpose                                  |
| :---------------------------------------------------------------------- | :--------------------------------------- |
| [C_Common_shell_files](https://github.com/JonMS95/C_Common_shell_files) | Process [configuration file](config.xml) |
| [C_Severity_Log](https://github.com/JonMS95/C_Severity_Log)             | Show logs                                |
| [C_Arg_Parse](https://github.com/JonMS95/C_Arg_Parse)                   | Parse command line arguments             |
| [C_Server_Socket](https://github.com/JonMS95/C_Server_Socket)           | Provide required TCP/IP connection       |
| [HTML_tutorial](https://github.com/JonMS95/HTML_tutorial)               | A sample webpage to test the server with |


## Installation instructions <a id="installation-instructions"></a> 📓
### Download and compile <a id="download-and-compile"></a> ⚙️
1. In order to download the repo, just clone it from GitHub to your choice path by using the [link](https://github.com/JonMS95/C_Server_Socket) to the project.

```bash
cd /path/to/repos
git clone https://github.com/JonMS95/C_Server_Socket
```

**Note**: by default, the path to the repository should be found within a directory (found in Desktop) called "scripts" (it's to say, **_~/Desktop/scripts_**). In order to modify this, change the paths specified in [config.xml](config.xml). If dependencies are not found in the specified path, the path itself would be created, then dependencies will be compiled there. 

2. Then navigate to the directory in which the repo has been downloaded, and set execution permissions to every file just in case they have not been sent beforehand.

```bash
cd /path/to/repos/C_Server_Socket

find . -type f -exec chmod u+x {} +
```

3. For the library to be built (i.e., clean, download dependencies and compile), just type the following:

```bash
make
```

The result of the line above will be a new API directory (which will match the used version). Within it, a *.h* and a *.so* file will be found.
- **/path/to/repos/CPP_HTTP_Server/API**
  - **vM_m**
    - **Dynamic_libraries**
      - **_libHttpServer.so.M.m_**
    - **Header_files**
      - **_HttpServer_api.h_**

Where **_M_** and **_m_** stand for the major and minor version numbers.
**_HttpServer_api.h_** could also be found in **_/path/to/repos/CPP_HTTP_Server/Source_files/HttpServer_api.h_** although it may differ depending on the version.

### Create certificate and private key <a id="create-certificate-and-private-key"></a> 🔐
In order to provide security (HTTP**S** instead of simply HTTP), the applcation layer should run over TLS. To create a proper certificate as well as a private key,
just follow the rules described in the [C_Server_Socket README document](https://github.com/JonMS95/C_Server_Socket?tab=readme-ov-file#create-certificate-and-private-key).


### Create sample webpage <a id="create-sample-webpage"></a> 📃
For the server to serve a webpage, the webpage itself alongside all of its resources should be created first. For testing purposes, a sample webpage has been created first
and it will be imported as a dependency on the test stage: [HTML_tutorial](https://github.com/JonMS95/HTML_tutorial). Of course, a path to any other webpage hosted in the
same host as the server can be used.


### Compile and run test <a id="compile-and-run-test"></a> 🧪
For the test executable file to be compiled and executed, use:

```bash
make test
```

Again, the one below is the path to the generated executable file:
- **/path/to/repos/CPP_HTTP_Server/Tests**
  - **Executable_files**
      - **_main_**
  - Source_files
  - Dependency_files


## Usage <a id="usage"></a> 🖱️
The following is the main server socket function prototype as found in the **_header API file_** (_/path/to/repos/CPP_HTTP_Server/API/vM_m/Header_files/ServerSocket_api.h_) or in the [repo file](Source_files/ServerSocket_api.h).

A couple of functions defined as static methods of the HttpInteract class have to be used to set up and run the server. First, the path to the webpage resources should be provided:

```c
static void SetPathToResources(const char* path_to_resources);
```

As explained before, this project is a callback function mentioned to be used as an input parameter for ServerSocketRun [C_Server_Socket](https://github.com/JonMS95/C_Server_Socket?tab=readme-ov-file#usage).

```c
static int InteractFn(int client_socket);
```

Which can be used as follows:

```c
HttpInteract::SetPathToResources(path_to_resources);

ServerSocketRun(server_port             ,
                max_clients_num         ,
                concurrency_enabled     ,
                false                   ,
                true                    ,
                true                    ,
                rx_timeout_s            ,
                rx_timeout_us           ,
                tx_timeout              ,
                tx_timeout_us           ,
                secure_connection       ,
                path_cert               ,
                path_pkey               ,
                HttpInteract::InteractFn);
```

Where each parameter used has been previously defined.

For reference, a proper API usage example has been provided on the [test source file](Tests/Source_files/main.c).
As this one uses [**C_Arg_Parse library**](https://github.com/JonMS95/C_Arg_Parse), input parameters can be provided by using a command-line interface.
An example of CLI usage is provided in the [**Shell_files/test.sh**](Shell_files/test.sh) file.


## To do <a id="to-do"></a> ☑️
- [ ] Add support for more methods: POST; PUT, DELETE, PATCH and OPTIONS
- [ ] Add more status codes to make the communication more understandable
- [ ] Instead of storing the whole response within a string, use a pointer to the target resource and send it later


## Related Documents <a id="related-documents"></a> 🗄️
* [LICENSE](LICENSE)
* [CONTRIBUTING.md](Docs/CONTRIBUTING.md)
* [CHANGELOG.md](Docs/CHANGELOG.md)

