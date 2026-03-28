
# UniComs - Unified Communication System

A Feature-Rich, Secure, Real-Time Terminal-Based Communication System Built from Scratch in C++

---

## Overview

UniComs is a complete real-time communication platform that runs entirely in the terminal. Built from scratch in C++, it demonstrates:

- Network Programming - Raw socket implementation (TCP/UDP)
- Cryptography - End-to-end encryption with Diffie-Hellman + AES-256
- System Programming - Threading, process management, file I/O
- UI/UX Design - Beautiful terminal interface with ncurses
- Real-time Systems - Voice streaming with ADPCM compression

---

## Features

### Messaging
- Direct Messages - One-on-one encrypted conversations
- Group Chats - Password-protected chat rooms
- Real-time Delivery - Instant message delivery with status indicators
- Message History - Persistent storage of all conversations
- Typing Indicators - See when others are typing

### File Transfer
- Binary File Support - Send any file type
- Base64 Encoding - Safe transmission over text protocol
- Chunked Transfer - Files split into 3KB chunks
- Progress Tracking - Real-time progress bar with speed display
- Auto-save - Files saved to received/ directory

### Voice Calls
- Real-time Audio - Low-latency voice communication
- ADPCM Compression - 4x compression with minimal quality loss
- UDP Streaming - Optimized for real-time transmission
- Wave Animation - Visual feedback during calls
- Call Controls - Accept/reject/end calls

### User Interface
- Gradient Headers - Animated color gradients
- ASCII Art Logo - Professional branding
- Message Bubbles - Color-coded messages
- Contact List - Online/offline indicators with unread badges
- Themes - Dark, Light, Neon, Matrix
- Animations - Loading spinners, typing indicators, wave effects

### Security
- End-to-End Encryption - AES-256 in CTR mode
- Perfect Forward Secrecy - Ephemeral keys via Diffie-Hellman
- Password Hashing - SHA-256 with salt
- Session Persistence - Encrypted keys stored locally
- No Plaintext Transmission - All messages encrypted before sending

---

## Architecture

### System Architecture

The system follows a client-server architecture:

- Client Layer: User interface built with ncurses, handles input and display
- Network Layer: TCP for messages/files, UDP for voice calls
- Storage Layer: CSV files for users, messages, and rooms
- Security Layer: Diffie-Hellman key exchange + AES-256 encryption

### Communication Flows

Direct Message Flow:
1. User A types message
2. Message encrypted with AES-256
3. Sent via TCP to server
4. Server routes to User B
5. User B receives and decrypts
6. Message displayed

File Transfer Flow:
1. User selects file
2. File converted to Base64
3. Split into 3KB chunks
4. FILE_OFFER sent to recipient
5. Recipient accepts
6. Chunks streamed
7. File reassembled and saved

Voice Call Flow:
1. User initiates call
2. CALL_OFFER via server
3. Recipient accepts
4. UDP ports exchanged
5. Direct UDP connection established
6. Audio captured via PortAudio
7. Compressed with ADPCM
8. Streamed over UDP
9. Decompressed and played

---

## Security

### Diffie-Hellman Key Exchange
Both users generate public keys from private keys. They exchange public keys and compute the same shared secret. No one listening can compute it.

### AES-256 Encryption
Messages are encrypted with the shared secret using AES-256 in CTR mode. Encrypted messages look like random text.

### Password Storage
Passwords are hashed with SHA-256 plus a random salt. Only the hash is stored, never the plaintext password.

---

## Installation

### Prerequisites

Linux:
sudo apt install libncurses5-dev portaudio19-dev g++ make

macOS:
brew install ncurses portaudio
xcode-select --install

### Build

git clone https://github.com/yourusername/unicoms.git
cd unicoms
make clean
make all

### Run

Terminal 1 - Server:
./server 8080

Terminal 2 - Client 1:
./client 127.0.0.1 8080

Terminal 3 - Client 2:
./client 127.0.0.1 8080

---

## Commands

### Authentication
/login <username> <password>     Login to existing account
/signup <username> <password>    Create new account
/logout                          Logout from current session

### Messaging
dm <username> <message>          Send direct message
join <room> <password>           Join a chat room
leave <room>                     Leave a chat room
msg <room> <message>             Send message to room
create <room> <password>         Create new chat room

### Information
users                            List all online users
rooms                            List all available rooms
members <room>                   List members in a room
history dm <user>                View DM history
history room <room>              View room history

### File Transfer
/send <filepath>                 Send a file
/accept                          Accept incoming file
/reject                          Reject incoming file

### Voice Calls
/call <username>                 Start voice call
/endcall                         End current call
/accept                          Accept incoming call
/reject                          Reject incoming call

### Utility
/help                            Show help
/clear                           Clear chat
/q                               Quit current chat
quit or exit                     Exit application

---

## Keyboard Shortcuts

F1           Toggle online-only filter
F2           Clear input field
F3           Show file transfers
F4           Cycle themes
Up Arrow     Scroll up
Down Arrow   Scroll down
Tab          Cycle contacts
Enter        Send message
Ctrl+C       Exit
Backspace    Delete character
Ctrl+U       Clear input line

---

## Project Structure

unicoms/
├── src/
│   ├── client/
│   │   ├── ClientController.cpp/h
│   │   ├── Terminal.cpp/h
│   │   ├── NetworkManager.cpp/h
│   │   ├── CryptoHandler.cpp/h
│   │   ├── FileHandler.cpp/h
│   │   ├── VoiceHandler.cpp/h
│   │   └── Client.cpp
│   │
│   ├── server/
│   │   ├── Server.cpp
│   │   ├── HandleMessage.cpp
│   │   ├── Login.cpp
│   │   ├── CmdFile.cpp
│   │   └── CmdCall.cpp
│   │
│   ├── db/
│   │   ├── Database.h
│   │   ├── Catalog.cpp/h
│   │   ├── TableEngine.cpp/h
│   │   ├── User.csv
│   │   ├── Message.csv
│   │   └── ChatRoom.csv
│   │
│   ├── ui/
│   │   └── TerminalUI.cpp/h
│   │
│   ├── voice/
│   │   ├── VoiceCall.cpp/h
│   │   ├── Adpcm.h
│   │   └── UdpSocket.h
│   │
│   └── utils/
│       ├── crypto/
│       │   ├── DiffieHellman.cpp/h
│       │   ├── Aes.cpp/h
│       │   └── Random.cpp/h
│       ├── Sha256.cpp/h
│       └── Utils.cpp/h
│
├── Makefile
└── README.md

---

## Team

Tabib Hassan        230042131    Client Developer
Nafis Ahnaf Jamil   230042150    Server Developer
Sieam Shahriare     230042153    Security Developer

### Contributions

Tabib Hassan (Client Developer)
- Built terminal user interface with ncurses
- Implemented raw mode input handling and password masking
- Created animations and gradient effects
- Integrated all client components
- Managed screen transitions

Nafis Ahnaf Jamil (Server Developer)
- Implemented TCP socket server from scratch
- Designed CSV-based database system
- Built message routing and broadcast mechanisms
- Implemented concurrent client handling
- Created room management system

Sieam Shahriare (Security & Voice Developer)
- Implemented Diffie-Hellman key exchange
- Built AES-256 encryption in CTR mode
- Created ADPCM audio compression
- Integrated PortAudio for voice streaming
- Developed UDP voice transmission

---

## License

MIT License

---

## Acknowledgments

- ncurses - Terminal graphics
- PortAudio - Audio handling
- Our Professor - Guidance and support

---

Built with ❤️ by Team 08

