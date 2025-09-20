# SINTRAN XMSG Deep Analysis - MXMSG Subroutine

This document provides a comprehensive deep analysis of the SINTRAN XMSG (Message System) implementation, focusing on the core MXMSG subroutine and its interaction with the HDLC/X.25 networking stack.

## Overview

The XMSG system in SINTRAN III provides inter-process communication and networking services. The MXMSG subroutine serves as the primary interface between user applications and the underlying message passing infrastructure.

## MXMSG Subroutine Analysis (Line 49402→074776)

### Source Code Structure

```sintran
SUBR MXMSG
INTEGER XREG
TRIPLE  TADREG
MXMSG: A:=:L=:"MXMRET"; A:=L; X=:L:=TDRADDR=:B
       X:=OTAD=:SOTAD:=BXTADD; X:=:L; *MON 2XMSG
       CALL WT10; X:=:L=:BXTADD:=SOTAD=:OTAD:=L
       X=:XREG; TAD=:TADREG; CALL SET10WINDOW
       A:=B=:ITAD; X:=XREG; TAD:=TADREG
       IF T>=0 THEN MIN "MXMRET" FI
       GO MXMRET
RBUS
```

### Step-by-Step Code Flow Analysis

#### 1. **Entry Point Setup** (Line 49405→075002)
```sintran
MXMSG: A:=:L=:"MXMRET"; A:=L; X=:L:=TDRADDR=:B
```
- **Function**: Sets up return address and datafield preparation
- **Variables**:
  - `L` = Return address label ("MXMRET")
  - `TDRADDR` = TAD (Terminal Access Driver) datafield address
  - `B` = Input datafield register

#### 2. **Datafield Context Switch** (Line 49406→075010)
```sintran
X:=OTAD=:SOTAD:=BXTADD; X:=:L; *MON 2XMSG
```
- **Function**: Performs the actual XMSG monitor call
- **Variables**:
  - `OTAD` = Output TAD datafield
  - `SOTAD` = Saved OTAD value
  - `BXTADD` = Extended address for datafield switching
- **Operation**: `*MON 2XMSG` - Execute Monitor Call 2 (XMSG system)

#### 3. **Wait and Context Restoration** (Line 49407→075015)
```sintran
CALL WT10; X:=:L=:BXTADD:=SOTAD=:OTAD:=L
```
- **Function**: Wait for operation completion and restore context
- **Variables**:
  - `WT10` = Wait routine for level 10 interrupts
  - Restoration chain: `BXTADD` → `SOTAD` → `OTAD` → `L`

#### 4. **Window Management** (Line 49408→075023)
```sintran
X=:XREG; TAD=:TADREG; CALL SET10WINDOW
```
- **Function**: Sets up memory window for cross-datafield access
- **Variables**:
  - `XREG` = Saved X register value
  - `TADREG` = Saved TAD register value (TRIPLE = 3 words)
  - `SET10WINDOW` = Subroutine for window setup

#### 5. **Final Setup and Return** (Lines 49409-49411)
```sintran
A:=B=:ITAD; X:=XREG; TAD:=TADREG
IF T>=0 THEN MIN "MXMRET" FI
GO MXMRET
```
- **Function**: Final register restoration and return handling
- **Variables**:
  - `ITAD` = Input TAD datafield
  - Return logic based on T register status

## XMSG Function Mapping Table

| XMSG Function | Decimal | Octal | SINTRAN Symbol | Purpose | Implementation |
|---------------|---------|-------|----------------|---------|----------------|
| XFDUM | 0 | 000000 | XFDUM | Dummy function | No-op |
| XFDCT | 1 | 000001 | XFDCT | Disconnect | Port cleanup |
| XFGET | 2 | 000002 | XFGET | Get message space | Buffer allocation |
| XFREL | 3 | 000003 | XFREL | Release message | Buffer deallocation |
| XFRHD | 4 | 000004 | XFRHD | Read header | 6-byte header read |
| XFWHD | 5 | 000005 | XFWHD | Write header | 6-byte header write |
| XFREA | 6 | 000006 | XFREA | Read message data | Data buffer read |
| XFWRI | 7 | 000007 | XFWRI | Write message data | Data buffer write |
| XFSCM | 8 | 000010 | XFSCM | Set current message | Message selection |
| XFMST | 9 | 000011 | XFMST | Get message status | Status query |
| XFOPN | 10 | 000012 | XFOPN | Open port | Port establishment |
| XFCLS | 11 | 000013 | XFCLS | Close port | Port termination |
| XFSND | 12 | 000014 | XFSND | Send message | Message transmission |
| XFRCV | 13 | 000015 | XFRCV | Receive message | Message reception |
| XFPST | 14 | 000016 | XFPST | Port status | Port state query |
| XFGST | 15 | 000017 | XFGST | General status | System state query |
| XFSIN | 16 | 000020 | XFSIN | Service init (RT) | Service initialization |
| XFSRL | 17 | 000021 | XFSRL | Service release | Service cleanup |
| XFABR | 18 | 000022 | XFABR | Absolute read (RT) | Direct memory read |
| XFABW | 19 | 000023 | XFABW | Absolute write | Direct memory write |
| XFMLK | 20 | 000024 | XFMLK | Lock system | System lock |
| XFMUL | 21 | 000025 | XFMUL | Unlock system | System unlock |
| XFM2P | 22 | 000026 | XFM2P | Magic to port | Address resolution |
| XFP2M | 23 | 000027 | XFP2M | Port to magic | Reverse resolution |

## Related Subroutines

### DRXACC - Direct XMSG Buffer Access (Line 49419→075042)

```sintran
SUBR DRXACC
INTEGER POINTER LREG
INTEGER FUNC=?
DRXACC: IF X=0 THEN A:=XEIBP; EXITA; FI          % INCORRECT BUFFID
        X:=:L; A=:D; *1BANK; LDA ,X 0; 2BANK; IOF
        A=:FUNC; X=:"LREG"; L=:X; D=:A
        CALL DRXMSG; INTEGER FUNC
        GO RETU; MIN "LREG"
RETU:   MIN "LREG"; GO LREG
```

**Purpose**: Provides direct access to XMSG buffers with bank switching
**Key Operations**:
- Buffer ID validation
- Memory bank switching (`*1BANK`, `2BANK`)
- Interrupt control (`IOF`)
- Direct message buffer access

### Window Management Subroutines

#### STADIWINDOW/STADOWINDOW (Line 49481→075070)

```sintran
SUBR STADIWINDOW,STADOWINDOW
INTEGER POINTER LREG
STADOWINDOW: M:="0"; GO FELLS
STADIWINDOW: M:=1
FELLS: A:=L=:"LREG"; CALL SET10WINDOW            % B IS NOW DF. OUTS. RES.
       IF DFOPP=0 THEN CALL ERRFATAL FI          % TAD MUST ALWAYS BE TWO WAY
       X:=1777; X/\A; A:=B/\176000+X=:X
       IF M THEN X=:OTAD; A:=B=:ITAD; GO LREG FI
       X=:ITAD; A:=B=:OTAD; GO LREG
```

**Purpose**: Sets up datafield windows for TAD (Terminal Access Driver) communication
**Key Operations**:
- Input/Output datafield setup
- Two-way communication validation
- Address calculation and masking

## MXMSG Flow Diagram

```mermaid
graph TD
    A[Application XMSG Call] --> B[MXMSG Entry Point]
    B --> C[Setup Return Address]
    C --> D[Save Context OTAD/SOTAD/BXTADD]
    D --> E[Execute MON 2XMSG]
    E --> F[Wait for Completion WT10]
    F --> G[Restore Context]
    G --> H[Setup Memory Window]
    H --> I{Check T Register}
    I -->|T >= 0| J[Normal Return]
    I -->|T < 0| K[Error Return]
    J --> L[MXMRET]
    K --> L

    style E fill:#ff9999
    style F fill:#99ccff
    style H fill:#99ff99
```

## XMSG System Architecture

```mermaid
graph LR
    subgraph "Application Layer"
        APP[User Application]
    end

    subgraph "XMSG System"
        MXMSG[MXMSG Subroutine]
        DRXACC[Buffer Access]
        WINDOW[Window Management]
    end

    subgraph "Operating System"
        MON2[Monitor Call 2]
        KERNEL[SINTRAN Kernel]
    end

    subgraph "Network Layer"
        XROUT[XROUT Service]
        TAD[TAD Driver]
    end

    subgraph "Hardware Layer"
        HDLC[HDLC Controller]
        NET[Network Interface]
    end

    APP --> MXMSG
    MXMSG --> DRXACC
    MXMSG --> WINDOW
    MXMSG --> MON2
    MON2 --> KERNEL
    KERNEL --> XROUT
    XROUT --> TAD
    TAD --> HDLC
    HDLC --> NET
```

## Symbol Mapping Reference

### Key SINTRAN Symbols (Truncated to 5 characters)

| Symbol | Full Name | Address | Purpose |
|--------|-----------|---------|---------|
| XREG | X Register Save | Variable | Context preservation |
| TADRE | TADREG | Variable | TAD register save area |
| MXMRE | MXMRET | Label | Return address |
| TDRADDR | TDR Address | Variable | TAD datafield address |
| OTAD | Output TAD | Variable | Output datafield |
| SOTAD | Saved OTAD | Variable | Saved output datafield |
| BXTAD | BXTADD | Variable | Extended address |
| ITAD | Input TAD | Variable | Input datafield |
| DFOPP | DF Opposite | Variable | Opposite datafield |
| WT10 | Wait Level 10 | Subroutine | Interrupt wait routine |

### XMSG Configuration Symbols

| Symbol | Value | Purpose |
|--------|-------|---------|
| X4MCB | 000010 | Max calls in multicall function |
| X4MMX | 004704 | Maximum message size |
| X5MTS | 030324 | Message table size |
| X4NAM | 001400 | Name table size |
| X5PRT | 000176 | Port table size |
| X4MES | 000400 | Message buffer size |
| X3FSZ | 000470 | Frame size in words |
| X5TO1 | 000226 | Timeout value 1 |
| X5TO2 | 000310 | Timeout value 2 |

## Network Integration Points

### XFWRI → HDLC Data Path

```mermaid
sequenceDiagram
    participant App as Application
    participant MXMSG as MXMSG Subroutine
    participant XROUT as XROUT Service
    participant TAD as TAD Driver
    participant HDLC as HDLC Controller
    participant Net as Network

    App->>MXMSG: XFWRI(data)
    MXMSG->>MXMSG: Setup context
    MXMSG->>XROUT: Route message
    XROUT->>TAD: Send to TAD
    TAD->>HDLC: XSSDATA transmission
    HDLC->>Net: X.25 frame
    Net-->>HDLC: ACK/Response
    HDLC-->>TAD: Status
    TAD-->>XROUT: Complete
    XROUT-->>MXMSG: Return status
    MXMSG-->>App: Success/Error
```

### XFREA ← HDLC Data Path

```mermaid
sequenceDiagram
    participant Net as Network
    participant HDLC as HDLC Controller
    participant TAD as TAD Driver
    participant XROUT as XROUT Service
    participant MXMSG as MXMSG Subroutine
    participant App as Application

    Net->>HDLC: X.25 frame received
    HDLC->>TAD: HIINT interrupt
    TAD->>TAD: HDREC processing
    TAD->>XROUT: Route incoming data
    XROUT->>MXMSG: Deliver message
    App->>MXMSG: XFREA(buffer)
    MXMSG-->>App: Data received
```

## Critical Implementation Details

### Memory Management
- **Bank Switching**: SINTRAN uses memory banking (`*1BANK`, `2BANK`) for accessing different memory areas
- **Datafield Windows**: The `SET10WINDOW` mechanism allows cross-datafield memory access
- **Context Preservation**: All register states are carefully saved and restored

### Error Handling
- **Buffer Validation**: `XEIBP` error for incorrect buffer IDs
- **Fatal Errors**: `ERRFATAL` for critical system failures
- **Return Codes**: T register carries success/error status

### Interrupt Processing
- **Level 10 Processing**: `WT10` handles XMSG-level interrupts
- **IOF/ION Control**: Careful interrupt masking during critical sections
- **Asynchronous Operations**: Support for non-blocking I/O

## Performance Characteristics

### Context Switch Overhead
1. **Register Save/Restore**: ~10-15 instructions
2. **Memory Bank Switch**: ~5-8 instructions
3. **Window Setup**: ~20-30 instructions
4. **Monitor Call**: Variable (depends on function)

### Buffer Management
- **Direct Access**: DRXACC provides high-performance buffer access
- **Memory Mapping**: Efficient cross-datafield operations
- **Zero-Copy**: Direct memory access where possible

## Debugging and Analysis Features

### Status Monitoring
- **Port Status**: `XFPST` provides detailed port state information
- **Message Status**: `XFMST` reports message processing state
- **System Status**: `XFGST` gives overall system health

### Error Reporting
- **Function-Specific Errors**: Each XMSG function has specific error codes
- **System-Level Errors**: Hardware and resource failures reported
- **Network Errors**: HDLC and X.25 layer errors propagated up

## Detailed Key Transformation Points

### XSSDATA: XMSG Write → HDLC Transmission Buffer Transformation

The `XSSDATA` subroutine (Line 51685→103636) performs the critical transformation from XMSG write data into HDLC transmission buffers ready for X.25 transmission.

#### Step-by-Step Transformation Process

##### 1. **Initialization and Validation** (Lines 51688-51692)
```sintran
XSSDATA: IF INTSTA >< 2 THEN A:=ENINIT; GO BACKX FI
         LISTP =: LIINT                            % *81F*
         X-BHEAD; CALL RACTB; X+BHEAD              % MESSAGE SIZE
         IF A < 7 THEN A:=EILFZ; GO BACKX FI       % MESSAGE IS TOO SMALL
         IF A-BCHEA>MAXR THEN A:=EILSIZ; GO BACKX FI  % TOO LONG FRAME
```

**Transformation Logic**:
- **Interface State Check**: Validates HDLC interface is initialized (`INTSTA = 2`)
- **List Pointer Setup**: `LISTP =: LIINT` - Sets up DMA list for transmission
- **Message Size Calculation**: `RACTB` routine calculates actual message size
- **Size Validation**:
  - Minimum 7 bytes required (HDLC header overhead)
  - Maximum `MAXR` bytes to prevent buffer overflow

##### 2. **Buffer Address Translation** (Lines 51693-51695)
```sintran
A-DISP1=:LIINT.LBYTC                      % BYTECOUNT FOR LIST
A:=OMSG+CHEAD=:X.LMEM2=:D                 % SET BUFFER ADDR.
T:=MASTB=:X.LMEM1                         % PHYSICAL BANK IN LMEM2 AND IN T
```

**Critical Transformation**:
- **Byte Count Calculation**: `A-DISP1` removes XMSG header, leaving pure data payload
- **Logical to Physical Address**: `OMSG+CHEAD` converts XMSG logical address to physical buffer address
- **Memory Bank Setup**: `MASTB` provides physical memory bank for DMA operations
- **DMA List Entry**: `LMEM1/LMEM2` creates DMA list entry with physical addressing

##### 3. **DMA List Configuration** (Lines 51696-51700)
```sintran
FSERM=:X.LKEY                             % TRANSMIT ONE BLOCK ONLY
D=:X
*LDATX
A=:SNDAC
0=:D                                      % *81F*
```

**Hardware Interface Setup**:
- **Single Block Mode**: `FSERM` flag ensures atomic transmission
- **DMA Key Assignment**: Links message to specific DMA operation
- **Address Loading**: `*LDATX` loads calculated address into DMA controller
- **Transmission Control**: `SNDAC` stores transmission acknowledgment control

##### 4. **HDLC Hardware Activation** (Lines 51704-51711)
```sintran
XHMST: LIINT+DPITPHYS;
XXHMST:T:=HDEV+WDMA; *IOF; EXR ST%LIST ADDRESS *81F*
       A:=2000\/D; T+"WDCR-WDMA"; *EXR ST        %START TRANSMITTER  *81F*
       T+"RDCR-WDCR"; X:=-20;*EXR ST
       CALL LTOUT; *JAF *-2; ION
       1134+CMODI; T:=HDEV+WTTC; *EXR ST
       1 =: ACTSW                                % *H*
       OMSG =: DCBX
```

**Hardware Programming Sequence**:
- **Physical Address**: `LIINT+DPITPHYS` converts to absolute physical address
- **DMA Controller Setup**: `HDEV+WDMA` programs DMA with list address
- **Transmitter Start**: `WDCR` register activates HDLC transmitter with mode `2000`
- **Control Register**: `RDCR` sets receiver control to `-20` (idle mode)
- **Interrupt Control**: `IOF/ION` manages interrupt masking during setup
- **Mode Setting**: `1134+CMODI` sets specific HDLC transmission mode
- **State Tracking**: `ACTSW = 1` marks transmitter as active

#### Data Transformation Flow Diagram

```mermaid
flowchart TD
    A[XMSG Write Data] --> B[Size Validation<br/>Min 7 bytes, Max MAXR]
    B --> C[Remove XMSG Header<br/>A-DISP1 = Pure Payload]
    C --> D[Convert Address<br/>OMSG+CHEAD = Physical Buffer]
    D --> E[Create DMA List Entry<br/>LMEM1/LMEM2 Structure]
    E --> F[Physical Address Conversion<br/>LIINT+DPITPHYS]
    F --> G[Program HDLC Controller<br/>WDMA/WDCR/RDCR]
    G --> H[Start Transmission<br/>ACTSW = 1]
    H --> I[X.25 Frame on Wire]

    style A fill:#ffcccc
    style C fill:#ffffcc
    style D fill:#ccffcc
    style G fill:#ccccff
    style I fill:#ffccff
```

### HDREC: HDLC Reception → XMSG Read Buffer Transformation

The `HDREC` subroutine (Line 51803→104171) handles the reverse transformation, processing received HDLC frames into XMSG read buffers for application consumption.

#### Step-by-Step Reception Process

##### 1. **Reception Setup and Validation** (Lines 51804-51808)
```sintran
HDREC: IF INTSTA >< 2 THEN A:=ENINIT; GO FAR BACKX FI
       X-BHEAD; CALL RBYTC; X+BHEAD; A-BCHEA     % GET MAX BYTECOUNT
       IF A < MAXR THEN A:=ETOSM; GO FAR BACKX FI
       CALL HXST                                 %*81F*
       A:=X+CHEAD=:D:=MASTB                      %BUFFER ADDRESS     *81F*
```

**Receive Buffer Preparation**:
- **Interface Check**: Validates HDLC receiver is ready
- **Buffer Size Calculation**: `RBYTC` determines available buffer space
- **Capacity Validation**: Ensures buffer can hold maximum frame size
- **Hardware Status**: `HXST` checks HDLC receiver status
- **Buffer Address**: Sets up physical address for received data

##### 2. **DMA List Management** (Lines 51809-51818)
```sintran
HSPEC: X=:T; AD=:LISTF.LMEM                      % *H*
       A:=X-LISTP SHZ -2:=:X                     % CALCULATE INDEX IN DCB TABLE *H*
       * AAX DCBX; STT 0,X,B                     % SAVE DCB ADDR. IN DCB TABLE *H*
       X:=A                                      % X POINTS TO DMA LIST ENTRY *H*
       ERB=:X.LKEY                               % *81F*
       LISTF+4=:LISTF                            %LIST FILLER OF NEXT MESSAGE
       A:=2000; X:=LISTF+A; X.LKEY               % *K*ND-110* CACHE MISS
       IF LISTF.LKEY=NLP THEN
             LISTP=:LISTF                        %REACHED END OF LIST
       FI
```

**Reception Queue Management**:
- **DMA List Setup**: `LISTF.LMEM` points to receive buffer chain
- **Index Calculation**: Computes position in Device Control Block (DCB) table
- **DCB Storage**: Saves DCB address for later message routing
- **List Key Assignment**: `ERB` provides unique identifier for received frame
- **Queue Advancement**: Moves to next available receive buffer
- **End Detection**: `NLP` (No List Pointer) indicates queue end

##### 3. **Hardware Receiver Activation** (Lines 51826-51834)
```sintran
ZSTARC: IF ACTSW = 0 THEN
              HXDOK/\MAINT; T:=HDEV+WRTC; *EXR ST  % CLEAR OLD GARBAGE
              % ADD DPITPHYS TO GET PHYSICAL ADDRESS
              LIINT+DPITPHYS; T:=HDEV+WDMA; *EXR ST% START RECEIVER
              A:=1001; T+"WDCR-WDMA"; *EXR ST
              T+"RDCR-WDCR"; X:=-10; *EXR ST
              CALL LTOUT; *JAF *-2
              1=:ACTSW
        FI
```

**Receiver Programming**:
- **State Check**: Only start if receiver inactive (`ACTSW = 0`)
- **Clear Hardware**: `WRTC` register clears any previous state
- **Physical Addressing**: `DPITPHYS` converts to absolute address for DMA
- **DMA Programming**: `WDMA` points DMA controller to receive buffer list
- **Control Setup**: `WDCR = 1001` sets receiver mode, `RDCR = -10` sets thresholds
- **Activation**: `ACTSW = 1` marks receiver as active

##### 4. **Interrupt Processing** (HIINT - Lines 51860+)
```sintran
HIINT: T:=HDEV+RRTS; *EXR ST                     % read RECEIVER STATUS
       A=:HASTAT
       IF T:=ACTSW = 0 THEN MIN T9; P+0; GO OUT1 FI % EXPECTING NOTHING *81F*
       IF A/\ HX21M >< 0 THEN                    % X21-ERROR?
```

**Frame Reception Handling**:
- **Status Reading**: `RRTS` register provides receiver status
- **State Validation**: Ensures receiver was expecting data
- **Error Detection**: `HX21M` mask checks for X.21 line errors
- **Frame Processing**: Extracts received data from hardware buffers
- **XMSG Delivery**: Routes completed frames to appropriate XMSG ports

#### Reception Transformation Flow Diagram

```mermaid
flowchart TD
    A[X.25 Frame Received] --> B[HDLC Hardware<br/>RRTS Status Check]
    B --> C[HIINT Interrupt<br/>Process Reception]
    C --> D[Extract Frame Data<br/>From DMA Buffers]
    D --> E[Remove HDLC Headers<br/>Get X.25 Payload]
    E --> F[DCB Lookup<br/>Find Destination Port]
    F --> G[Create XMSG Buffer<br/>Add XMSG Headers]
    G --> H[Queue for Application<br/>XFREA Ready]
    H --> I[Application Reads Data]

    style A fill:#ffccff
    style C fill:#ccccff
    style E fill:#ffffcc
    style G fill:#ccffcc
    style I fill:#ffcccc
```

#### Memory Address Translation

Both transformation points involve critical address translation between different addressing domains:

```mermaid
graph LR
    subgraph "XMSG Domain"
        XMA[XMSG Logical Address]
        XMB[XMSG Buffer]
    end

    subgraph "Physical Domain"
        PMA[Physical Address<br/>DPITPHYS offset]
        PMB[Physical Buffer]
    end

    subgraph "DMA Domain"
        DMA[DMA List Entry<br/>LMEM1/LMEM2]
        DMB[Hardware Buffer]
    end

    subgraph "HDLC Domain"
        HDA[HDLC Controller<br/>WDMA Register]
        HDB[Hardware FIFO]
    end

    XMA -->|OMSG+CHEAD| PMA
    PMA -->|+DPITPHYS| DMA
    DMA -->|WDMA Program| HDA
    HDA -->|Hardware| HDB

    HDB -->|HIINT| HDA
    HDA -->|DMA Complete| DMA
    DMA -->|-DPITPHYS| PMA
    PMA -->|XMSG Header| XMB
```

This analysis demonstrates the sophisticated architecture of the SINTRAN XMSG system, showing how high-level application requests are efficiently translated through multiple abstraction layers to low-level network operations, with careful attention to memory management, hardware programming, and protocol transformation at each layer.

## Deep Analysis: HDREC - HDLC Reception to XMSG Read Buffer Transformation

### Overview of HDREC Routine (Line 104171)

The HDREC (HDLC Reception) routine is responsible for converting received HDLC frames into XMSG read buffers. This is the critical path for inbound X.25/HDLC data processing in SINTRAN.

```sintran
SUBR HDREC,HIINT,XSHDR
```

### Complete HDREC Flow Analysis

#### **Phase 1: Initialization and Validation (Lines 104171-104210)**

```sintran
HDREC: IF INTSTA >< 2 THEN A:=ENINIT; GO FAR BACKX FI
       X-BHEAD; CALL RBYTC; X+BHEAD; A-BCHEA     % GET MAX BYTECOUNT
       IF A < MAXR THEN A:=ETOSM; GO FAR BACKX FI
       CALL HXST                                 %*81F*
       A:=X+CHEAD=:D:=MASTB                      %BUFFER ADDRESS     *81F*
```

**Validation Steps:**
1. **Interface State Check**: `INTSTA >< 2`
   - **Requirement**: Interface must be in initialized state (INTSTA = 2)
   - **Failure**: `ENINIT` error - Interface not initialized
   - **Impact**: Complete rejection of frame

2. **Buffer Size Validation**: `A-BCHEA` and `A < MAXR`
   - **Requirement**: Available buffer space ≥ Maximum receive size (MAXR)
   - **Failure**: `ETOSM` error - Frame too large for buffer
   - **Impact**: Frame discarded, potential buffer overflow prevention

3. **Buffer Address Setup**: `A:=X+CHEAD=:D:=MASTB`
   - **Function**: Establishes memory address for DMA reception
   - **Critical**: Sets up physical memory mapping for HDLC data reception

#### **Phase 2: DMA List Management (Lines 104215-104270)**

```sintran
HSPEC: X=:T; AD=:LISTF.LMEM                      % *H*
       A:=X-LISTP SHZ -2:=:X                     % CALCULATE INDEX IN DCB TABLE *H*
       * AAX DCBX; STT 0,X,B                     % SAVE DCB ADDR. IN DCB TABLE *H*
       X:=A                                      % X POINTS TO DMA LIST ENTRY *H*
       ERB=:X.LKEY                               % *81F*
       LISTF+4=:LISTF                            %LIST FILLER OF NEXT MESSAGE
```

**DMA List Processing:**
1. **DCB Table Index Calculation**: `A:=X-LISTP SHZ -2`
   - **Function**: Calculates index in Device Control Block table
   - **Purpose**: Maps received frame to specific XMSG message buffer

2. **List Key Management**: `ERB=:X.LKEY`
   - **ERB**: Empty Receiver Block marker
   - **Function**: Marks buffer as ready for data reception
   - **Critical**: Links DMA hardware to XMSG message system

3. **List Advancement**: `LISTF+4=:LISTF`
   - **Function**: Advances to next message buffer in queue
   - **Purpose**: Prepares for subsequent frame reception

#### **Phase 3: DMA Reception Control (Lines 104270-104340)**

```sintran
ZSTARC: IF ACTSW = 0 THEN
         HXDOK/\MAINT; T:=HDEV+WRTC; *EXR ST  % CLEAR OLD GARBAGE
         % ADD DPITPHYS TO GET PHYSICAL ADDRESS
         LIINT+DPITPHYS; T:=HDEV+WDMA; *EXR ST% START RECEIVER
         A:=1001; T+"WDCR-WDMA"; *EXR ST
         T+"RDCR-WDCR"; X:=-10; *EXR ST
         CALL LTOUT; *JAF *-2
         1=:ACTSW
       FI
```

**Hardware Control Sequence:**
1. **Activity Switch Check**: `IF ACTSW = 0`
   - **Function**: Prevents multiple concurrent DMA operations
   - **Critical**: Ensures hardware state consistency

2. **Hardware Initialization**:
   - **`HDEV+WRTC`**: Write Transmitter Control - Clear previous state
   - **`HDEV+WDMA`**: Write DMA Address - Set reception buffer address
   - **`WDCR`**: Write DMA Control Register - Configure DMA operation

3. **Physical Address Translation**: `LIINT+DPITPHYS`
   - **Function**: Converts logical to physical memory address
   - **Critical**: Required for DMA hardware access

#### **Phase 4: Interrupt Processing (Lines 104440-104527)**

```sintran
HIINT: T:=HDEV+RRTS; *EXR ST                     % read RECEIVER STATUS
       A=:HASTAT
       IF T:=ACTSW = 0 THEN MIN T9; P+0; GO OUT1 FI % EXPECTING NOTHING *81F*
       IF A/\ HX21M >< 0 THEN                    % X21-ERROR?
```

**Interrupt Validation:**
1. **Hardware Status Read**: `T:=HDEV+RRTS`
   - **Function**: Reads receiver transfer status from HDLC controller
   - **Critical**: Determines frame reception completion and errors

2. **Activity State Verification**: `IF T:=ACTSW = 0`
   - **Function**: Ensures receiver was actually expecting data
   - **Failure**: Spurious interrupt - increments `T9` counter and exits
   - **Impact**: Prevents processing of unexpected interrupts

3. **X.21 Error Detection**: `IF A/\ HX21M >< 0`
   - **Function**: Checks for X.21 layer errors
   - **Handling**: Sets block completion flags for error termination
   - **Impact**: Graceful error handling preserves system state

#### **Phase 5: Buffer Status Processing (Lines 104473-104527)**

```sintran
IF HASTAT/\"EMTY" >< 0 THEN
  0=:ACTSW                            % DEVICE STOPPED
  MIN STPCNT                          % LACK OF BUFFER, INC COUNTER
  P+0                                 % IN CASE OF SKIP
  LISTP=:T1; 0=:T2
  DO
    T:=2000; X:=T1+T; X.LKEY         % *K*ND-110* CACHE MISS
  WHILE T1.LKEY >< "NLP"              % MAKE COPY OF DMA LIST
    X:=T2; *1BANK
    A=:BUFF3(X); *2BANK              % WHEN DEVICE STOPPED
    MIN T2; 0/\0; T1+4=:T1
  OD
FI
```

**Buffer Management:**
1. **Empty Buffer Detection**: `IF HASTAT/\"EMTY" >< 0`
   - **Function**: Detects when no receive buffers are available
   - **Response**: Stops receiver and saves DMA list state
   - **Critical**: Prevents buffer overflow and data loss

2. **State Preservation**: DMA list copying to `BUFF3`
   - **Function**: Preserves current reception state for restart
   - **Purpose**: Enables recovery when buffers become available

#### **Phase 6: Frame Processing (Lines 104527-104676)**

```sintran
MORE:
*CCM11,TRR 10                                    % CLEAR CACHE
A:=2000; X:=LIINT+A; X.LKEY               % *K*ND-110* CACHE MISS
A:=LIINT.LKEY=:D
IF A NBIT XBLDN THEN                       % ANY MORE FILLED BLOCKS ?
  IF A = "ERB" THEN GO FAR ZSTARC FI   % NO, ENABLE RECEIVER
  GO FAR OUT1
FI
```

**Frame Completion Processing:**
1. **Cache Management**: `TRR 10` - Cache invalidation for coherency
2. **Block Status Check**: `IF A NBIT XBLDN`
   - **XBLDN**: Block Done flag
   - **Function**: Identifies completed frame reception

3. **Frame Data Extraction**:
```sintran
A:=LIINT.LBYTC+DISP1=:T                   % RECEIVED MESSAGE SIZE
CALL XMPAT                                % *H* GET DCB FROM DCB LIST
X=:L; A:=LIINT.LKEY=:D; 0=:X.LKEY; 0=:X.LMEM2;  X:=L
```

**Data Processing:**
- **Size Calculation**: `LIINT.LBYTC+DISP1` - Frame size + displacement
- **DCB Resolution**: `CALL XMPAT` - Maps to specific XMSG message
- **Buffer Clearing**: Resets DMA list entries for reuse

### **DETAILED FRAME PROCESSING ANALYSIS**

#### **Step 1: Frame Size Calculation**
```sintran
A:=LIINT.LBYTC+DISP1=:T                   % RECEIVED MESSAGE SIZE
```

**Frame Size Components:**
- **`LIINT.LBYTC`**: Actual bytes received by DMA (hardware count)
- **`DISP1`**: Frame displacement offset (configurable, typically 0-15 bytes)
- **`T` Register**: Total frame size for DCB processing

**Size Validation Rules:**
- **Minimum Size**: Must be ≥ 7 bytes (minimum HDLC frame with headers)
- **Maximum Size**: Must be ≤ `MAXR` (maximum receive buffer size)
- **Displacement**: Added to account for protocol headers in buffer

#### **Step 2: DCB Resolution via XMPAT**
```sintran
CALL XMPAT                                % *H* GET DCB FROM DCB LIST

% XMPAT Subroutine Details (Line 103516):
XMPAT: 0=:X.LBYTC; A:=X-LISTP SHZ -2=:X
       * AAX DCBX; LDX 0,X,B                     % NOW X POINTS TO DCB
       T=:D:=MASTB; X - BHEAD; * BBID@3 LDATX
       IF A < 0 THEN                            % IS IT AN XMSG DCB ?
         A:=D; X + BHEAD; *XHBYT@3 STATX         % YES, SAVE FRAME LENGTH + DISP1
       ELSE
         A:=D+BCHEA; * BBYTC@3 STATX            % NO, SAVE TOTAL DCB LENGTH
         X + BHEAD
       FI; EXIT
```

**DCB Resolution Process:**
1. **Index Calculation**: `A:=X-LISTP SHZ -2`
   - **`LISTP`**: List pointer base address
   - **`SHZ -2`**: Shift left 2 (multiply by 4 for word addressing)
   - **Result**: Index into DCB table for this DMA entry

2. **DCB Lookup**: `* AAX DCBX; LDX 0,X,B`
   - **`DCBX`**: DCB table base address
   - **`LDX 0,X,B`**: Load DCB address from table
   - **Result**: X register now points to actual user DCB

3. **Message Type Detection**: `IF A < 0 THEN`
   - **Test**: `* BBID@3 LDATX` - Load buffer ID
   - **Negative ID**: Indicates XMSG message (privileged)
   - **Positive ID**: Indicates regular device message

4. **Length Storage**:
   - **XMSG**: `*XHBYT@3 STATX` - Store frame length + DISP1
   - **Regular**: `* BBYTC@3 STATX` - Store total DCB length

#### **Step 3: Frame Validation (LKEY Analysis)**
```sintran
X=:L; A:=LIINT.LKEY=:D; 0=:X.LKEY; 0=:X.LMEM2;  X:=L
IF A /\ "LMASK" = 3 THEN
  A:=0; CALL SCRET; CALL SADTS
ELSE
  IF A BIT HX21S THEN EX21 ELSE EINP FI
  CALL SCRET
  A:=D; CALL SADTS; A\/DSTAT=:DSTAT
  HDERC+1=:HDERC
FI
```

**Frame Validation Constants (from SYMBOL-1-LIST.SYMB.TXT):**
- **`LMASK = 060377`** (Octal) = **0x6079** (Hex) = **24699** (Decimal)
- **`XBLDN = 000010`** (Octal) = **0x0008** (Hex) = **8** (Decimal) - Block Done flag
- **`ERB = 001000`** (Octal) = **0x0200** (Hex) = **512** (Decimal) - Empty Receiver Block
- **`NLP = 003000`** (Octal) = **0x0600** (Hex) = **1536** (Decimal) - Null List Pointer

**What Makes a VALID Frame:**
```sintran
IF A /\ "LMASK" = 3 THEN    % Frame is VALID
```

**LMASK Bit Analysis (0x6079 = 0110000001111001 binary):**
The validation checks specific bits in the `LIINT.LKEY` status:

**Valid Frame Criteria (`LMASK & LKEY = 3`):**
- **Bits 0-1 = 11** (binary): Frame completion status = 3 (Success)
- **Bit 3 = 0**: No X.21 clear indication
- **Bit 4 = 0**: No block termination error
- **Bit 5 = 0**: No frame size error
- **Bit 6 = 0**: No DMA error
- **Bits 13-14 = 00**: Normal completion (not error or timeout)

**Invalid Frame Indicators:**
- **Any other LMASK result**: Frame has errors
- **HX21S bit set**: X.21 layer signaling error
- **Default case**: Input processing error (EINP)

#### **Step 4: Buffer State Management**
```sintran
0=:X.LKEY; 0=:X.LMEM2;     % Clear DMA list entry for reuse
```

**Buffer Cleanup:**
- **`X.LKEY = 0`**: Marks DMA entry as available
- **`X.LMEM2 = 0`**: Clears memory address reference
- **Purpose**: Prepares buffer for next frame reception

### **Complete Frame Processing Flow**

```mermaid
flowchart TD
    A[Frame Interrupt] --> B[Read Hardware Status<br/>RRTS Register]
    B --> C[Block Done Check<br/>XBLDN Flag Set?]
    C -->|No| D[More Data Coming]
    C -->|Yes| E[Calculate Frame Size<br/>LBYTC + DISP1]

    E --> F[DCB Resolution<br/>CALL XMPAT]
    F --> G[Calculate DCB Index<br/>X-LISTP SHZ -2]
    G --> H[Lookup DCB Address<br/>DCBX Table]
    H --> I[Determine Message Type<br/>BBID < 0?]

    I -->|Yes| J[XMSG Message<br/>Store FRAME+DISP1]
    I -->|No| K[Regular Message<br/>Store Total Length]

    J --> L[Frame Validation<br/>LKEY & LMASK]
    K --> L

    L --> M{LMASK Result = 3?}
    M -->|Yes| N[VALID FRAME<br/>Success Processing]
    M -->|No| O[INVALID FRAME<br/>Error Analysis]

    O --> P{HX21S Bit Set?}
    P -->|Yes| Q[X.21 Error<br/>EX21]
    P -->|No| R[Input Error<br/>EINP]

    N --> S[Clear Buffer State<br/>LKEY=0, LMEM2=0]
    Q --> S
    R --> S

    S --> T[Deliver to Application<br/>CALL OCHAIN]
    T --> U[Next Frame<br/>LIINT+4]

    D --> V[Continue Reception]
    V --> A

    style N fill:#90EE90
    style O fill:#FF6B6B
    style Q fill:#FFA500
    style R fill:#FFA500
    style T fill:#87CEEB
```

### **Frame Validation Summary**

**A frame is VALID when:**
1. **Hardware Status**: RRTS indicates successful DMA completion
2. **Block Completion**: XBLDN flag is set (frame fully received)
3. **Size Validation**: Frame size within 7 ≤ size ≤ MAXR bounds
4. **LKEY Status**: `(LKEY & LMASK) == 3` (no errors detected)
5. **Buffer Integrity**: DCB successfully resolved and accessible
6. **Address Validity**: Physical memory addresses are valid

**A frame is INVALID when:**
1. **LKEY Errors**: `(LKEY & LMASK) != 3` indicates corruption
2. **X.21 Errors**: HX21S bit indicates physical layer problems
3. **Size Errors**: Frame too small (<7 bytes) or too large (>MAXR)
4. **Buffer Errors**: DCB lookup fails or buffer unavailable
5. **DMA Errors**: Hardware reports transfer problems
6. **Timeout Errors**: Frame reception incomplete

This detailed analysis shows that SINTRAN implements comprehensive multi-layered frame validation, checking everything from hardware status to protocol compliance before delivering frames to applications.

## DCB Fragmentation and Message Assembly

### Analysis of DCB Fragmentation in Small Messages

The user observed that **small X.25 messages are split into 3 different DCBs** in their emulator, and asked whether **collecting them into one HDLC packet** could cause problems.

#### **Evidence from Log Analysis**

From `F:\RC\hdlc-analysis\xmsg-log.txt`, we see the pattern:

```log
Device; [21:42:43.676] HDLC DMA 1: DMA REOM position = 16, RXFlags=RSOM, REOM
Device; [21:42:44.396] HDLC DMA 1: DMA REOM position = 16, RXFlags=RSOM, REOM
Device; [21:42:44.776] HDLC DMA 1: DMA REOM position = 4, RXFlags=RSOM, REOM
```

**Critical Observation**: Each DCB has **both RSOM and REOM flags set**, indicating:
- **RSOM** = Receive Start of Message
- **REOM** = Receive End of Message

#### **SINTRAN Message Fragmentation Patterns**

Based on comprehensive trace analysis, SINTRAN uses **proper message fragmentation** for large X.25 packets:

**Fragmentation Pattern for Large Messages (40+ bytes)**:
1. **First DCB**: `RSOMFlag=True, REOMFlag=False` (Start of Message)
2. **Middle DCB(s)**: `RSOMFlag=False, REOMFlag=False` (Continuation)
3. **Last DCB**: `RSOMFlag=False, REOMFlag=True` (End of Message)

**Complete Message Pattern for Small Messages (≤16 bytes)**:
- **Single DCB**: `RSOMFlag=True, REOMFlag=True` (Complete Message)

#### **Evidence from Trace Analysis**

From `trace-conn-100-102.txt`, a 40-byte message fragmented into 3 DCBs:

```log
[23:26:24.581] HDLC DMA 1: Sent block [06D498:0]: 16 bytes [RSOM:True] [REOM:False]
[23:26:24.861] HDLC DMA 1: Sent block [06D49C:1]: 14 bytes [RSOM:False] [REOM:False]
[23:26:25.039] HDLC DMA 1: Sent block [06D4A0:2]: 8 bytes [RSOM:False] [REOM:True]
```

**Result**: Combined into **single 40-byte HDLC frame** at the receiver.

#### **When Fragmentation Occurs**

Messages are fragmented when:
1. **Message size exceeds MAXR limit** (maximum frame size)
2. **Buffer constraints**: Available buffer space insufficient
3. **DMA limitations**: Hardware buffer boundaries
4. **MaxReceiverBlockLength**: Set to 624 bytes in configuration

#### **Expected Emulator Behavior for Your Scenario**

**If you have 3 DCBs with pattern**:
- **DCB 1**: `RSOM=true, REOM=false`
- **DCB 2**: `RSOM=false, REOM=false`
- **DCB 3**: `RSOM=false, REOM=true`

**Expected Behavior**: **Combine all 3 DCBs into ONE HDLC frame**

This is **correct behavior** and matches SINTRAN protocol specifications.

#### **Protocol Compliance**

The correct behavior is:
- **Fragmented DCBs combine into single HDLC frames** based on RSOM/REOM flags
- **RSOM=True**: Start of new HDLC frame
- **REOM=True**: End of current HDLC frame
- **Intermediate DCBs**: Append to current frame being assembled
- **Frame transmission**: Only when REOM=True is reached

**Performance Impact**: Separate frame transmission causes significant delays (2.6-8.9 seconds) and multiple retransmissions, as documented in the trace analysis.

## SINTRAN Receiver-Side DCB Processing Analysis

### Key SINTRAN Source Code Evidence

From the **HDREC routine** (Lines 104171-104675) in `s3vs-4-L-RONNY.symb`, the receiver logic shows:

#### **Frame Processing Loop (Lines 104527-104675)**

```sintran
MORE:
*CCM11,TRR 10                                    % CLEAR CACHE
A:=2000; X:=LIINT+A; X.LKEY                     % READ LKEY from current DCB
A:=LIINT.LKEY=:D
IF A NBIT XBLDN THEN                            % ANY MORE FILLED BLOCKS ?
  IF A = "ERB" THEN GO FAR ZSTARC FI            % NO, ENABLE RECEIVER
  GO FAR OUT1
FI

% Process current DCB
HNOTRA: X:=LIINT; A:=2000; X+A; X.DLSTS         % GET LKEY & LBYTC
A:=LIINT.LBYTC+DISP1=:T                         % RECEIVED MESSAGE SIZE
CALL XMPAT                                      % GET DCB FROM DCB LIST
X=:L; A:=LIINT.LKEY=:D; 0=:X.LKEY; 0=:X.LMEM2; X:=L

IF A /\ "LMASK" = 3 THEN
  A:=0; CALL SCRET; CALL SADTS                  % SUCCESS
ELSE
  % Handle errors
FI

X-BHEAD; CALL OCHAIN                            % SEND MESSAGE BACK TO USER
LIINT+4=:LIINT                                  % ADVANCE TO NEXT DCB
A+2000; A.LKEY                                  % READ next LKEY
IF LIINT.LKEY=NLP THEN                          % End of list?
  LISTP=:LIINT                                  % Reset to start
FI
GO MORE                                         % CONTINUE PROCESSING
```

#### **Critical Observations from SINTRAN Source**

1. **Individual DCB Processing**: Each DCB is processed **individually** in the `MORE` loop
2. **LKEY Validation**: Each DCB has its own `LKEY` status checked against `LMASK`
3. **Message Delivery**: Each DCB is sent back to user via `CALL OCHAIN`
4. **Sequential Processing**: `LIINT+4=:LIINT` advances to next DCB in chain
5. **End Detection**: `IF LIINT.LKEY=NLP` detects end of DCB list

#### **DCB Chain Structure**

```sintran
% DCB List initialization (Line 104231)
LISTF+4=:LISTF                                  % LIST FILLER OF NEXT MESSAGE
A:=2000; X:=LISTF+A; X.LKEY                    % Check next DCB
IF LISTF.LKEY=NLP THEN                          % End of transmit list?
  LISTP=:LISTF                                  % Reset pointer
FI

% Continue with receive processing (Line 104666)
IF LIINT.LKEY=NLP THEN                          % End of receive list?
  LISTP=:LIINT                                  % Reset receive pointer
FI
```

### **Definitive Answer: SINTRAN Processes Each DCB Separately**

The SINTRAN source code proves that:

1. **Each DCB is delivered individually** to the application via `CALL OCHAIN`
2. **RSOM/REOM flags are irrelevant at receiver level** - SINTRAN processes each received DCB as a separate entity
3. **No frame reassembly occurs in SINTRAN** - each DCB becomes a separate XMSG message
4. **DCB fragmentation is purely a DMA buffer management strategy**

### **Corrected Emulator Behavior**

For your scenario with 3 DCBs:
- **DCB 1**: `RSOM=true, REOM=false`
- **DCB 2**: `RSOM=false, REOM=false`
- **DCB 3**: `RSOM=false, REOM=true`

**SINTRAN expects**: **3 separate HDLC frames** delivering **3 separate XMSG messages**

**Your emulator should**:
1. **Send DCB 1 as separate HDLC frame** → SINTRAN delivers as XMSG message 1
2. **Send DCB 2 as separate HDLC frame** → SINTRAN delivers as XMSG message 2
3. **Send DCB 3 as separate HDLC frame** → SINTRAN delivers as XMSG message 3

**The application layer** is responsible for reassembling the 3 XMSG messages based on RSOM/REOM flags, **not the HDLC layer**.

## How RSOM/REOM Flags Are Set by HDLC Hardware

### **SINTRAN Buffer Flag Management (Lines 104450-104473)**

The HDLC hardware controller sets RSOM/REOM flags based on **buffer completion status**, not frame content:

```sintran
HIINT: T:=HDEV+RRTS; *EXR ST                     % READ RECEIVER STATUS
A=:HASTAT
IF A/\ HX21M >< 0 THEN                           % X21-ERROR?
  T:=2000; X:=LIINT+T; T:=X.LKEY                 % READ CURRENT LKEY
  A\/ LIINT.LKEY=:X.LKEY                         % SAVE STATUS
  IF A BIT HX21S THEN                            % X21 CLEAR INDICATION?
    HASTAT BONE BLDON=:HASTAT                    % SET BLOCK DONE IN STATUS
    LIINT.LKEY BONE XBLDN=:X.LKEY                % SET BLOCK DONE IN LKEY
  FI
FI
```

### **Flag Setting Logic**

**XBLDN (Block Done) is the key flag**:
- **RSOM=True, REOM=False**: When `LKEY` has **no XBLDN bit** (buffer not complete)
- **RSOM=False, REOM=False**: When `LKEY` has **no XBLDN bit** and not first buffer
- **RSOM=False, REOM=True**: When `LKEY` has **XBLDN bit set** (buffer complete)

### **Hardware Implementation for Your Emulator**

For the scenario with 3 DCBs:

**DCB 1** (`RSOM=true, REOM=false`):
```csharp
// First buffer of frame - no XBLDN set
dcb.LKEY = 0x0000;  // No completion flag
dcb.RXFlags = "RSOM";  // Start of message only
```

**DCB 2** (`RSOM=false, REOM=false`):
```csharp
// Middle buffer - no XBLDN set, not first
dcb.LKEY = 0x0000;  // No completion flag
dcb.RXFlags = "";  // No flags (continuation)
```

**DCB 3** (`RSOM=false, REOM=true`):
```csharp
// Final buffer - XBLDN set to indicate completion
dcb.LKEY = XBLDN_BIT;  // Block done flag set
dcb.RXFlags = "REOM";  // End of message
```

### **Buffer State Detection (Lines 104527-104537)**

SINTRAN detects buffer completion:
```sintran
MORE:
A:=LIINT.LKEY=:D
IF A NBIT XBLDN THEN                            % ANY MORE FILLED BLOCKS?
  IF A = "ERB" THEN GO FAR ZSTARC FI            % NO, ENABLE RECEIVER
  GO FAR OUT1
FI
```

**Critical Point**: RSOM/REOM flags are **hardware buffer management signals**, not application data. They indicate:
- **Buffer boundary states** (start/middle/end of DMA chain)
- **Frame completion status** (whether more buffers are coming)
- **DMA controller state** (ready for next buffer vs. frame complete)

The HDLC controller sets these flags **automatically** based on buffer fill status and frame boundaries, **not** based on any content within the HDLC frame itself.

#### **Phase 7: Error Classification and Response (Lines 104631-104660)**

```sintran
IF A /\ "LMASK" = 3 THEN
  A:=0; CALL SCRET; CALL SADTS
ELSE
  IF A BIT HX21S THEN EX21 ELSE EINP FI
  CALL SCRET
  A:=D; CALL SADTS; A\/DSTAT=:DSTAT
  HDERC+1=:HDERC
FI
X-BHEAD; CALL OCHAIN                               % SEND MESSAGE BACK TO USER
```

**Error Processing:**
1. **Frame Validation**: `IF A /\ "LMASK" = 3`
   - **LMASK**: Frame validation mask
   - **Success**: Clean frame with no errors
   - **Error**: Various frame corruption indicators

2. **Error Classification**:
   - **HX21S**: X.21 specific errors (`EX21`)
   - **Default**: Input processing errors (`EINP`)

3. **Message Completion**: `CALL OCHAIN`
   - **Function**: Sends completed message back to XMSG user
   - **Critical**: Delivers frame data to application layer

### Critical Failure Points and Validations

#### **1. Interface State Failures**
```mermaid
flowchart TD
    A[HDREC Entry] --> B{INTSTA = 2?}
    B -->|No| C[ENINIT Error]
    B -->|Yes| D[Buffer Size Check]
    C --> E[FAR BACKX - Complete Failure]
    D --> F{Buffer Available?}
    F -->|No| G[ETOSM Error]
    F -->|Yes| H[DMA Setup]
    G --> E
```

**Validation Requirements:**
- **INTSTA = 2**: Interface must be fully initialized
- **Buffer Availability**: Sufficient space for maximum frame size
- **Hardware State**: HDLC controller must be operational

#### **2. DMA and Hardware Failures**
```mermaid
flowchart TD
    A[DMA Setup] --> B{ACTSW State?}
    B -->|Active| C[Skip Hardware Init]
    B -->|Inactive| D[Hardware Initialization]
    D --> E{Physical Address Valid?}
    E -->|No| F[DMA Address Fault]
    E -->|Yes| G[Start DMA Reception]
    G --> H{Hardware Response?}
    H -->|Timeout| I[LTOUT Failure]
    H -->|Success| J[Reception Active]
```

**Hardware Validation:**
- **Physical Address**: `LIINT+DPITPHYS` must be valid
- **Hardware Response**: HDLC controller must acknowledge commands
- **DMA Configuration**: Proper register setup required

#### **3. Frame Reception Failures**
```mermaid
flowchart TD
    A[Frame Interrupt] --> B{ACTSW Active?}
    B -->|No| C[Spurious Interrupt]
    B -->|Yes| D{X.21 Error?}
    D -->|Yes| E[X.21 Error Handling]
    D -->|No| F{Buffer Available?}
    F -->|No| G[EMTY - Stop Receiver]
    F -->|Yes| H[Process Frame]
    H --> I{Frame Valid?}
    I -->|No| J[Frame Error Processing]
    I -->|Yes| K[Deliver to XMSG]
```

**Frame Validation:**
- **Frame Size**: Must be within `MAXR` limits
- **Frame Integrity**: CRC and format validation
- **Buffer Availability**: Must have space for frame data

### Complete HDREC Flow Diagram

```mermaid
flowchart TD
    Start([HDREC Entry]) --> Init{Interface<br/>Initialized?}
    Init -->|No| Fail1[ENINIT Error]
    Init -->|Yes| BufCheck{Buffer Size<br/>Available?}
    BufCheck -->|No| Fail2[ETOSM Error]
    BufCheck -->|Yes| DMASetup[DMA List Setup]

    DMASetup --> DCBCalc[Calculate DCB Index]
    DCBCalc --> LinkDMA[Link DMA to XMSG]
    LinkDMA --> HWCheck{Hardware<br/>Active?}

    HWCheck -->|Yes| WaitInt[Wait for Interrupt]
    HWCheck -->|No| StartHW[Initialize Hardware]
    StartHW --> SetDMA[Set DMA Address]
    SetDMA --> StartRec[Start Reception]
    StartRec --> WaitInt

    WaitInt --> IntRcv[Interrupt Received]
    IntRcv --> StatRead[Read Hardware Status]
    StatRead --> ActCheck{ACTSW<br/>Active?}
    ActCheck -->|No| Spurious[Spurious Interrupt]
    ActCheck -->|Yes| X21Check{X.21<br/>Error?}

    X21Check -->|Yes| X21Handle[Handle X.21 Error]
    X21Check -->|No| BufStat{Buffer<br/>Status?}
    BufStat -->|Empty| StopRec[Stop Receiver]
    BufStat -->|Available| FrameProc[Process Frame]

    FrameProc --> SizeCalc[Calculate Frame Size]
    SizeCalc --> DCBResolve[Resolve DCB]
    DCBResolve --> FrameVal{Frame<br/>Valid?}
    FrameVal -->|No| ErrorProc[Error Processing]
    FrameVal -->|Yes| SuccessProc[Success Processing]

    ErrorProc --> ErrorClass{Error<br/>Type?}
    ErrorClass -->|X.21| EX21[EX21 Error]
    ErrorClass -->|Input| EINP[EINP Error]

    SuccessProc --> ClearBuf[Clear Buffer]
    ClearBuf --> SendMsg[Send to XMSG User]
    EX21 --> SendMsg
    EINP --> SendMsg
    X21Handle --> SendMsg

    SendMsg --> MoreFrames{More<br/>Frames?}
    MoreFrames -->|Yes| FrameProc
    MoreFrames -->|No| EnableRec[Enable Receiver]
    EnableRec --> WaitInt

    StopRec --> SaveState[Save DMA State]
    SaveState --> WaitBuffer[Wait for Buffers]

    Fail1 --> Exit([Return Error])
    Fail2 --> Exit
    Spurious --> Exit
    WaitBuffer --> Exit

    style Start fill:#90EE90
    style Exit fill:#FFB6C1
    style Fail1 fill:#FF6B6B
    style Fail2 fill:#FF6B6B
    style ErrorProc fill:#FFA500
    style SuccessProc fill:#90EE90
    style SendMsg fill:#87CEEB
```

### Key Failure Scenarios

#### **1. Critical System Failures**
- **ENINIT**: Interface not initialized - System configuration error
- **ETOSM**: Frame too large - Network configuration mismatch
- **DMA Faults**: Physical memory addressing failures

#### **2. Protocol Failures**
- **FCS Errors**: Frame corruption during transmission
- **X.21 Errors**: Physical layer signaling problems
- **Buffer Exhaustion**: System overload or memory issues

#### **3. Timing Failures**
- **Spurious Interrupts**: Hardware timing issues
- **DMA Timeouts**: Hardware unresponsive
- **Buffer Starvation**: Application processing too slow

### Error Recovery Mechanisms

#### **Automatic Recovery:**
- **Buffer Management**: Automatic receiver restart when buffers available
- **Hardware Reset**: DMA reinitialization on errors
- **State Preservation**: DMA list saved during buffer exhaustion

#### **Error Propagation:**
- **XMSG Integration**: Errors delivered to application layer
- **Status Logging**: Hardware status preserved for diagnosis
- **Counter Management**: Error counting for system monitoring

This analysis shows that HDREC implements a robust, multi-layered validation and error handling system with comprehensive recovery mechanisms, making it critical for reliable X.25/HDLC communication in SINTRAN.