#include <stdio.h>
#include "csapp.h"

int open_clientfd(char *hostname, char*port); 

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if(argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }

    host = argv[1];
    port = argv[2];

    clientfd = open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while(fgets(buf, MAXLINE, stdin) != NULL)
    {
        size_t len = strlen(buf);
        Rio_writen(clientfd, buf, len);
        Rio_readlineb(&rio, buf, MAXLINE);
        fputs(buf, stdout);
    }

    Close(clientfd);
    return 0;
}

// 클라이언트 소켓을 생성하고, 서버에 연결을 시도하는 함수
int open_clientfd(char *hostname, char*port) 
{    
    int clientfd;
    struct addrinfo hints, *result, *p;    
    
    // hints 초기화 및 설정: TCP 소켓, 숫자 포트, 현재 사용 가능한 주소 체계만 사용
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
    hints.ai_family = AF_UNSPEC;    
    hints.ai_socktype = SOCK_STREAM;
    
    // 도메인 이름 + 포트를 실제 주소 리스트로 변환
    int rc = getaddrinfo(hostname, port, &hints, &result);
    if (rc != 0) 
    {
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
      return -1;
    }   

    // 주소 리스트를 순회하면서 연결을 시도
    for(p = result; p; p = p->ai_next)
    {
        clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(clientfd == -1) 
            continue;

        if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break;

        close(clientfd);
    }

    // 주소 리스트 해제
    freeaddrinfo(result);

    if(!p) 
        return -1;
    
    else return clientfd;
}
