# _Make Web Server and Proxy_ (CS:APP)

> ### SW Jungle Week07 (12 ~ 19 May, 2022)

## TIL (Today I Learned)
### `Sun. 15`

- make TINY web server
    - make main page and adder page

![main](https://s3.us-west-2.amazonaws.com/secure.notion-static.com/8a5d572b-535d-4dbe-87ba-3dc5f45d4dd2/Untitled.png?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=AKIAT73L2G45EIPT3X45%2F20220515%2Fus-west-2%2Fs3%2Faws4_request&X-Amz-Date=20220515T121737Z&X-Amz-Expires=86400&X-Amz-Signature=4e0f2ebaa4e4cd2566a533ef97d20c087dc1cdeadb1ac1eb95b6db5999385ee5&X-Amz-SignedHeaders=host&response-content-disposition=filename%20%3D%22Untitled.png%22&x-id=GetObject)

![adder](https://s3.us-west-2.amazonaws.com/secure.notion-static.com/373d6ae7-c755-4b4b-a889-55f2a0cc659c/Untitled.png?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=AKIAT73L2G45EIPT3X45%2F20220515%2Fus-west-2%2Fs3%2Faws4_request&X-Amz-Date=20220515T121746Z&X-Amz-Expires=86400&X-Amz-Signature=aaaf3a75e721d550582d62726ca070297f99dfed28bbcf0ac5f2ad23a4080bdb&X-Amz-SignedHeaders=host&response-content-disposition=filename%20%3D%22Untitled.png%22&x-id=GetObject)

- `change main.html more fancy`
    - write some information about this page
    - make button which linked to CGI adder HTML
    - add gif file and MPG file

![Untitled](https://s3.us-west-2.amazonaws.com/secure.notion-static.com/6618fbdf-5d77-4fd2-9f34-ed005081ebff/Untitled.png?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=AKIAT73L2G45EIPT3X45%2F20220515%2Fus-west-2%2Fs3%2Faws4_request&X-Amz-Date=20220515T122022Z&X-Amz-Expires=86400&X-Amz-Signature=51709000452a7a75d9940333ad151cdae0c69a8e249d3628d775245a4ab42c30&X-Amz-SignedHeaders=host&response-content-disposition=filename%20%3D%22Untitled.png%22&x-id=GetObject)

- `CS:APP HW11.7`
    - add MPG file type

- `CS:APP HW11.10`
    - make HTML for CGI adder function

![CGI](https://s3.us-west-2.amazonaws.com/secure.notion-static.com/b48d5dd5-52cc-4b12-9c75-8810afa30968/Untitled.png?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=AKIAT73L2G45EIPT3X45%2F20220515%2Fus-west-2%2Fs3%2Faws4_request&X-Amz-Date=20220515T121830Z&X-Amz-Expires=86400&X-Amz-Signature=4cdcbb1d205593f89ca7a08ef6410bda6c055b51b8e66ddf60c878e5aab65528&X-Amz-SignedHeaders=host&response-content-disposition=filename%20%3D%22Untitled.png%22&x-id=GetObject)

### `Sat. 14`

- Study CS:APP Chapter 11. Network Programming
    - especially study about Socket (Ch 11.4)
        - socket function
        - echo routine
        - web - MIME, HTTP transaction, static and dynamic contents
    

### `Fri. 13`
- Study CS:APP Chapter 10. System-Level I/O
- Study CS:APP Chapter 11. Network Programming
    - especially study Ch 11.1 ~ 11.3


### `Thu. 12 May` - Week07 Start

- clone web-server to my [github repo.](https://github.com/latteishorse/webproxy-jungle)
- setting test env.
    - aws ec2 (ubuntu 20.04)
- Study about Network
    - Network Basics
        - throughput, latency, network topology
        - Network Command
        - Network Protocol
    - TCP/IP 4 Layer, OSI 7 Layer
    - IP Address
    - HTTP
    
---
<details>
<summary>Original webproxy README</summary>
<div markdown="1">

``` txt
####################################################################
# CS:APP Proxy Lab
#
# Student Source Files
####################################################################

This directory contains the files you will need for the CS:APP Proxy
Lab.

proxy.c
csapp.h
csapp.c
    These are starter files.  csapp.c and csapp.h are described in
    your textbook. 

    You may make any changes you like to these files.  And you may
    create and handin any additional files you like.

    Please use `port-for-user.pl' or 'free-port.sh' to generate
    unique ports for your proxy or tiny server. 

Makefile
    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make handin" to create the tarfile that you will be handing
    in. You can modify it any way you like. Your instructor will use your
    Makefile to build your proxy from source.

port-for-user.pl
    Generates a random port for a particular user
    usage: ./port-for-user.pl <userID>

free-port.sh
    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: ./free-port.sh

driver.sh
    The autograder for Basic, Concurrency, and Cache.        
    usage: ./driver.sh

nop-server.py
     helper for the autograder.         

tiny
    Tiny Web server from the CS:APP text
```
</div>
</details>

*This page was most recently updated on May 15th, 2022*