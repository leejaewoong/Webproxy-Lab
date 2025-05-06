#include <stdio.h>
#include "csapp.h"

void echo(int connfd);
int open_listenfd(char *port);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    
    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(argv[1]);
    printf("listenfd: %d\n", listenfd);

    while(1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);

        printf("Connected to (%s:%s)\n", client_hostname, client_port);

        echo(connfd);
        close(connfd);
    }

    return 0;
}

int open_listenfd(char *port) 
{
    // 서버가 클라이언트 요청을 받을 수 있는 리스닝 소켓을 만드는 함수
    int listenfd, optval = 1;    
    struct addrinfo hints, *result, *p; 

    // hints 초기화 및 설정: TCP, 수신용, 숫자 포트, 사용 가능한 주소 체계만
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // 포트 번호로 바인딩 가능한 주소 리스트 생성
    getaddrinfo(NULL, port, &hints, &result);

    // 주소 리스트를 순회하며 바인딩 가능한 주소를 찾음
    for(p = result; p; p = p->ai_next)
    {
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(listenfd < 0) continue;   
        
        // 소켓 재사용 가능하도록 설정 (TIME_WAIT 문제 방지)
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(listenfd);        
    }
    
    // 주소 리스트 해제
    freeaddrinfo(result);
        
    if(!p) return -1;

    // 연결 요청 대기 큐 설정 (기본 크기 LISTENQ 사용)    
    if((listen(listenfd, LISTENQ)) < 0)
    {
        close(listenfd);
        return -1;
    }

    // 리스팅 소켓 디스크립터 반환
    return listenfd;        
}

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        Rio_writen(connfd, buf, n);
    }
}

