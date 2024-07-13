#include "cops.h"

/*
 * COPS Common Object format utility - Populate the given destination buffer (uint8_t) with the
 * provided S-Num and S-Type OR C-Num and C-Type values. The buffer should expect the first two
 * bytes be populated by a default length of 8, while the next two bytes are reserved for the
 * unique object ID S/C ID values.
 */
static void cops_oid(uint8_t* dst, uint8_t num, uint8_t type);

static void
concat(uint8_t* dst, size_t* sz_ptr, uint8_t* src, size_t n) {
        size_t i;
        size_t j = 0;

        /* Append the source bytes to the dst starting at the pointer. */
        for (i = *sz_ptr; i < (*sz_ptr + n); i++) {
                *(dst + i) = *(src + (j++));
        }
        dst[i] = '\0';

        *sz_ptr += (int)n;
}

static void
cops_packlen(uint8_t* dest, uint16_t val) {
        uint8_t* ptr = (uint8_t*)dest;

        /* Store the most significant byte. */
        *(ptr) = (val >> 8) & 0xFF;

        /* Store the least significant byte. */
        *(ptr + 1) = val & 0xFF;
}

static inline void
cops_oid(uint8_t* dst, uint8_t num, uint8_t type) {
        cops_packlen(dst, COPS_COMMON_OBJ_LEN);

        /* Store identifiers. */
        *(dst + 2) = num;
        *(dst + 3) = type;
}

bool
cops_opcode_ok(uint8_t opcode) {
        switch (opcode) {
                case 1:
                case 2:
                case 3:
                case 4:
                case 6:
                case 7:
                case 8:
                case 9:  return true;
                default: return false;
        }
}

bool
cops_header_ok(uint8_t opcode, uint16_t client_type, uint32_t message_len) {
        return (cops_opcode_ok(opcode) && client_type == 32778 && message_len != 0 && message_len < 1000) ||
               (opcode == 9 && client_type == 0);
}

bool
cops_class_ok(uint8_t cnum, uint8_t ctype) {
        switch (cnum) {
                case 1:
                case 2:
                case 6:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                        switch (ctype) {
                                case 1:
                                case 2:
                                case 3:  return true;
                                default: return false;
                        }
                default: return false;
        }
}

char*
cops_otoa(unsigned char opcode) {
        switch ((int)opcode) {
                case 1:  return "REQ";
                case 2:  return "DEC";
                case 3:  return "RPT";
                case 4:  return "DRQ";
                case 5:  return "SSQ";
                case 6:  return "OPN";
                case 7:  return "CAT";
                case 8:  return "CC";
                case 9:  return "KA";
                case 10: return "SSC";
                default: return "NIL";
        }
}

char*
cops_otos(unsigned char opcode) {
        switch ((int)opcode) {
                case 1:  return "Handle";
                case 2:  return "Context";
                case 3:  return "In Interface";
                case 4:  return "Out Interface";
                case 5:  return "Reason Code";
                case 6:  return "Decision";
                case 7:  return "LPDP Decision";
                case 8:  return "Error";
                case 9:  return "Client Specific Info";
                case 10: return "Keep-Alive Timer";
                case 11: return "PEP Identification";
                case 12: return "Report Type";
                case 13: return "PDP Redirect Address";
                case 14: return "Last PDP Address";
                case 15: return "Accounting Timer";
                case 16: return "Message Integrity";
                default: return "NIL";
        }
}

void
cops_handle(uint8_t* dst, const char* handle) {
        cops_oid(dst, 1, 1);

        /* Client handle used to uniquely identify a particular
         * PEP's request for a client-type. */
        memcpy(dst + 4, handle, 4);
}

void
cops_context(uint8_t* dst) {
        cops_oid(dst, 2, 1);
        uint16_t enc_rt = htons(8); /* Context request-type. */
        uint16_t enc_mt = htons(0); /* Context message-type. */

        /* Combine flags. */
        char combo[4];
        memcpy(combo, &enc_rt, 2);
        memcpy(combo + 2, &enc_mt, 2);
        memcpy(dst + 4, &combo, 4);
}

void
cops_decision(uint8_t* dst) {
        cops_oid(dst, 6, 1);

        uint16_t values[2] = {htons(1), htons(1)};
        memcpy(dst + 4, values, 4);
}

size_t
pack_ctl_objs(uint8_t* dst, uint8_t* handle, uint8_t* context, uint8_t* decision, uint8_t* command,
              uint8_t* application, uint8_t* subscriber, size_t decision_length, size_t ip_length) {
        size_t i = 0;

        /* Client Specific objects */
        concat(dst, &i, handle, 8);
        concat(dst, &i, context, 8);
        concat(dst, &i, decision, 8);

        /* Decision headers. */
        cops_packlen((uint8_t*)dst + i, decision_length);
        i += 2;

        dst[i++] = 6; /* opcode */
        dst[i++] = 4; /* client_type */

        concat(dst, &i, command, 8);
        concat(dst, &i, application, 8);
        concat(dst, &i, subscriber, ip_length);

        return i;
}

void
cops_client_accept(uint8_t* dst, const uint32_t ka_timer, const uint32_t acct_timer) {
        uint8_t* ptr = (uint8_t*)dst;

        uint8_t v = 1;
        uint8_t f = 0;

        int len = (acct_timer == 0) ? 16 : 24;
        int header_v = (v << 4) | f;

        unsigned int client_type = 32778;

        /* Header. */
        *(ptr) = header_v;
        *(ptr + 1) = 7; /* Client-Accept Opcode. */
        *(ptr + 2) = (uint32_t)(client_type >> 8) & 0xFF;
        *(ptr + 3) = client_type & 0xFF;

        /* Pack length in network byte order. */
        *(ptr + 4) = (len >> 24) & 0xFF;
        *(ptr + 5) = (len >> 16) & 0xFF;
        *(ptr + 6) = (len >> 8) & 0xFF;
        *(ptr + 7) = len & 0xFF;

        /* KA Object length. */
        *(ptr + 8) = (8 >> 8) & 0xFF;
        *(ptr + 9) = 8 & 0xFF;
        *(ptr + 10) = 10; /* Keep-Alive timer C-Num. */
        *(ptr + 11) = 1;  /* Keep-Alive timer C-Type. */

        /* Pack KA Timer in network byte order. */
        *(ptr + 12) = (ka_timer >> 24) & 0xFF;
        *(ptr + 13) = (ka_timer >> 16) & 0xFF;
        *(ptr + 14) = (ka_timer >> 8) & 0xFF;
        *(ptr + 15) = ka_timer & 0xFF;

        if (acct_timer == 0)
                return;

        /* Acct. Object length. */
        *(ptr + 16) = (8 >> 8) & 0xFF;
        *(ptr + 17) = 8 & 0xFF;
        *(ptr + 18) = 15; /* Accounting timer C-Num. */
        *(ptr + 19) = 1;  /* Accounting timer C-Type. */

        /* Pack length in network byte order. */
        *(ptr + 20) = (acct_timer >> 24) & 0xFF;
        *(ptr + 21) = (acct_timer >> 16) & 0xFF;
        *(ptr + 22) = (acct_timer >> 8) & 0xFF;
        *(ptr + 23) = acct_timer & 0xFF;
}

void
cops_keepalive(uint8_t* dst) {
        int header_v = (1 << 4) | 0;
        int offset = 0;
        dst[offset++] = header_v;
        dst[offset++] = 9; /* Keep-Alive Opcode. */
        dst[offset++] = (uint32_t)(0 >> 8) & 0xFF;
        dst[offset++] = 0 & 0xFF;

        /* Pack length in network byte order. */
        dst[offset++] = (8 >> 24) & 0xFF;
        dst[offset++] = (8 >> 16) & 0xFF;
        dst[offset++] = (8 >> 8) & 0xFF;
        dst[offset++] = 8 & 0xFF;
}

void
new_cops_message(uint8_t* dst, uint16_t opcode, uint8_t* data, int length) {
        int offset = 0;
        uint8_t flags = 0;
        uint32_t client_type = 32778;

        int version = (1 << 4) | flags;
        dst[offset++] = version;
        dst[offset++] = opcode;
        dst[offset++] = (uint32_t)(client_type >> 8) & 0xFF;
        dst[offset++] = client_type & 0xFF;
        dst[offset++] = (length >> 24) & 0xFF;
        dst[offset++] = (length >> 16) & 0xFF;
        dst[offset++] = (length >> 8) & 0xFF;
        dst[offset++] = length & 0xFF;
        if (data)
                memcpy(dst + offset, data, length - 8);
}
