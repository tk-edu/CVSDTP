# cvsdtp (working title)
## Cooperative Visual Search Data Transfer Protocol

## Purpose
This data transfer protocol exists as a means by which cooperative visual search task information may be transferred over a LAN of two computers running the visual search task software. It exists on top of the TCP/IP protocols.

## Header
The header of each packet is defined as follows:
| Bits / Bytes | Field                   | Possible Values                                                                                                              |
|--------------|-------------------------|------------------------------------------------------------------------------------------------------------------------------|
|0-1           |Packet type              |<li>__00__ - Initialization</li><li>__01__ - Synchronization / Update</li><li>__10__ - UNUSED</li><li>__11__ - Completion</li>|
|3             |Sender's identity        |<li>__0__ - PC1</li><li>__1__ -  PC2</li>                                                                                     |
|4             |Receiver's identity      |<li>__0__ - PC1</li><li>__1__ -  PC2</li>                                                                                     |

## Data
The data section of each packet type starts at 0x01 (offsets in table are relative to this), and are defined as follows:

### Initialization
| Bits / Bytes | Field                   | Possible Values                          |
|--------------|-------------------------|------------------------------------------|
|0x00-0x10     |Sender's target state    |*see target state specification below*    |
|0x11-0x20     |Sender's distractor state|*see distractor state specification below*|

### Synchronization / Update
| Bits / Bytes | Field                   | Possible Values                          |
|--------------|-------------------------|------------------------------------------|
|0x00-0x04     |Target to update         |*see target specification below*          |
|0x05-0x15     |Sender's target state    |*see target state specification below*    |

### Target State
list of (x, y) coordinates, shape, and color(?)

### Distractor State
list of (x, y) coordinates, shape, and color(?)

## Initialization Sequence
- Both instances are started, listening to each other and each sending their own random seed
- One of the instances happens to receive 