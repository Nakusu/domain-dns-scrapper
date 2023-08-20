
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

#define BUFFER_SIZE 1024
#define MUTEX_NUMBER 800
#define MUTEX_TIMEOUT_MICROSECONDS 50000
#define THEAD_TIMEOUT_CREATION_MICROSECONDS 200

char *strjoin(char const *s1, char const *s2);

static pthread_mutex_t mutexs[MUTEX_NUMBER];
static int threads_dones = 0;
static char *validSubDomains = "\0";


typedef struct threadArg
{
    int index;
    char *domain;
    char **validSubDomains;
} threadArg;

size_t arrofArrayLength(char **arr)
{
    if (!arr || *arr == NULL)
        return 0;

    for (size_t i = 0; 1 == 1; i++)
        if (arr[i] == NULL)
            return i - 1;
    return 0;
}

char **appendStr(char **strs, char *str)
{
    size_t len = arrofArrayLength(strs);

    char **copy = malloc(sizeof(char *) * (len + 2));

    copy[len] = str;
    copy[len + 1] = NULL;

    free(strs);

    return copy;
}

int hasStr(char **strs, char *str)
{
    size_t len = arrofArrayLength(strs);

    for (size_t i = 0; i < len; i++)
    {
        if (strcmp(strs[i], str) == 0)
            return 1;
    }

    return 0;
}

char *randstring(size_t length)
{

    static char charset[] = "abcdefghijklmnopqrstuvwxyz.-";
    char *randomString = NULL;

    if (length)
    {
        randomString = malloc(sizeof(char) * (length + 1));

        if (randomString)
        {
            for (size_t n = 0; n < length; n++)
            {
                int key = rand() % (int)(sizeof(charset) - 1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}

int socket_connect(char *host, in_port_t port)
{
    struct hostent *hp;
    struct sockaddr_in addr;
    int on = 1, sock;

    if (!(hp = gethostbyname(host)))
    {
        return -1;
    }
    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

    if (sock == -1)
        return sock;

    if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
        return -1;
    return sock;
}

int testDomain(char *domain, int port)
{
    int fd;
    char buffer[BUFFER_SIZE];
    int exists;

    fd = socket_connect(domain, port);
    exists = 1;
    if (fd <= 0)
    {
        return 0;
    }

    write(fd, "GET /\r\n", strlen("GET /\r\n"));
    bzero(buffer, BUFFER_SIZE);

    read(fd, buffer, BUFFER_SIZE - 1);

    if (strlen(buffer) == 0)
    {
        exists = 0;
    }

    shutdown(fd, SHUT_RDWR);
    close(fd);

    return exists;
}

void *threadProcess(void *args)
{
    threadArg *datas = (threadArg*)args;
    char *tmp;
    size_t i = 0;

    while (1 == 1) {
        if (i >= MUTEX_NUMBER - 1) {
            i = 0;
        } else {
            i++;
        }
        if (pthread_mutex_trylock(&mutexs[i]) == 0)
            break;

        usleep(MUTEX_TIMEOUT_MICROSECONDS);
    }

    if (testDomain(datas->domain, 443))
    {
        tmp = strjoin(*datas->validSubDomains, datas->domain);
        datas->validSubDomains = &tmp;
        printf(GREEN "subdomain %s exists!\n" RESET, datas->domain);
    }
    if (testDomain(datas->domain, 80))
    {
        tmp = strjoin(*datas->validSubDomains, datas->domain);
        datas->validSubDomains = &tmp;
        printf(GREEN "subdomain %s exists!\n" RESET, datas->domain);
    }
    pthread_mutex_unlock(&mutexs[i]);
    // printf("[%i] thread is %sended%s, liberate mutex %s[%li]%s\n", datas->index, GREEN, RESET, MAGENTA, i, RESET);

    threads_dones += 1;
    
    return NULL;
}

void sigIntlCatch() {
    printf(GREEN "List of valid subdomains finded : %s\n" RESET, validSubDomains);

    exit(0);
}

int main(int ac, char **av)
{
    char **triesDomains = malloc(sizeof(char *) * 2);
    triesDomains[0] = "Test\0";
    triesDomains[1] = NULL;

    if (ac != 3)
    {
        printf(RED "You must specify an domain and a number of tries\n" RESET);
        return 1;
    }

    pthread_t threads[atoi(av[2])];
    for (size_t i = 0; i < MUTEX_NUMBER; i++)
        pthread_mutex_init(&mutexs[i], NULL);


    if (strchr(av[1], '.') == NULL)
    {
        printf(RED "Your domain %s, is invalid !\n" RESET, av[1]);
        return 1;
    }

    printf(MAGENTA "Start sub domain parsing for domain : %s\n" RESET, av[1]);

    for (int i = 0; i < atoi(av[2]); i++)
    {
        int maxLength = rand() % 35;
        if (!maxLength)
            continue;

        char *subDomain = strjoin(strjoin(randstring(maxLength), "."), av[1]);

        // If subDomain has already been tested, pass it!
        if (hasStr(triesDomains, subDomain))
        {
            i -= 1;
            printf(RED "Domain already tested!\n" RESET);
            continue;
        }

        triesDomains = appendStr(triesDomains, subDomain);

        printf("[%i] test subdomain %s\n", i, subDomain);

        threadArg *args = malloc(sizeof(threadArg));
        args->index = i;
        args->domain = subDomain;
        args->validSubDomains = &validSubDomains;

        pthread_create(&threads[i], NULL, &threadProcess, args);
        usleep(THEAD_TIMEOUT_CREATION_MICROSECONDS);
    }

    // Catch SIGINT for not lost results
    signal(SIGINT, sigIntlCatch);

    float process_percentage;
    while (threads_dones < (atoi(av[2]) - 1)) {
        process_percentage = threads_dones * 100 / atof(av[2]);
        printf("Process is done at %s%.2f%%%s. Still running %i threads\n", GREEN, process_percentage, RESET, (atoi(av[2]) - threads_dones));
        sleep(1);
    }

    printf(GREEN "List of valid subdomains finded : %s\n" RESET, validSubDomains);
}