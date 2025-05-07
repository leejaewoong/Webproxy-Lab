#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

void parse_uri(char *uri, char *hostname, char *port, char *path);
void build_request(char *hostname, char *port, char *path, char *request, char *method, char *uri, char *rest_header);


int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    int proxyfd, n;
    char buf[MAXLINE], uri[MAXLINE], path[MAXLINE], method[MAXLINE], version[MAXLINE], request[MAXLINE], rest_header[MAXLINE], hdrline[MAXLINE];
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
    rio_t rio_client, rio_server;
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);   
    
    // 버퍼 라이더 초기화
    Rio_readinitb(&rio_client, connfd);    

    // 요청라인 파싱
    Rio_readlineb(&rio_client, buf, MAXLINE);
    printf("original buf:\n");
    printf("%s\n", buf);

    // 요청라인에서 method, uri, version 추출
    sscanf(buf, "%s %s %s", method, uri, version);   
    
    // uri에서 host, port, path 분리
    parse_uri(uri, hostname, port, path);
    printf("uri: %s, hostname: %s, prot: %s, path: %s\n", uri, hostname, port, path);

    // rest_header 추출    
    rest_header[0] = '\0';
    while (Rio_readlineb(&rio_client, hdrline, MAXLINE) > 0) 
    {
      if (!strcmp(hdrline, "\r\n")) 
        break;  
      strcat(rest_header, hdrline);
    }
    strcat(rest_header, "Connection: close\r\nProxy-Connection: close\r\n");

    // 요청라인 재조립
    build_request(hostname, port, path, request, method, uri, rest_header);      
    printf("request:\n");
    printf("%s\n", request);

    // 원 서버 연결
    proxyfd = Open_clientfd(hostname, port);
    Rio_writen(proxyfd, request, strlen(request));    
    
    // 서버 응답 스트리밍하여 클라이언트에 전달
    Rio_readinitb(&rio_server, proxyfd);  
    while((n = Rio_readnb(&rio_server, buf, MAXLINE)) > 0)    
    {
      printf("response:\n");
      printf("%s\n", buf);
      Rio_writen(connfd, buf, n);    
    }

    // 연결 종료
    Close(proxyfd);
    Close(connfd); 
  }
}


void parse_uri(char *uri, char *hostname, char *port, char *path) 
{
  char *hostbegin, *pathbegin, *portbegin;  
  int portno = 80;

  if (strncasecmp(uri, "http://", 7) == 0)
    hostbegin = uri + 7;
  else
    hostbegin = uri;
  
  pathbegin = strchr(hostbegin, '/');
  if (pathbegin) 
  {
    strcpy(path, pathbegin);
    *pathbegin = '\0';
  } 
  else   
    strcpy(path, "/");
  
  portbegin = strchr(hostbegin, ':');  
  if (portbegin) 
  {
    *portbegin = '\0';
    portno = atoi(portbegin + 1);
  }

  strcpy(hostname, hostbegin);
  sprintf(port, "%d", portno);
}


void build_request(char *hostname, char *port, char *path, char *request, char *method, char *uri, char *rest_header)
{
  char host_name[MAXLINE];  

  /* 요청 라인 + Host 헤더(필요시 추가) */
  sprintf(request, "%s %s HTTP/1.0\r\n", method, path);

  /* 추가 헤더 삽입 (예: Host, Connection 등) */
  sprintf(host_name, "Host: %s:%s\r\n", hostname, port);
  strcat(request, host_name);    
  strcat(request, rest_header);    

  /* 메시지 종료 빈 라인 추가 */
  strcat(request, "\r\n");
}