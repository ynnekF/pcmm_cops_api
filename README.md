# Common Open Policy Service API

The COPS API implements standard COPS Decision message management/packing specifically for Policy Server to CMTS communication interfaces. All Policy Server messages being sent to a CMTS must be formatted as Decision messages, while all CMTS to Policy Server messages will be formatted as Report-State messages.

This specific API implements two files, `cops.c` and `cops.h`, which can be used in conjunciton with a larger service to implement
basic QoS services according to the PCMM (PacketCable Multimedia) I09 Specification.

### COPS Common Message Format

Each COPS message consists of the COPS header followed by a number of typed objects. The Application Manager, Policy Server
and CMTS MUST use the COPS Common Message format as defined below as the message format for all message exchanges. In the object specifications that follow, each row represents a 4-byte word as all objects align on 4-byte word boundaries.
```
+-------------------+---------------+------------------+----------------------------+
| Version (4-bit)   | Flags (4-bit) | Op-Code (1-byte) | Client-Type (2-byte)       |
+-------------------+---------------+------------------+----------------------------+
|                                 Message Length                                    |
+-----------------------------------+------------------+----------------------------+
| Length  (2-bytes)                 | C-Num (1-byte)   | C-Type (1-byte)            |
+-----------------------------------+------------------+----------------------------+
```

**`Version`** is a 4-bit field giving the current COPS version number. This field MUST be set to 1.<br>

**`Flags`** is a 4-bit field. The least significant bit is the solicited message flag. When a COPS message is sent in response
to another message (e.g., a solicited decision sent in response to a request) this flag MUST be set to 1. In other cases (e.g.,
an unsolicited decision) the flag MUST NOT be set (value = 0). In keeping with the DQoS model, the first Decision message sent
in response to a Request message is a solicited response and its solicited message flag MUST be set. All other Decision messages
are unsolicited and the solicited message flag MUST be cleared. All other flags MUST be set to zero.<br>

**`Op-code`** is a 1-byte unsigned integer field that gives the COPS operation to be performed
* 1 = Request (REQ)
* 2 = Decision(DEC)
* 3 = Report-State (RPT)
* 4 = Delete Request State (DRQ)
* 6 = Client-Open (OPN)
* 7 = Client-Accept (CAT)
* 8 = Client-Close (CC)
* 9 = Keep-Alive (KA)

### Handle `cops_handle`

The handle object encapsulates a unique value that identifies an installed state. This ID is used by most COPS operations. 
```
+--------------+--------------+--------------+
| Length = 8   | C-Num = 1    | C-Type = 1   |
+--------------+--------------+--------------+
|                   Handle                   |
+--------------------------------------------+
```
The client handle is used to refer to a request state initiated by a particular PEP and installed at the PDP for a client-type. 
A PEP will specify a client handle in its Request messages, Report messages and Delete messages sent to the PDP. In all cases,
the client handle is used to uniquely identify a particular PEP's request for a client-type. The PEP establishes a request state
client handle for which the remote PDP may maintain state. The remote PDP then uses this handle to refer to the exchanged 
information and decisions communicated over the TCP connection to a particular PEP for a given client-type. Once a stateful
handle is established for a new request, any subsequent modifications of the request can be made using the REQ message specifying
the previously installed handle.
  
### Context `cops_context`

R-Type (Request Type Flag):
* 0x01 = Incoming-Message/Admission Control request
* 0x02 = Resource-Allocation request
* 0x04 = Outgoing-Message request
* 0x08 = Configuration request

M-Type (Message Type): Client Specific 16 bit values of protocol message types
```
      0              1             2              3
+--------------+--------------+--------------+--------------+
|            R-Type           |            M-Type           |
+--------------+--------------+--------------+--------------+
```
### Decision `cops_decision`

 COPS Object Decision: C-Num = 6, C-Type = 1, Decision Flags Mandatory

Decision made by the PDP. Appears in replies. The specific non-mandatory decision objects required in a decision to a particular
request depend on the type of client.

Commands:
* 0 = NULL Decision (No configuration data available)
* 1 = Install (Admit request/Install configuration)
* 2 = Remove (Remove request/Remove configuration)

Flags:
* 0x01 = Trigger Error (Trigger error message if set)

Note: Trigger Error is applicable to client-types that
are capable of sending error notifications for signaled
messages.
 
```
      0              1             2              3
+--------------+--------------+--------------+--------------+
|        Command-Code         |            Flags            |
+--------------+--------------+--------------+--------------+

```
The PDP responds to the REQ with a DEC message that includes the associated client handle and one or more decision objects
grouped relative to a Context object and Decision Flags object type pair. If there was a protocol error an error object is returned
instead.

### Client-Accept `cops_client_accept`
tbd

### Keep-Alive `cops_keepalive`
tbd