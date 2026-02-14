struct ticket_req {
    int pid;
};

struct ticket_res {
    int ticket;
    unsigned int served_sec;
    unsigned int served_usec;
};

program BAKERY_PROG {
    version BAKERY_VER {
        ticket_res GET_TICKET(ticket_req) = 1;
    } = 1;
} = 0x20000011;
