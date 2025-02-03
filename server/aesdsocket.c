/*
** aesdsocket.c -- a stream socket server test
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>


#define PORT "9000"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 60000


bool err_flag = false;
bool daemon_flag = false;
bool caught_sigint = false;
bool caught_sigterm = false;


static void sigchild_handler(int signal_number)
{
    if(signal_number == SIGINT)
    {
        caught_sigint = true;
    }
    else if(signal_number == SIGTERM)
    {
        caught_sigterm = true;
    }
}

void sigpar_handler(int signal_number)
{
    if(signal_number == SIGINT)
    {
        caught_sigint = true;
    }
    else if(signal_number == SIGTERM)
    {
        caught_sigterm = true;
    }
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void application_core(int sock_desc)
{
    int new_fd;
    int numbytes = 0;
    socklen_t sin_size;
    char buf[MAXDATASIZE];
    char s[INET6_ADDRSTRLEN];
    char send_buf[MAXDATASIZE];
    unsigned long size_file = 0;
    struct sockaddr_storage their_addr;

    FILE *fp = fopen("/var/tmp/aesdsocketdata.txt", "a+");

    if(listen(sock_desc, BACKLOG) == -1)
    {
        syslog(LOG_ERR, "Listen failed\n");
        err_flag = true;
        exit(EXIT_FAILURE);
    }

    syslog(LOG_DEBUG, "server: waiting for connections...\n");

    while(1)
    {

        if(caught_sigint)
        {
            syslog(LOG_DEBUG, "Caught SIGINT signal, exiting\n");
            break;
        }
        if(caught_sigterm)
        {
            syslog(LOG_DEBUG, "Caught SIGTERM signal, exiting\n");
            break;
        }

        sin_size = sizeof(their_addr);
        new_fd = accept(sock_desc, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd == -1)
        {
            syslog(LOG_ERR, "Accept failed\n");
            err_flag = true;
            break;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));

        syslog(LOG_DEBUG, "Accepted connection from %s\n", s);

        if((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1)
        {
            syslog(LOG_ERR, "Receiving failed\n");
            err_flag = true;
            exit(EXIT_FAILURE);
        }

        fprintf(fp, "%s", buf);

        if(strstr(buf, "\n") != NULL)
        {
            fseek(fp, 0, SEEK_END);
            size_file = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            fread(send_buf, sizeof(char), size_file, fp);

            if(send(new_fd, send_buf, size_file, 0) == -1)
            {
                syslog(LOG_ERR, "Sending failed\n");
                err_flag = true;
                exit(EXIT_FAILURE);
            }

            close(new_fd);

            syslog(LOG_DEBUG, "Closed connection from %s\n", s);
        }
    }

    if(caught_sigint || caught_sigterm)
    {
        caught_sigint = false;
        caught_sigterm = false;

        fclose(fp);

        remove("/var/tmp/aesdsocketdata.txt");

        close(sock_desc);

        closelog();

        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char **argv)
{
    int rv;
    int sockfd;
    int reuse_yes = 1;
    struct addrinfo hints, *servinfo;
    struct sigaction par_action, child_action;


    if(argc > 1)
    {
        if(strcmp(argv[1], "-d") == 0)
        {
            daemon_flag = true;
        }
        else
        {
            daemon_flag = false;
        }
    }

    openlog("socketuser", LOG_PID, LOG_USER);

    memset(&par_action, 0, sizeof(struct sigaction));
    par_action.sa_handler = sigpar_handler;
    sigemptyset(&par_action.sa_mask);
    if(sigaction(SIGTERM, &par_action, NULL) != 0)
    {
        syslog(LOG_ERR, "Error %d (%s) registering for SIGTERM", errno, strerror(errno));
        return -1;
    }
    if(sigaction(SIGINT, &par_action, NULL) != 0)
    {
        syslog(LOG_ERR, "Error %d (%s) registering for SIGINT", errno, strerror(errno));
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        syslog(LOG_ERR, "Error getting the IP address\n");
        return -1;
    }

    if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    {
        syslog(LOG_ERR, "Socket creation failed\n");
        return -1;
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_yes, sizeof(int)) == -1)
    {
        syslog(LOG_ERR, "Setsockopt failed\n");
        return -1;
    }

    if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        close(sockfd);
        syslog(LOG_ERR, "Bind failed\n");
        return -1;
    }

    freeaddrinfo(servinfo); // all done with this structure

    syslog(LOG_DEBUG, "Bind successfull!\n");

    pid_t pid;

    if(daemon_flag == true)
    {

        pid = fork();

        if(pid < 0)
        {
            syslog(LOG_ERR, "Fork failed!\n");
            return -1;
        }
        else if(pid == 0)
        {
            setsid();

            umask(0);

            chdir("/");

            syslog(LOG_DEBUG, "Child working with PID: %d\n", getpid());

            memset(&child_action, 0, sizeof(struct sigaction));
            child_action.sa_handler = sigchild_handler;
            if(sigaction(SIGTERM, &child_action, NULL) != 0)
            {
                syslog(LOG_ERR, "Error %d (%s) registering for SIGTERM", errno, strerror(errno));
                err_flag = true;
                exit(EXIT_FAILURE);
            }
            if(sigaction(SIGINT, &child_action, NULL) != 0)
            {
                syslog(LOG_ERR, "Error %d (%s) registering for SIGINT", errno, strerror(errno));
                err_flag = true;
                exit(EXIT_FAILURE);
            }

            application_core(sockfd);
        }
        else
        {
            if(err_flag == true)
            {
                return -1;
                exit(EXIT_FAILURE);
            }
            else
            {
                return 0;
                exit(EXIT_SUCCESS);
            }
        }
    }
    else
    {
        application_core(sockfd);
    }
}
