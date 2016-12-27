#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <inttypes.h>
#include <unistd.h>
#include <netinet/in.h>
#include "io.h"

int main(int argc, char* argv[]) {
    int s, len;
    struct sockaddr_un remote;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if(argc != 2) {
        fprintf(stderr, "Usage: guess-client UNIX_SOCKET_PATH\n");
        return 1;
    }

    printf("Trying to connect...\n");

    remote.sun_family = AF_UNIX;

    strcpy(remote.sun_path, argv[1]);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    printf("Connected.\n");

    uint32_t left_bound = 0, right_bound = 1000000000, current_attempt, number_htonl;
    int attempt_no = 1;
    char answer;

    for(;;) {
        if (answer == '=') {
            fprintf(stdout, "Good job!! The number is %u\n", current_attempt);
            return 0;
        }
        current_attempt = left_bound + (right_bound - left_bound) / 2;
        number_htonl = htonl(current_attempt);
        if (!SendAll(s, (char*)&number_htonl, sizeof(number_htonl))) {
            break;
        }

        if (!RecvAll(s, &answer, sizeof(answer))) {
            break;
        }

        if (answer == '>') {
            left_bound = current_attempt + 1;
        }

        if (answer == '<') {
            right_bound = current_attempt - 1;
        }

        fprintf(stderr, "attempt %d, sent %u, recieved %c\n", attempt_no, current_attempt, answer);
        attempt_no++;
    }

    return 1;
}
