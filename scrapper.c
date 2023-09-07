
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

#define MUTEX_NUMBER 400
#define MUTEX_TIMEOUT_MICROSECONDS 40000
#define THEAD_TIMEOUT_CREATION_MICROSECONDS 200
#define THREAD_CHECK_TIMEOUT_MICROSECONDS 2000
#define MAX_THREADS_NUMBER 1000

char *strjoin(char const *s1, char const *s2);

static pthread_mutex_t mutexs[MUTEX_NUMBER];
static float entries;
static int threads_dones = 0;
static int threads_fill = 0;
static char *validSubDomains = "\0";


typedef struct threadArg
{
    int index;
    char *domain;
} threadArg;

size_t arrofArrayLength(char **arr)
{   
    size_t i = 0;
    
    while (arr[i] != NULL) {
        i++;
    }

    return i;
}

char **appendStr(char **strs, char *str)
{
    size_t len = arrofArrayLength(strs);
    size_t i = 0;

    char **copy = malloc(sizeof(char*) * (len + 2));

    while (strs[i]) {
        copy[i] = strs[i];
        i++;
    }
    
    copy[i] = str;
    copy[i + 1] = NULL;

    free(strs);

    i = 0;
    while (copy[i]) {
        i++;
    }

    return copy;
}

int hasStr(char **strs, char *str)
{
    if (str == NULL)
        return 0;

    for (size_t i = 0; strs[i]; i++)
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

int testDomain(char *domain)
{
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    int result = getaddrinfo(domain, NULL, &hints, &res);
    if (result == 0) {
        freeaddrinfo(res);
        return 1;
    }

    return 0;
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

    if (testDomain(datas->domain))
    {
        tmp = strjoin(validSubDomains, strjoin(" ", datas->domain));
        validSubDomains = tmp;
        printf(GREEN "subdomain %s exists!\n" RESET, datas->domain);
    }
    pthread_mutex_unlock(&mutexs[i]);
    // printf("[%i] thread is %sended%s, liberate mutex %s[%li]%s\n", datas->index, GREEN, RESET, MAGENTA, i, RESET);

    threads_dones += 1;

    free(datas);
    
    return NULL;
}

void sigIntlCatch() {
    printf(GREEN "List of valid subdomains finded : %s\n" RESET, validSubDomains);
    exit(0);
}

void *monitoring(void *args) {
    float process_percentage;
    while (threads_dones < (int)entries - 1) {
        process_percentage = threads_dones * 100 / entries;
        printf("Process is done at %s%.2f%%%s. Still running %i threads\n", GREEN, process_percentage, RESET, (int)entries - threads_dones);
        sleep(1);
    }

    return args;
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

    pthread_t threads[MAX_THREADS_NUMBER];
    pthread_t thead_monitoring;

    for (size_t i = 0; i < MUTEX_NUMBER; i++)
        pthread_mutex_init(&mutexs[i], NULL);


    if (strchr(av[1], '.') == NULL)
    {
        printf(RED "Your domain %s, is invalid !\n" RESET, av[1]);
        return 1;
    }

    entries = atof(av[2]);

    pthread_create(&thead_monitoring, NULL, &monitoring, NULL);

    printf(MAGENTA "Start sub domain parsing for domain : %s\n" RESET, av[1]);

    // Catch SIGINT for not lost results
    signal(SIGINT, sigIntlCatch);

    for (int i = 0; i < atoi(av[2]); i++)
    {
        int maxLength = rand() % 35;
        if (!maxLength)
            continue;

        char *subDomain = strjoin(strjoin(randstring(maxLength), "."), av[1]);

        printf("[%i] test subdomain %s\n", i, subDomain);


        // If subDomain has already been tested, pass it!
        if (hasStr(triesDomains, subDomain))
        {
            i -= 1;
            printf(RED "Domain %s already tested!\n" RESET, subDomain);
            free(subDomain);
            continue;
        }

        triesDomains = appendStr(triesDomains, subDomain);

        threadArg *args = malloc(sizeof(threadArg));
        args->index = i;
        args->domain = subDomain;

        pthread_create(&threads[threads_fill], NULL, &threadProcess, args);
        threads_fill += 1;

        usleep(THEAD_TIMEOUT_CREATION_MICROSECONDS);

        if (threads_fill >= MAX_THREADS_NUMBER) {
            for (int j = 0; j < threads_fill ; j++) {
                pthread_join(threads[j], NULL);
            }
            threads_fill = 0;
        }
    }

    printf(GREEN "List of valid subdomains finded : %s\n" RESET, validSubDomains);
}
