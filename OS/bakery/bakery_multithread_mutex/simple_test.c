#include "bakery.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    CLIENT *clnt;
    struct Ticket ticket_result;
    enum clnt_stat retval_1;
    
    clnt = clnt_create("localhost", BAKERY_PROG, BAKERY_VER, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror("localhost");
        exit(1);
    }
    
    retval_1 = get_ticket_1(NULL, &ticket_result, clnt);
    if (retval_1 != RPC_SUCCESS) {
        clnt_perror(clnt, "get_ticket failed");
    } else {
        printf("Got ticket #%d\n", ticket_result.number);
    }
    
    clnt_destroy(clnt);
    return 0;
}
