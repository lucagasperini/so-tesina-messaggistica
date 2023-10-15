# so-tesina-messaggistica
### Introduction
Just a simple client/server messaging service
### Installation
#### Dependencies
- CMake 3.11+
- GCC 10+
- pthread
### Build
```
git clone https://github.com/lucagasperini/so-tesina-messaggistica
```
```
cd so-tesina-messaggistica
```
```
cmake -Bbuild
```
```
cd build && make
```
### How to use
#### Configurazione nuovo utente
```
./server --add-user
```
#### Avvio server
(Start listen on port 1234)
```
./server 1234
```
(Start listen on default port 3333)
```
./server
```
#### Avvio client
(Start connection on host 127.0.0.1 and port 1234)
```
./client 127.0.0.1:1234
```
(Start connection on default host 127.0.0.1 and default port 3333)
```
./client
```
