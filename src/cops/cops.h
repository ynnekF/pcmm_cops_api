#ifndef COPS_H
#define COPS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#define COPS_COMMON_OBJ_LEN 8

bool cops_header_ok(uint8_t opcode, uint16_t client_type, uint32_t message_len);

/* 
 * Accepts a Op Code value and returns the corresponding COPS Common object name
 * @1 .........Handle Object (Handle)
 * @2 .........Context Object (Context)
 * @3 .........In-Interface Object (IN-Int)
 * @4 .........Out-Interface Object (OUT-Int)
 * @5 .........Reason Object (Reason)
 * @6 .........Decision Object (Decision)
 * @7 .........LPDP Decision Object (LPDPDecision)
 * @8 .........Error Object (Error)
 * @9 .........Client Specific Information Object (ClientSI)
 * @10 .........Keep-Alive Timer Object (KATimer)
 * @11 .........PEP Identification Object (PEPID)
 * @12 .........Report-Type Object (Report-Type)
 * @13 .........PDP Redirect Address (PDPRedirAddr)
 * @14 .........Last PDP Address (LastPDPAddr)
 * @15 .........Accounting Timer Object (AcctTimer)
 * @16 .........Message Integrity Object (Integrity)
 */
char* cops_otos(unsigned char opcode);

/* 
 * Accepts a Op Code value and returns the corresponding PEP to PDP message name
 * The COPS operations:
 * @1 = Request..........................(REQ)
 * @2 = Decision.........................(DEC)
 * @3 = Report State.....................(RPT)
 * @4 = Delete Request State.............(DRQ)
 * @5 = Synchronize State Req............(SSQ)
 * @6 = Client-Open......................(OPN)
 * @7 = Client-Accept....................(CAT)
 * @8 = Client-Close......................(CC)
 * @9 = Keep-Alive........................(KA)
 * @10= Synchronize Complete.............(SSC)
 */
char* cops_otoa(unsigned char opcode);

/*
 * Return 1 or 0 depending on whether the given Op Code is valid.
 * Op Code: 8 bits
 * The COPS operations:
 * @1 = Request..........................(REQ)
 * @2 = Decision.........................(DEC)
 * @3 = Report State.....................(RPT)
 * @4 = Delete Request State.............(DRQ)
 * @5 = Synchronize State Req............(SSQ)
 * @6 = Client-Open......................(OPN)
 * @7 = Client-Accept....................(CAT)
 * @8 = Client-Close......................(CC)
 * @9 = Keep-Alive........................(KA)
 * @10= Synchronize Complete.............(SSC)
 *
 * Note: RFC 2748 The COPS Protocol, outlines Op codes 1-10, but the PCMM spec.
 *       only references the below values (Codes 5 and 10 are omitted).
 */
bool cops_opcode_ok(uint8_t opcode);

/*
 * C-Num identifies the class of information contained in the object, and the C-Type identifies
 * the subtype or version of the information contained in the object.  Standard COPS objects (as
 * defined in [IETF RFC 2748]) used in this specification, and their C-Num values, are:
 *  @1 = Handle
 *  @2 = Context
 *  @6 = Decision
 *  @8 = Error
 *  @9 = Client Specific Info
 *  @10 = Keep-Alive-Timer
 *  @11 = PEP Identificatio
 */
bool cops_class_ok(uint8_t cnum, uint8_t ctype);

/*
 * Pack the destination buffer with a COPS Handle object (C-NUm = 1, C-Type = 1)
 *
 * @dst         Destination buffer
 * @handle      PEP specified client handle
 */
void cops_handle(uint8_t* dst, const char* handle);

/*
 * Pack the destination buffer with a COPS Context object (C-Num = 2, C-Type = 1)
 *
 * R-Type (Request Type Flag):
 *      0x01 = Incoming-Message/Admission Control request
 *      0x02 = Resource-Allocation request
 *      0x04 = Outgoing-Message request
 *      0x08 = Configuration request
 * M-Type (Message Type):
 *      Client Specific 16 bit values of protocol message types
 *              0              1             2              3
 *      +--------------+--------------+--------------+--------------+
 *      |            R-Type           |            M-Type           |
 *      +--------------+--------------+--------------+--------------+
 *
 * @dst         Destination buffer
 */
void cops_context(uint8_t* dst);

/*
 * COPS Object Decision: C-Num = 6, C-Type = 1, Decision Flags (Mandatory
 *
 * Decision made by the PDP. Appears in replies. The specific non-
 * mandatory decision objects required in a decision to a particular
 * request depend on the type of client.
 * Commands:
 *      0 = NULL Decision (No configuration data available)
 *      1 = Install (Admit request/Install configuration)
 *      2 = Remove (Remove request/Remove configuration)
 *
 * Flags:
 *      0x01 = Trigger Error (Trigger error message if set)
 *              Note: Trigger Error is applicable to client-types that
 *              are capable of sending error notifications for signaled
 *              messages.
 *
 *              0              1             2              3
 *      +--------------+--------------+--------------+--------------+
 *      |        Command-Code         |            Flags            |
 *      +--------------+--------------+--------------+--------------+
 *
 *  The PDP responds to the REQ with a DEC message that includes the
 *  associated client handle and one or more decision objects grouped
 *  relative to a Context object and Decision Flags object type pair. If
 *  there was a protocol error an error object is returned instead.
 *
 * @dst         Destination buffer
 */
void cops_decision(uint8_t* dst);

/*
 * Pack a Client-Accept Message for the PEP (CMTS)
 *
 * After receiving an OPN (Client-Open) message from the PEP, the PDP will send
 * a Client-Accept message if the protocol version is specified in the Version
 * Info object is supported. This message contains the Keep-Alive Timer object.
 *
 * @cnum    Client-Accept valid C-Num value (10/15)
 * @ctype   Client-Accept valid C-Type value (1)
 */
void cops_client_accept(uint8_t* dst, const uint32_t ka_timer, const uint32_t acct_timer);

/* Build the keep-alive object. */
void cops_keepalive(uint8_t* dst);

/* 
 * Finalzie a COPS message with the correct headers. 
 *
 * Version is a 4-bit field giving the current COPS version number. This field MUST be set to 1. Flags is 
 * a 4-bit field. When a COPS message is sent in response to another message (e.g., a solicited decision sent in 
 * response to a request) this flag MUST be set to 1.
 *
 * Op-code is a 1-byte unsigned integer field that gives the COPS operation to be performed.
 *
 * +-------------------+---------------+------------------+----------------------------+
 * | Version (4-bit)   | Flags (4-bit) | Op-Code (1-byte) | Client-Type (2-byte)       |
 * +-------------------+---------------+------------------+----------------------------+
 * |                                 Message Length                                    |
 * +-----------------------------------+------------------+----------------------------+
 * | Length  (2-bytes)                 | C-Num (1-byte)        | C-Type (1-byte)       |
 * +-----------------------------------+------------------+----------------------------+
 *
 * Client-Type is a 2-byte unsigned integer identifier. For PacketCable Multimedia use, the Client-Type
 * MUST be set to PacketCable Multimedia client (0x800A). For Keep-Alive messages (Op-code = 9) the Client
 * -Type MUST be set to zero, as the KA is used for connection verification rather than per-client session 
 * verification.
 *
 * Message Length is a 4-byte unsigned integer value giving the size of the overall message in octets.
 *  Messages MUST be aligned on 4-byte boundaries, so the length MUST be a multiple of four.
 *
 * Following the COPS common header are one or more objects. All the objects MUST conform to the same
 * object format where each object consists of one or more 4-byte words with a four-octet header, using
 * the following format.
 *
 */
void new_cops_message(uint8_t* dst, uint16_t opcode, uint8_t* data, int length);

/* 
 * Pack the client-specific and additional COPS objects into the dest. buffer. 
 *
 * These objects are encoded similarly to the client-specific objects for COPS-PR and
 * as in COPS-PR these objects are numbered using a client-specific number space, which
 * is independent of the top-level COPS object number space. For this reason, the object
 * numbers and types are given as S-Num and S-Type, respectively. S-Num and S-Type MUST
 * be one octet. The COPS Length field MUST be two octets.
 */
size_t pack_ctl_objs(uint8_t* dst, uint8_t* handle, uint8_t* context, uint8_t* decision, uint8_t* command,
                     uint8_t* application, uint8_t* subscriber, size_t decision_length, size_t ip_length);

#endif
