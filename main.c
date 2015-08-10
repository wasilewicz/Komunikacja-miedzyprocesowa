#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <stdlib.h>


#define BUFF_SIZE 1024
typedef struct person
{
    char name[50];
    struct person* next;
    
} user;

static int handle_server_fd(int server_fd, int epoll_fd);
static void handle_client_fd(struct epoll_event *e, int epoll_fd);
void add_user(char* name, int fd);
void write_all_users();

struct person *first; 

  
 
  
int main(int argc, const char *argv[])
{
    first = malloc(sizeof(struct person));
    first->next = NULL;
     
    int i = 0;
    char buff[BUFF_SIZE];
    ssize_t msg_len = 0;

    int srv_fd = -1;
    int cli_fd = -1;
    int epoll_fd = -1;

    struct sockaddr_in srv_addr;
    
    struct epoll_event e, es[2];

    memset(&srv_addr, 0, sizeof(srv_addr));
    memset(&e, 0, sizeof(e));

    srv_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (srv_fd < 0) 
    {
        printf("Cannot create socket\n");
        return 1;
    }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(5050);
    if (bind(srv_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) 
    {
        printf("Cannot bind socket\n");
        close(srv_fd);
        return 1;
    }

    if (listen(srv_fd, 1) < 0) 
    {
        printf("Cannot listen\n");
        close(srv_fd);
        return 1;
    }

    epoll_fd = epoll_create(2);
    if (epoll_fd < 0) 
    {
        printf("Cannot create epoll\n");
        close(srv_fd);
        return 1;
    }

    e.events = EPOLLIN;
    e.data.fd = srv_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, srv_fd, &e) < 0) 
    {
        printf("Cannot add server socket to epoll\n");
        close(epoll_fd);
        close(srv_fd);
        return 1;
    }

    for(;;) {
        i = epoll_wait(epoll_fd, es, 2, -1);
        if (i < 0) 
        {
            printf("Cannot wait for events\n");
            close(epoll_fd);
            close(srv_fd);
            return 1;
        }

        for (--i; i > -1; --i) 
        {
            if (es[i].data.fd == srv_fd) 
            {
                if (handle_server_fd(srv_fd, epoll_fd) < 0)
                    return 1;
            } 
            else 
            {
                handle_client_fd(&es[i], epoll_fd);
            }
        }
    }

	return 0;
}

static int handle_server_fd(int server_fd, int epoll_fd)
{
    int cli_fd = -1;
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr_len);
    struct epoll_event e;

    memset(&cli_addr, 0, sizeof(cli_addr));

    cli_fd = accept(server_fd, (struct sockaddr*) &cli_addr, &cli_addr_len);
    if (cli_fd < 0) {
        printf("Cannot accept client\n");
        close(epoll_fd);
        close(server_fd);
        return 1;
    }

    memset(&e, 0, sizeof(e));
    e.data.fd = cli_fd;
    e.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_fd, &e) < 0) {
        printf("Cannot accept client\n");
        close(epoll_fd);
        close(server_fd);
        return 1;
    }
}

static void handle_client_fd(struct epoll_event *e, int epoll_fd)
{
    if ((e->events & EPOLLERR) || (e->events & EPOLLRDHUP) || (e->events & EPOLLHUP)) 
    {
        //TODO: serve bad event on client socket
        //      * remove socket from the user list
        //      * remove socket from epoll: epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->data.fd, e);
        //      * close this socket
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->data.fd, e);
        close (epoll_fd);
    }
    else if (e->events & EPOLLIN) 
    {
        char msg[BUFF_SIZE];
        size_t msg_len = 0;
        msg_len = read(e->data.fd, &msg_len, sizeof(size_t));
        if(msg_len == -1)
        {
            printf("bytes reading error \n");
        }
        //TODO error handling
        
        
        if(read(e->data.fd, msg, BUFF_SIZE) == -1)
        {
            printf("reading error \n");
            close(e->data.fd);
            close(epoll_fd);
            
        }
        
        write(e->data.fd, &msg_len , sizeof(size_t));
        write(e->data.fd, msg, msg_len);
        
        msg[msg_len] = '\0';
        switch(msg[0])
        {
            case '1':
                printf("one \n");   
                break;
            case '2':
                printf("Your nick is: ");
                int i=0;
                char name[50];
                int a;
                for(a=2;a<msg_len;a++)
                {
                    name[i] = msg[a];
                    i++;
                    printf("%c", msg[a]);
                }
                printf("\n");
                add_user(name, e->data.fd);
                break;
            case '3':
                break;
            case '4':
                break;
            case '5':
                break;
            case '6':
                write_all_users();
                break;
            case '7':
                break;
            default:
                printf("Wrong first number \n");
                break;
        }
        
        close(e->data.fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->data.fd, e);
        
        
    }
}

void add_user(char *_name, int _fd)
{
    if( first->next == NULL )
    {
        strcpy(first->name,_name);
        first->next = (struct person*)malloc(sizeof(struct person));
    }
    else
    {
        struct person *pointer_to_user = first->next;

        if(pointer_to_user->next != NULL)
        {
            while(pointer_to_user-> next != NULL)
            {
                pointer_to_user = pointer_to_user->next;
            }
        }

        strcpy(pointer_to_user->name, _name);
        pointer_to_user->next = (struct person*)malloc(sizeof(struct person));
    }
}

void write_all_users()
{
    struct person* pointer_to_user = first;
    while(pointer_to_user->next != NULL)
    {
        printf("%s ",pointer_to_user->name );
        pointer_to_user = pointer_to_user->next;
    }
}





