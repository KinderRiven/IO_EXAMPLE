#include "nanomsg/nn.h"
#include "nanomsg/survey.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

char url[] = "tcp://0.0.0.0:14396";

void* server(void* argv)
{
    char buf[128] = "ABCD";
    char* msg;
    int fd = nn_socket(AF_SP, NN_SURVEYOR);
    printf("nn_socket (%d)!\n", fd);
    int timeo = 5000;
    nn_setsockopt(fd, 0, NN_SNDTIMEO, &timeo, sizeof(timeo) >= 0);
    nn_setsockopt(fd, 0, NN_RCVTIMEO, &timeo, sizeof(timeo) >= 0);
    int res = nn_bind(fd, url);
    int i = 0;
    printf("%d\n", res);
    while (true) {
        int rc = nn_send(fd, buf, sizeof(buf), 0);
        printf("server_send_%d:%d\n", i++, rc);
        rc = nn_recv(fd, &msg, -1, 0);
        printf("server_recv_%d:%d\n", i, rc);
        if (rc != -1) {
            printf("server_recv_%d:%s\n", i++, msg);
            nn_freemsg(msg);
        }
    }
    return nullptr;
}

void* client(void* argv)
{
    char buf[128] = "EFGH";
    char* msg;
    int fd = nn_socket(AF_SP, NN_RESPONDENT);
    int res = nn_connect(fd, url);
    printf("%d %d\n", fd, res);
    for (int i = 0; i < 50; i++) {
        int rc = nn_recv(fd, &msg, -1, 0);
        printf("client_recv_%d:%d\n", i, rc);
        if (rc != -1) {
            printf("server_recv_%d:%s\n", i, msg);
            nn_freemsg(msg);
        }
        rc = nn_send(fd, buf, sizeof(buf), 0);
        printf("client_send_%d:%d\n", i, rc);
    }
    return nullptr;
}

int main(int argc, char** argv)
{
    pthread_t server_pid, client_pid;
    pthread_create(&server_pid, nullptr, server, nullptr);
    pthread_create(&client_pid, nullptr, client, nullptr);

    pthread_join(server_pid, nullptr);
    pthread_join(client_pid, nullptr);
    return 0;
}