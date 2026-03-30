# UniComs SPL1

A secure, real-time communication platform with end-to-end encryption, voice calling, and file sharing capabilities

---

## Overview

UniComs SPL1 is a terminal-based communication system built in C++17 that provides secure instant messaging, voice calls, and file transfers. The application features a client-server architecture with strong security measures including Diffie-Hellman key exchange and AES encryption for end-to-end encrypted communications.

### Key Capabilities

- **Secure Authentication**: User registration and login with SHA256 password hashing
- **End-to-End Encryption**: DH key exchange with AES encryption for private messages
- **Chat Rooms**: Create and join public or password-protected chat rooms
- **Direct Messaging**: Encrypted one-on-one conversations
- **Voice Calling**: Real-time voice calls with ADPCM compression via PortAudio
- **File Sharing**: Secure peer-to-peer file transfers
- **Message History**: Retrieve chat history from both DMs and rooms
- **Multi-Platform**: Supports macOS (Darwin) and Linux

---

## Architecture

### System Components

```text
UniComs/
├── server/          # TCP server handling client connections
├── client/          # Terminal-based client UI
├── db/             # Database layer with CSV storage
├── voice/          # Voice call handling with PortAudio
├── utils/          # Utility functions (hashing, string ops)
└── utils/crypto/   # Cryptographic implementations
```

### Communication Flow

1. **Server** (`src/server/Server.cpp`)
   - Listens on TCP port 8080 (configurable)
   - Spawns a thread per client connection
   - Manages user sessions, authentication, and message routing
   - Maintains chat rooms and message history

2. **Client** (`src/client/Client.cpp`)
   - Terminal UI with raw mode input
   - Network manager for async message handling
   - Crypto handler for E2E encryption
   - Voice handler for audio calls
   - File handler for transfers

3. **Database** (`src/db/`)
   - CSV-based storage for users, messages, and chat rooms
   - Schema catalog system (`Catalog.cpp`) for table definitions
   - Table engine for CRUD operations

---

## Security Features

### Authentication

- Username validation (alphanumeric only)
- Password hashing with SHA256 + salt
- Session management with single-session enforcement

### Encryption

- **Key Exchange**: Diffie-Hellman (2048-bit) for secure session key establishment
- **Symmetric Encryption**: AES for message encryption
- **Per-Peer Keys**: Unique encryption keys for each communication peer
- **Secure by Default**: All direct messages are encrypted

### Data Storage

- Sensitive data stored in `keys/` directory (gitignored)
- Passwords never stored in plaintext
- Encrypted message history

---

## Technology Stack

| Component | Technology |
|-----------|------------|
| Language | C++17 |
| Audio | PortAudio 2.0 |
| Build System | GNU Make |
| Threading | pthreads |
| Cryptography | Custom DH, AES, SHA256 |
| Storage | CSV files + custom DB engine |
| Network | BSD sockets (TCP + UDP) |
| OS | macOS/Linux |

---

## Dependencies

### Required Packages

**macOS (Homebrew):**

```bash
brew install portaudio
```

**Ubuntu/Debian:**

```bash
sudo apt-get install libportaudio2
```

**Fedora:**

```bash
sudo dnf install portaudio-devel
```

---

## Building

```bash
# Compile both server and client
make

# Or individually
make server
make client

# Clean build artifacts
make clean
```

### Build Output

- `./server` - Server executable
- `./client` - Client executable

---

## Running

### 1. Start the Server

```bash
# Default port 8080
./server

# Custom port
./server 8080
```

Server output:

```
[server] Listening on port 8080
[server] New connection fd=4
```

### 2. Connect Clients

```bash
./client <server-ip> <port>
```

Example:

```bash
./client 127.0.0.1 8080
./client localhost 8080
```

---

## User Guide

### Authentication

Upon first launch, you'll see:

```
==================================
              UNICOMS
==================================
[1] Login
[2] Signup
Choice:
```

**Signup:**

1. Choose option 2
2. Enter desired username (alphanumeric only)
3. Set password
4. Account created successfully

**Login:**

1. Choose option 1
2. Enter username and password
3. Access main menu upon success

### Main Menu

After login, you'll access the main menu:

```
==================================
          MAIN MENU
==================================
Logged in as: <username>
[1] dm <username>     - Start private chat
[2] join <room> [pw]  - Join a chat room
[3] create <room> [pw]- Create new room
[4] users             - List all users
[5] rooms             - List all rooms
[6] history dm <user> - View DM history
[7] history room <r>  - View room history
[8] logout            - Log out
[9] quit              - Exit client
>
```

### Chat Interface

**Direct Message (DM)**

```
/dm alice
```

Once in a DM, type messages directly. End-to-end encrypted.

**Chat Rooms**

```
/create general
```

Create and automatically join a room.

**Available Chat Commands** (within DM/room):

- `/q` - Quit current chat, return to menu
- `/help` - Show help message
- `/call` - Start voice call (requires peer in same DM)
- `/endcall` - End active voice call
- `/history` - Load chat history for current conversation
- `/send <filepath>` - Send a file to peer/room
- `/accept` - Accept incoming file or call request
- `/reject` - Reject incoming file or call request

**Note**: Voice calls and file transfers are peer-to-peer; both users must be in the same DM and accept the connection.

### Security Notes

- Messages are encrypted per peer using unique keys
- DH key exchange occurs automatically on first DM
- Encrypted session persists across conversations with same peer
- Each user has unique encryption keys stored locally in `keys/` directory

---

## Project Structure

```
UniComs_SPL1/
├── Makefile              # Build configuration
├── .gitignore           # Git ignore patterns
├── keys/                # User encryption keys (gitignored)
│   ├── personA_personB.key
│   └── personB_personC.key
└── src/
    ├── server/
    │   ├── Server.cpp/.h      # Main server entry point
    │   ├── Socket.cpp/.h      # Socket utilities
    │   ├── handleMessage.cpp  # Message dispatcher
    │   ├── Login.cpp/.h       # Authentication handlers
    │   ├── CmdFile.cpp/.h     # File transfer commands
    │   ├── CmdCall.cpp/.h     # Voice call commands
    │   ├── Broadcast.cpp/.h   # Message broadcasting
    │   ├── JoinChatRoom.cpp/.h
    │   ├── GetChatRoomList.cpp/.h
    │   └── GetClientInfo.cpp/.h
    ├── client/
    │   ├── Client.cpp         # Main client entry point
    │   ├── ClientController.cpp/.h  # UI logic & command handling
    │   ├── Terminal.cpp/.h    # Terminal UI/UX
    │   ├── NetworkManager.cpp/.h  # Async network I/O
    │   ├── CryptoHandler.cpp/.h  # Encryption/decryption
    │   ├── FileHandler.cpp/.h    # File transfer logic
    │   ├── VoiceHandler.cpp/.h   # Voice call management
    │   └── Connect.cpp/.h      # Connection establishment
    ├── db/
    │   ├── Catalog.cpp/.h      # Schema manager (DDL)
    │   ├── Indexer.cpp/.h      # Indexing engine
    │   ├── TableEngine.cpp/.h  # Table CRUD operations
    │   ├── CsvStorage.cpp/.h   # CSV file I/O
    │   └── Message.csv         # Message storage (generated)
    ├── voice/
    │   ├── VoiceCall.cpp/.h    # PortAudio capture/playback
    │   ├── Adpcm.h             # ADPCM compression
    │   └── UdpSocket.h         # UDP socket for voice stream
    └── utils/
        ├── Utils.cpp/.h        # String, memory utilities
        ├── Sha256.cpp/.h       # Password hashing
        └── crypto/
            ├── Aes.cpp/.h      # AES encryption
            ├── DiffieHellman.cpp/.h  # DH key exchange
            └── Random.cpp/.h   # RNG for crypto
```

---

## Technical Details

### Database Schema

The system uses a CSV-based relational database with the following tables:

**User**

- `id` (primary)
- `username` (unique)
- `password` (SHA256 hash)

**Message**

- `id` (primary)
- `room` (empty for DMs)
- `sender`
- `content` (encrypted for DMs)
- `timestamp`

**ChatRoom**

- `id` (primary)
- `name` (unique)
- `password` (hashed, empty for public)

**ChatRoomMember**

- `user_id` (foreign key)
- `room_id` (foreign key)

### Messaging Protocol

**Client → Server Commands:**

- `SIGNUP <username> <password>`
- `LOGIN <username> <password>`
- `CHECK_USER <username>`
- `DM <target> <encrypted_msg>`
- `MSG <room> <encrypted_msg>`
- `JOIN <room> [password]`
- `CREATE_ROOM <room> [password]`
- `LEAVE <room>`
- `LIST_USERS`
- `LIST_ROOMS`
- `HISTORY_DM <user>`
- `HISTORY_ROOM <room>`
- `CALL <target>`
- `CALL_ACCEPT <port>`
- `CALL_REJECT`
- `CALL_END`
- `FILE_SEND <target> <filename> <size>`
- `FILE_ACCEPT`
- `FILE_REJECT`

**Server → Client Responses:**

- `OK <message>`
- `ERR <message>`
- `FOUND` / `NOT_FOUND` (for username checks)
- `MSG_FROM <sender> <encrypted_msg>`
- `ROOM_MSG <room> <sender> <content>`
- `INFO <message>`
- `CALL_OFFER <caller> <ip> <port>`
- `CALL_ACCEPTED <callee_ip> <callee_port>`
- `CALL_REJECTED`
- `CALL_ENDED`
- `FILE_OFFER <sender> <filename> <size>`
- `FILE_ACCEPTED`
- `FILE_REJECTED`
- `FILE_DATA <chunk>`
- `FILE_END <saved_path>`

### Encryption Flow

1. **Initial DH Exchange**:

   ```
   Client A → Client B: DH_INIT <pubkey_A>
   Client B → Client A: DH_REPLY <pubkey_B> <encrypted_session_key>
   ```

   Both compute shared secret.

2. **Per-Message Encryption**:
   - AES-CTR mode with per-message nonce
   - Session key derived from DH shared secret
   - Unique nonce per message prevents replay

3. **Key Storage**:
   - Per-peer public/private keys stored in `keys/<username>.key`
   - Files are binary and gitignored

### Voice Call Implementation

- **Transport**: UDP (low latency)
- **Codec**: ADPCM (4-bit compression, ~4:1 ratio)
- **Sampling**: 16-bit PCM @ 8000 Hz (telephone quality)
- **Buffering**: 320-frame buffers (~40ms latency)
- **Streaming**: Dual-threaded (capture & playback)

---

## Development Notes

### Recent Changes

- **New Login/Signup Flow** - Modernized authentication UI with better UX
- **Fixed Room Issues** - Resolved password-protected room functionality
- **Reverted Call Functions** - Stabilized voice call implementation
- **Added Colorings** - Terminal color support for better readability
- **ASCII Art** - Branded title screens

### Known Limitations

- Voice calls are one-to-one only (no conference calls)
- File transfers are direct peer-to-peer (server relays metadata only)
- No message deletion or editing
- Database is file-based; no concurrent server access (single instance)
- Keys are stored locally; losing `keys/` directory breaks decryption of old messages

---

## Configuration

### Port Configuration

Change default port in:

- `src/server/Server.cpp:17` (default: 8080)

### Database Location

Database files are created in working directory:

- `Message.csv` - Message storage
- `ddl.csv` - Schema definitions (auto-generated)

### Build Flags

Edit `Makefile` to adjust:

- `CXXFLAGS` - Compiler flags (currently C++17, warnings)
- `LDFLAGS` - Linker flags (PortAudio paths)

---

## Troubleshooting

### "PortAudio not found"

Install PortAudio development package (see Dependencies).

### "Address already in use"

Change server port or wait for OS to release the port (use `lsof -i:8080` to find processes).

### Cannot connect

- Verify server is running
- Check firewall settings
- Ensure correct IP/port
- If connecting across machines, ensure server binds to 0.0.0.0 (not just 127.0.0.1)

### Voice call fails

- Both peers must be in the same DM
- UDP ports must be open (client uses random high port)
- Microphone permissions on macOS: allow terminal in System Preferences

### Decryption failures

- Keys in `keys/` directory must match the original username
- Username validation may have changed; ensure alphanumeric usernames only
- Re-login to refresh session keys

---

## Authors

Developed by team **IUT_Dhongorsho**

### Members

- Nafis Ahnaf Jamil
- Tabib Hassan
- Sieam Shahriare

---

*Built with C++17, sockets, and a passion for secure communication.*