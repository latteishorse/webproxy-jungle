#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000 // 최대 캐시 크기
#define MAX_OBJECT_SIZE 102400 // 오브젝트 사이즈
#define LRU_MAGIC_NUMBER 9876 // Least Recently Used
// LRU: 가장 오랫동안 참조되지 않은 페이지를 교체하는 기법

#define CACHE_OBJS_COUNT 5

static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
static const char *requestline_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";
static const char *host_hdr_format = "Host: %s\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";

static const char *host_key = "Host";
static const char *connection_key = "Connection";
static const char *proxy_connection_key = "Proxy-Connection";
static const char *user_agent_key = "User-Agent";

void *thread(void *vargsp); 
void doit(int connfd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio);
int connect_endServer(char *hostname, int port, char *http_header);

// cache function
void cache_init();
int cache_find(char *url);
void cache_uri(char *uri, char *buf);

void readerPre(int i);
void readerAfter(int i);
//-----

typedef struct 
{
  char cache_obj[MAX_OBJECT_SIZE];
  char cache_url[MAXLINE];
  int LRU;
  int isEmpty;

  int readCnt;  // 카운트 -> 최근 방문 0, 방문 안할수록 +1
  sem_t wmutex;  // protects accesses to cache
  sem_t rdcntmutex;  // protects accesses to readcnt
}cache_block;


typedef struct
{
  cache_block cacheobjs[CACHE_OBJS_COUNT];  // ten cache blocks
  int cache_num;
}Cache;

Cache cache;

/*
* argc: 메인 함수에 전달 되는 데이터의 수
* argv: 메인 함수에 전달 되는 실질적인 정보
*/
int main(int argc, char **argv) {
  int listenfd, connfd; // listen 식별자, connfd 식별자
  socklen_t clientlen; // socklen_t는 소켓 관련 매개 변수에 사용되는 것으로 길이 및 크기 값에 대한 정의를 내려준다
  char hostname[MAXLINE], port[MAXLINE]; // hostname: 접속한 클라이언트 ip, port: 접속한 클라이언트 port
  pthread_t tid;
  struct sockaddr_storage clientaddr; // 어떤 타입의 소켓 구조체가 들어올지 모르기 때문에 충분히 큰 소켓 구조체로 선언

  cache_init();

  if (argc != 2) {
    // fprintf: 출력을 파일에다 씀. strerr: 파일 포인터
    fprintf(stderr, "usage: %s <port> \n", argv[0]);
    exit(1);  // exit(1): 에러 시 강제 종료
  }
  Signal(SIGPIPE, SIG_IGN);
  // 비정상 종료된 소켓에 접근할시 발생하는 프로세스 종료 signal 을 무시하는 함수
  // 나머지 클라이언트들과의 연결 중인 전체 프로세스를 유지해야하기때문. 
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s %s).\n", hostname, port);

    // 첫 번째 인자 *thread: 쓰레드 식별자
    // 두 번째: 쓰레드 특성 지정 (기본: NULL)
    // 세 번째: 쓰레드 함수
    // 네 번째: 쓰레드 함수의 매개변수
    Pthread_create(&tid, NULL, thread, (void *)connfd);

    // pthread 에서 해줄테니 main 에서 doit 하거나 close 하지 않는다. 
    // doit(connfd);
    // Close(connfd);
  }
  return 0;
}

void *thread(void *vargsp) {
  int connfd = (int)vargsp;
  // 해당 쓰레드를 원래 프로세스로부터 분리한다. 
  // 프로세스로 부터 분리되면 특정 쓰레드가 끝날때 자동으로 메모리를 반환해서 
  // 다른 쓰레드가 해당 메모리를 사용할 수 있도록 만든다. 

  Pthread_detach(pthread_self());
  doit(connfd);
  Close(connfd);
}

void doit(int connfd) {
  int end_serverfd;

  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; // buf: request 헤더 정보 담을 공간
  char endserver_http_header[MAXLINE]; // 서버에 보낼 헤더 정보를 담을 공간 
  char hostname[MAXLINE], path[MAXLINE]; //hostname: IP담을 공간, path: 경로 담을 공간 
  int port; // port 담을 변수
  
  // rio: client's rio / server_rio: endserver's rio
  rio_t rio, server_rio; // 클라이언트, 엔드 서버 (proxy의 rio 구조체)

  Rio_readinitb(&rio, connfd); // rio 구조체 초기화
  Rio_readlineb(&rio, buf, MAXLINE); // buf에 fd에서 읽을 값이 담김
  sscanf(buf, "%s %s %s", method, uri, version);  // sscanf는 첫 번째 매개 변수가 우리가 입력한 문자열, 두 번째 매개 변수 포맷, 나머지 매개 변수에 포맷에 맞게 데이터를 읽어서 인수들에 저장
  if (strcasecmp(method, "GET")) {
    printf("Proxy does not implement the method");
    return;
  } //여기까지 concurrent proxy 와 동일
  
  char url_store[100];
  strcpy(url_store, uri);

  int cache_index;
  // in cache then return the cache content
  if ((cache_index=cache_find(url_store)) != -1) {
    readerPre(cache_index);
    Rio_writen(connfd, cache.cacheobjs[cache_index].cache_obj, strlen(cache.cacheobjs[cache_index].cache_obj));
    readerAfter(cache_index);
    return;
  }  // 여기서 캐시를 찾아서 바로 클라이언트에 쏴준다
  
  // parse the uri to get hostname, file path, port
  parse_uri(uri, hostname, path, &port);

  // build the http header which will send to the end server
  build_http_header(endserver_http_header, hostname, path, port, &rio);

  // connect to the end server
  end_serverfd = connect_endServer(hostname, port, endserver_http_header);
  if (end_serverfd < 0) {
    printf("connection failed\n");
    return;
  }

  Rio_readinitb(&server_rio, end_serverfd);

  // write the http header to endserver
  Rio_writen(end_serverfd, endserver_http_header, strlen(endserver_http_header));

  // recieve message from end server and send to the client
  char cachebuf[MAX_OBJECT_SIZE];
  int sizebuf = 0;
  size_t n;
  while ((n=Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
    // printf("proxy received %ld bytes, then send\n", n);
    sizebuf += n;
    if (sizebuf < MAX_OBJECT_SIZE)
      strcat(cachebuf, buf);
    Rio_writen(connfd, buf, n);
  }
  Close(end_serverfd);

  // store it
  if (sizebuf < MAX_OBJECT_SIZE) {
    cache_uri(url_store, cachebuf);
  }
}

// 서버에 요청할 헤더
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio) {
  char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];
  
  // request line
  sprintf(request_hdr, requestline_hdr_format, path); // 파싱을 했던 path 삽입

  // get other request header for client rio and change it
  while (Rio_readlineb(client_rio, buf, MAXLINE) > 0) {
    if (strcmp(buf, endof_hdr) == 0)
      break;  // EOF
    
    // 대소문자 구문하지 않고 host_key 찾았으면 host_hdr값 세팅
    if (!strncasecmp(buf, host_key, strlen(host_key))) {
      strcpy(host_hdr, buf);
      continue;
    }

    // Connection, Proxy-Connection, User-Agent를 제외한 요청 라인을 other_hdr에 담음
    if (!strncasecmp(buf, connection_key, strlen(connection_key))
      && !strncasecmp(buf, proxy_connection_key, strlen(proxy_connection_key))
      && !strncasecmp(buf, user_agent_key, strlen(user_agent_key))) {
        strcat(other_hdr, buf);
      }
  }
  // 요청 라인을 다 읽었는데 Host가 없으면 현재 domain을 host_hdr에 담음
  if (strlen(host_hdr) == 0) {
    sprintf(host_hdr, host_hdr_format, hostname);
  }
  sprintf(http_header, "%s%s%s%s%s%s%s",
          request_hdr,
          host_hdr,
          conn_hdr,
          prox_hdr,
          user_agent_hdr,
          other_hdr,
          endof_hdr);
  return;
}

// 서버와 proxy와 연결
inline int connect_endServer(char *hostname, int port, char *http_header) {
  char portStr[100];
  sprintf(portStr, "%d", port);
  return Open_clientfd(hostname, portStr);
}

// parse the uri to get hostname, file path, port
void parse_uri(char *uri, char *hostname, char *path, int *port) {
  *port = 80; // 클라이언트에서 포트값을 안넣었을 경우
  char *pos = strstr(uri, "//"); // "http://"가 있으면 //부터 return 
  pos = pos!=NULL? pos+2:uri;
  char *pos2 = strstr(pos, ":");

  /* port가 있으면*/
  if (pos2 != NULL) {
    *pos2 = '\0'; // ':'를 '\0'으로 변경
    sscanf(pos, "%s", hostname);
    sscanf(pos2+1, "%d%s", port, path);
  } 
  else 
  {
    pos2 = strstr(pos, "/");
    if (pos2 != NULL) {
      *pos2 = '\0';  
      sscanf(pos, "%s", hostname);
      *pos2 = '/';
      sscanf(pos2, "%s", path);
    } 
    else
    {
      scanf(pos, "%s", hostname); //pos에서 문자열 포맷으로 ip를 hostname에 담음
    }
  }
  return;
}

void cache_init() {
  cache.cache_num = 0;
  int i;
  for (i=0; i<CACHE_OBJS_COUNT; i++) {
    cache.cacheobjs[i].LRU = 0;
    cache.cacheobjs[i].isEmpty = 1;

    // Sem_init 첫 번째 인자: 초기화할 세마포어의 포인터
    // 두 번째: 0 - 쓰레드들끼리 세마포어 공유, 그 외 - 프로세스 간 공유
    // 세 번째: 초기 값
    Sem_init(&cache.cacheobjs[i].wmutex, 0, 1);
    Sem_init(&cache.cacheobjs[i].rdcntmutex, 0, 1);
    // semaphore 값을 할당하는데 이때 값이 0이면 접근할 수 없고 0보다 크면 해당자원에 접근가능
    // 초기값을 1 로 놓는 이유

    cache.cacheobjs[i].readCnt = 0;
  }
}

void readerPre(int i) {
  P(&cache.cacheobjs[i].rdcntmutex);
  cache.cacheobjs[i].readCnt++;
  if (cache.cacheobjs[i].readCnt == 1)
    P(&cache.cacheobjs[i].wmutex);
  V(&cache.cacheobjs[i].rdcntmutex);
}

void readerAfter(int i) {
  P(&cache.cacheobjs[i].rdcntmutex);
  cache.cacheobjs[i].readCnt--;
  if (cache.cacheobjs[i].readCnt == 0)
    V(&cache.cacheobjs[i].wmutex);
  V(&cache.cacheobjs[i].rdcntmutex);
}

int cache_find(char *url) {
  int i;
  for (i=0; i<CACHE_OBJS_COUNT; i++) {
    readerPre(i);
    if ((cache.cacheobjs[i].isEmpty == 0) && (strcmp(url, cache.cacheobjs[i].cache_url) == 0))
      break;
    readerAfter(i);
  }
  if (i >= CACHE_OBJS_COUNT)
    return -1;
  return i;
}

int cache_eviction() {
  int min = LRU_MAGIC_NUMBER;
  int minindex = 0;
  int i;
  for (i=0; i<CACHE_OBJS_COUNT; i++) {
    readerPre(i);
    // cache block이 다 차있지 않을때는 비어있는 인덱스를 반환
    if (cache.cacheobjs[i].isEmpty == 1) {
      minindex = i;
      readerAfter(i);
      break;
    }
    // cache block 이 다 차있을때는 가장 낮은 LRU를 가진 cache block을 
    // 덮어쓰기위해 minindex로 갱신하고 이 값을 리턴. 
    // -> 'eviction policy'
    if (cache.cacheobjs[i].LRU < min) {
      minindex = i;
      min = cache.cacheobjs[i].LRU;
      readerAfter(i);
      continue;
    }
    readerAfter(i);
  }
  return minindex;
}

void writePre(int i) {
  P(&cache.cacheobjs[i].wmutex);
}

void writeAfter(int i) {
  V(&cache.cacheobjs[i].wmutex);
}

// 자신을 제외한 cache block 들의 LRU를 내려주는 함수
void cache_LRU(int index) {
  int i;
  for (i=0; i<index; i++) {
    writePre(i);
    if (cache.cacheobjs[i].isEmpty == 0 && i != index)
      cache.cacheobjs[i].LRU--;
    writeAfter(i);
  }
  i++;
  for (i; i<CACHE_OBJS_COUNT; i++) {
    writePre(i);
    if (cache.cacheobjs[i].isEmpty == 0 && i != index) {
      cache.cacheobjs[i].LRU--;
    }
    writeAfter(i);
  }
}

void cache_uri(char *uri, char *buf) {
  int i = cache_eviction();
  
  writePre(i);

  strcpy(cache.cacheobjs[i].cache_obj, buf);
  strcpy(cache.cacheobjs[i].cache_url, uri);
  cache.cacheobjs[i].isEmpty = 0;
  cache.cacheobjs[i].LRU = LRU_MAGIC_NUMBER;
  cache_LRU(i);

  writeAfter(i);
}
