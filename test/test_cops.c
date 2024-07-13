#include "test_cops.h"

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-label"
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif

#define info() printf("TEST: %s\n", __func__)

uint16_t
unpack_u16(char* src) {
        uint8_t* ptr = (uint8_t*)src;

        /*
         * Shift the first byte to a higher order byte position,
         * then a bitwise OR will combine it with the next byte.
         */
        return *(ptr) << 8 | *(ptr + 1);
}

uint16_t
unpack_u8(uint8_t* src) {
        uint8_t* ptr = (uint8_t*)src;
        /*
         * Shift the first byte to a higher order byte position,
         * then a bitwise OR will combine it with the next byte.
         */
        return *(ptr) << 8 | *(ptr + 1);
}

uint32_t
unpack_u32(uint32_t* src) {
        uint8_t* ptr = (uint8_t*)src;

        return *(ptr) << 24 |     /* Left shift first byte by 24 bits */
               *(ptr + 1) << 16 | /* Left shift second byte by 16 bits */
               *(ptr + 2) << 8 |  /* Left shift third byte by 8 bits */
               *(ptr + 3);        /* Use fourth byte as is */
}

void
tp_cops_report_state_opcode_to_acronym(void) {
        info();
        TP_ASSERT(MATCHES(cops_otoa(1), "REQ"));
        TP_ASSERT(MATCHES(cops_otoa(2), "DEC"));
        TP_ASSERT(MATCHES(cops_otoa(3), "RPT"));
        TP_ASSERT(MATCHES(cops_otoa(4), "DRQ"));
        TP_ASSERT(MATCHES(cops_otoa(5), "SSQ"));
        TP_ASSERT(MATCHES(cops_otoa(6), "OPN"));
        TP_ASSERT(MATCHES(cops_otoa(7), "CAT"));
        TP_ASSERT(MATCHES(cops_otoa(8), "CC"));
        TP_ASSERT(MATCHES(cops_otoa(9), "KA"));
        TP_ASSERT(MATCHES(cops_otoa(10), "SSC"));
}

void
tp_cops_report_state_opcode_ok(void) {
        info();
        TP_ASSERT(cops_opcode_ok(1));
        TP_ASSERT(cops_opcode_ok(2));
        TP_ASSERT(cops_opcode_ok(3));
        TP_ASSERT(cops_opcode_ok(4));
        TP_ASSERT(cops_opcode_ok(6));
        TP_ASSERT(cops_opcode_ok(7));
        TP_ASSERT(cops_opcode_ok(8));
        TP_ASSERT(cops_opcode_ok(9));
        TP_ASSERT(!cops_opcode_ok(5));
        TP_ASSERT(!cops_opcode_ok(10));
}

void
tp_cops_report_state_opcode_to_string(void) {
        info();
        TP_ASSERT(MATCHES(cops_otos(1), "Handle"));
        TP_ASSERT(MATCHES(cops_otos(2), "Context"));
        TP_ASSERT(MATCHES(cops_otos(3), "In Interface"));
        TP_ASSERT(MATCHES(cops_otos(4), "Out Interface"));
        TP_ASSERT(MATCHES(cops_otos(5), "Reason Code"));
        TP_ASSERT(MATCHES(cops_otos(6), "Decision"));
        TP_ASSERT(MATCHES(cops_otos(7), "LPDP Decision"));
        TP_ASSERT(MATCHES(cops_otos(8), "Error"));
        TP_ASSERT(MATCHES(cops_otos(9), "Client Specific Info"));
        TP_ASSERT(MATCHES(cops_otos(10), "Keep-Alive Timer"));
        TP_ASSERT(MATCHES(cops_otos(11), "PEP Identification"));
        TP_ASSERT(MATCHES(cops_otos(12), "Report Type"));
        TP_ASSERT(MATCHES(cops_otos(13), "PDP Redirect Address"));
        TP_ASSERT(MATCHES(cops_otos(14), "Last PDP Address"));
        TP_ASSERT(MATCHES(cops_otos(15), "Accounting Timer"));
        TP_ASSERT(MATCHES(cops_otos(16), "Message Integrity"));
}

void
tp_cops_common_handle_object(void) {
        info();

        /* Prepare input params. */
        uint8_t handle_object[8];
        const char handle[4] = {'a', 'b', 'c', 'd'};

        /* Pack object. */
        cops_handle(handle_object, handle);

        /* Unpack object. */
        uint16_t len = unpack_u16((char*)handle_object);
        TP_ASSERT(len == 8);
        TP_ASSERT(handle_object[2] == 1);
        TP_ASSERT(handle_object[3] == 1)
        for (int i = 4; i < 8; i++) {
                TP_ASSERT(handle_object[i] == handle[i - 4]);
        }
}

void
tp_cops_common_context_object(void) {
        info();

        /* Prepare input params. */
        uint8_t context[8];

        /* Pack object. */
        cops_context(context);

        /* Unpack object. */
        uint16_t len = unpack_u16((char*)context);
        uint16_t rt = unpack_u16((char*)context + 4);
        uint16_t mt = unpack_u16((char*)context + 6);

        /* Assert. */
        TP_ASSERT(len == 8);
        TP_ASSERT(context[2] == 2);
        TP_ASSERT(context[3] == 1)
        TP_ASSERT(rt == 8);
        TP_ASSERT(mt == 0);
}

void
tp_cops_common_decision_object(void) {
        info();

        /* Prepare input params. */
        uint8_t d[8];

        /* Pack object. */
        cops_decision(d);

        /* Unpack object. */
        uint16_t len = unpack_u16((char*)d);
        uint16_t cmd_code = unpack_u16((char*)d + 4);
        uint16_t flags = unpack_u16((char*)d + 6);

        /* Assert. */
        TP_ASSERT(len == 8);
        TP_ASSERT(d[2] == 6);
        TP_ASSERT(d[3] == 1);
        TP_ASSERT(cmd_code == 1);
        TP_ASSERT(flags == 1);
}

void
tp_cops_client_accept_message(void) {
        info();

        /* Keep-alive and Accounting timers. */
        uint32_t estab_ka_timer = 30;
        uint32_t estab_ac_timer = 15;

        uint8_t msg[24];

        /* Pack buffer with KA object. */
        cops_client_accept(msg, estab_ka_timer, estab_ac_timer);

        /* Client-Accept header flags. */
        TP_ASSERT(msg[0] == 16);

        /* Client-Accept Opcode. */
        TP_ASSERT(msg[1] == 7);

        /* Default client-type value. */
        uint16_t client_type = unpack_u8(msg + 2);
        TP_ASSERT(client_type == 32778);

        /* Immediately post-header, assert length of the message. */
        uint32_t tot_len = unpack_u32(msg + 4);
        TP_ASSERT(tot_len == 24);

        /* KA-object assertions. */
        uint16_t ka_len = unpack_u8(msg + 8);
        uint32_t ka_timer = unpack_u32(msg + 12);
        TP_ASSERT(ka_len == 8);
        TP_ASSERT(ka_timer == estab_ka_timer);
        TP_ASSERT(msg[10] == 10); /* ka c-num. */
        TP_ASSERT(msg[11] == 1);  /* ka c-type. */

        /* Account timer assertions. */
        uint16_t ac_len = unpack_u8(msg + 16);
        uint32_t ac_timer = unpack_u32(msg + 20);
        TP_ASSERT(ac_len == 8);
        TP_ASSERT(ac_timer == 15);
        TP_ASSERT(msg[18] == 15); /* ac c-num. */
        TP_ASSERT(msg[19] == 1);  /* ac c-type. */
}

void
tp_cops_keepalive_message(void) {
        info();

        /* Pack keepalive. */
        uint8_t ka[8];
        cops_keepalive(ka);

        /* Unpack keepalive content. */
        uint32_t val = unpack_u32(ka + 4);

        /* Keep-alive assertions. */
        TP_ASSERT(ka[1] == 9);  /* Keep-Alive Opcode. */
        TP_ASSERT(ka[0] == 16); /* Keep-Alive flags. */
        TP_ASSERT(val == 8);    /* Keep-Alive data. */
}

int
test_runner(void) {
        tp_cops_common_handle_object();
        tp_cops_common_context_object();
        tp_cops_common_decision_object();
        tp_cops_client_accept_message();
        tp_cops_keepalive_message();
        tp_cops_report_state_opcode_ok();
        tp_cops_report_state_opcode_to_acronym();
        tp_cops_report_state_opcode_to_string();

        return EXIT_SUCCESS;
}
