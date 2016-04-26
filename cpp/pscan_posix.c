#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>

#define TIMEOUT 1

int conn_nonb(char *ip,int port,int nsec);
void scan_r(char *ip,int s_port,int e_port,int *result);
void thread_run(void *arg);
void mulite_thread_run(char *ip,int s_port,int e_port,int thread_count);
pthread_mutex_t mut;

int **res;

struct argument
{
    char *ip;
    int s_port;
    int e_port;
    int i;
};


int main(int argc, const char * argv[])
{

    // insert code here...
    if (argc < 4) {
        printf("arg input error，plz use: target_ip start_port end_port [theads num]\n");
        return 0;
    }
    printf("----start----\n");

    //int fd[65535];
    char *ip = argv[1];
    int s_port = atoi(argv[2]);
    int e_port = atoi(argv[3]);
    int open[65535];
    int count = 0;

    if(argc == 5)
    {
        //multiple threads
        int ts = atoi(argv[4]);
        res = (int **)malloc(sizeof(int*) * ts);
        mulite_thread_run(ip, s_port, e_port,ts);
    }else{
        //single thread
        for (int i = s_port; i <= e_port; i++) {
            //printf("scan %d port\n",i);
            int r = -1;
            if ((r = conn_nonb(ip, i, TIMEOUT)) == 0) {
                open[count] = i;
                count++;
            }
        }
    }


    return 0;
}

void mulite_thread_run(char *ip,int s_port,int e_port,int thread_count)
{
    int all = e_port - s_port + 1;
    int ts = thread_count;
    int c = all / ts + 1;

    pthread_mutex_init(&mut, NULL);
    pthread_t *thread;
    thread = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    for (int i = 0; i < ts; i++) {
        struct argument *arg_thread;
        arg_thread = (struct argument *)malloc(sizeof(struct argument));
        arg_thread->ip = ip;
        arg_thread->s_port = s_port+i * c;
        arg_thread->e_port = s_port+(i+1)*c;
        arg_thread->i=i;
        pthread_create(&thread[i], NULL, thread_run,(void *)arg_thread);
    }


    for (int j = 0; j < ts; j++) {
        void *thread_return;
        int ret=pthread_join(thread[j],&thread_return);/*wait for thread task finished and receive it's return value*/
        if(ret != 0)
			;
            //printf("call pthread_join error!\n");
        else
			;
            //printf("call pthread_join success! thread quit with value %d\n",(int)thread_return);
    }

    printf("----result----\n");
    for (int k = 0 ; k < ts; k++) {
        int count = res[k][0];
        for(int i = 1 ; i <= count;i++)
        {
            printf("%d open\n",res[k][i]);
        }
    }

    printf("scan end\n------------------\n");
}

void thread_run(void *arg)
{
    struct argument *arg_thread;/*argument value including target info*/

    arg_thread=(struct argument *)arg;
    int size = arg_thread->e_port - arg_thread->s_port + 2;
    int *result = (int *)malloc(sizeof(int)*size);
    scan_r(arg_thread->ip, arg_thread->s_port, arg_thread->e_port, result);
    res[arg_thread->i] = result;
    pthread_exit(NULL);
}

void scan_r(char *ip,int s_port,int e_port,int *result)
{
    int count = 0;
    for (int i = s_port; i <= e_port; i++) {
        //printf("scan %d port\n",i);
		;

        int r = -1;
        if ((r=conn_nonb(ip, i, TIMEOUT)) == 0) {
            count++;
            result[count]= i;
        }

    }
    result[0] = count;

}

int conn_nonb(char *ip,int port,int nsec)
{
    int flags, n, error;
    //socklen_t len;
    fd_set rset,wset;
    struct timeval tval;

    FD_ZERO(&wset);
    FD_ZERO(&rset);
    tval.tv_sec = nsec;
    tval.tv_usec = 0;
    //struct servent *sent;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    error = 0;
    if((n=connect(sockfd,(struct sockaddr *)&address,sizeof(address)))<0){
        if(errno!=EINPROGRESS)
        {
            printf("Connecting error!\n");
            return -1;
        }
        else if(n==0)
        { //This case may be happen on localhost?
            printf("Connecting success! \n");
            return 0;
        }
    }
    FD_SET(sockfd,&rset);
    wset=rset;
    //usleep(10);


    int rst = select(sockfd + 1, &rset,&wset,NULL,&tval);

    switch (rst) {
        case -1:
            perror("Select error"); exit(-1);
        case 0:
            close(sockfd);
            //printf("Timed Out!\n");
            break;
        default:
            if (FD_ISSET(sockfd,&rset)||FD_ISSET(sockfd,&wset)) {
                int error;
                socklen_t len = sizeof (error);
                if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len) < 0)
                {
                    printf ("getsockopt fail,connected fail\n");
                    return -1;
                }
                if(error==0)
                {
                    //printf ("%d open\n",port);
                    return 0;
                }
            }
            close(sockfd);
    }
    return -1;
}

