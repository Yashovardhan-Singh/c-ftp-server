# Simple FTP Client-Server in C (Linux Sockets)

## Overview

This project implements a **basic FTP-like system in C using POSIX sockets**, consisting of:

* A **server** that listens for incoming connections and processes commands
* A **client** that connects to the server and allows user interaction

Note: This is **not a fully compliant FTP implementation (RFC 959)**. It uses a **single TCP connection** for both commands and data transfer, unlike real FTP which uses separate control and data channels.

## Platform & Requirements

### Platform

* Linux (tested on typical GNU/Linux environments)
* Should work on WSL or Unix-like systems with minimal changes

### Requirements

* GCC or Clang (C compiler)
* Meson build system
* Ninja (backend for Meson)
* POSIX-compliant system (for sockets, file I/O, directory handling)

Install dependencies (Ubuntu/Debian):

```bash
sudo apt install build-essential meson ninja-build
```

## Build Instructions (Meson)

### 1. Initialize build directory

```bash
meson setup build
```

### 2. Compile

```bash
meson compile -C build
```

### 3. Output binaries

After compilation:

```
build/fserver
build/fclient
```

## Example `meson.build`

```meson
project('simple-ftp', 'c')

executable('fserver', 'server.c')
executable('fclient', 'client.c')
```

## Running the Application

### Step 1: Start Server

```bash
./build/fserver
```

Output:
```
Server running on port 2121...
```

### Step 2: Start Client (in another terminal)

```bash
./build/fclient
```

## Supported Commands

| Command     | Description                    |
| ----------- | ------------------------------ |
| USER <name> | Send username (dummy auth)     |
| PASS <pass> | Send password (dummy auth)     |
| LIST        | List files in server directory |
| GET <file>  | Download file from server      |
| PUT <file>  | Upload file to server          |
| QUIT        | Disconnect                     |

## Example Session

```
ftp> USER test
331 OK

ftp> PASS test
230 Logged in

ftp> LIST
file1.txt
file2.c

ftp> PUT local.txt

ftp> GET remote.txt

ftp> QUIT
```

## Code Explanation

### Server (`server.c`)

#### Responsibilities

* Creates a TCP socket
* Binds to port `2121`
* Listens for incoming connections
* Accepts and handles one client at a time

#### Key Components

**1. Socket Setup**

```c
socket(AF_INET, SOCK_STREAM, 0);
bind(...);
listen(...);
accept(...);
```

**2. Command Processing Loop**

* Receives commands via `recv()`
* Parses using `strncmp()`
* Dispatches to handlers

**3. Command Handlers**

* `handle_list()` → uses `opendir()` + `readdir()`
* `handle_get()` → reads file and sends via socket
* `handle_put()` → receives file and writes to disk

**4. Protocol Design**

* Text-based commands
* Responses like:

  * `220` → server ready
  * `331` → username OK
  * `230` → login success
  * `150` → transfer starting
  * `226` → transfer complete

### Client (`client.c`)

#### Responsibilities

* Connects to server via TCP
* Reads user input from terminal
* Sends commands to server
* Handles file transfers

#### Key Components

**1. Connection Setup**

```c
socket(...)
connect(...)
```

**2. Interactive Loop**

* Reads input using `fgets()`
* Sends via `send()`
* Receives server response

**3. File Transfer**

* `GET` → receives file and writes locally
* `PUT` → reads file and sends to server

## Limitations

This implementation simplifies FTP significantly:

* No separate data connection (no `PORT` / `PASV`)
* No concurrency (single client at a time)
* No authentication system
* No binary-safe transfer protocol (uses EOF marker)
* No error recovery or robustness

## Possible Improvements

* Add **multi-client support** (`fork()` or threads)
* Implement **PASV mode** (true FTP behavior)
* Add **authentication system**
* Use **binary-safe transfer protocol**
* Replace blocking I/O with `select()` / `epoll`
* Add logging and error handling

## Notes

* Default port is **2121** (non-privileged, avoids root)
* Server operates in its **current working directory**
* File paths are **relative**
* Dummy auth
