const GET_TICKET = 1;
const GET_SERVICE = 2;

struct Ticket {
    int number;
};

struct ServiceRequest {
    int ticket_number;
};

struct ServiceResponse {
    long result;
};

program BAKERY_PROG {
    version BAKERY_VER {
        struct Ticket GET_TICKET(void) = 1;
        struct ServiceResponse GET_SERVICE(struct ServiceRequest) = 2;
    } = 1;
} = 0x20000002;