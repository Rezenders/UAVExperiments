import socket
import os
import sys
import time
import select
import re
import math
from collections import deque

ALICEPORT = 4445
BOBPORT = 4446
MANAGERPORT = 4447

PORTS = [ALICEPORT, BOBPORT, MANAGERPORT]

buffers = {}
mailboxes = {}

def createSocket(TCP_PORT):
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    #serverSocket.setblocking(0)
    if TCP_PORT == ALICEPORT:
        serverSocket.bind(('192.168.7.1', TCP_PORT))
    else:
        serverSocket.bind(('localhost', TCP_PORT))
    serverSocket.listen(5)

    return serverSocket

def sendTo(s, msg): #sendTo(port socket to send, list of sockets, message)
    s.sendall(msg)

def readlines(sock, extBuff, recv_buffer=16384, delim='\n'):
    data = True
    buff = extBuff
    line = ''
    while data:
        data = sock.recv(recv_buffer)
        buff += data
        while buff.find(delim) != -1:
            line, buff = buff.split('\n', 1)
            return line+'\n', buff
    return line, buff

sockList = []
#Connect to all sub-systems (sockets) before anything
for p in PORTS:
    sock = createSocket(p)
    clientSock, address = sock.accept()
    sockList.append(clientSock)
    mailboxes.update({p : deque()})
    buffers.update({p : ''})

print("ALL CONNECTED")

while True:
    #Check which sockets are readable or writable
    readable, writable, exceptional = select.select(sockList, sockList, sockList, 0.1)

    #Interpret received messages
    for s in readable: #for every socket object
        _, receivePort = s.getsockname() #receivePort = port_number

        data, buffers[receivePort] = readlines(s,buffers[receivePort])
        if data != '': #if it's not a null message
            print(str(receivePort) + 'sent: ' + data)
            if data == 'connListRequest\n':
                print('responding connListRequest')
                rsp = 'connectedList:' + str(PORTS) + '\n'
                mailboxes[receivePort].append(rsp)
            else:
                dest, msg = re.split(';',data)
                if dest == '': #bcast
                    for box in mailboxes:
                        if box != receivePort:
                            mailboxes[box].append(msg)
                else:
                    if int(dest) in mailboxes:
                        mailboxes[int(dest)].append(msg)

    for w in writable:
        _, sendPort = w.getsockname()
        if (sendPort in mailboxes) and (len(mailboxes[sendPort])>0):
            sendTo(w,mailboxes[sendPort].popleft())

    time.sleep(0.001)
