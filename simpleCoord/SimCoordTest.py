import socket
import os
import sys
import time
import select
import re
import math
from collections import deque


#SERVER
####constants####
BOBPORT = 6969
ALICEPORT = 6970
MANAGERPORT = 6971

####methods####
#def decodeSock(msg, port):
#    return msg[:-1]

def encodeSock(msg, port):
    return msg + '\n'

def createSocket(TCP_PORT):
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
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

####variable initialization####
localPorts = [BOBPORT, ALICEPORT, MANAGERPORT]
sockList = [] #sockets accepted
lastTime = time.time()
mail=''
mailBoxes={}

#Connect to all sub-systems (sockets) before anything
for p in localPorts:
    sock = createSocket(p)
    clientSock, address = sock.accept()
    sockList.append(clientSock)
    mailBoxes.update({p : deque()})


while True:
    #Check which sockets are readable or writable
    readable, writable, exceptional = select.select(sockList, sockList, sockList, 0.01)

    #Interpret received messages
    for s in readable: #for every socket object
        _, receivePort = s.getsockname(); #receivePort = port_number

        data, mail = readlines(s,mail) #s.recv(10240)
        if data != '': #if it's not a null message

            if '*' in data[0]:
                if ('alice' in data[1:data.find(',')]):

                    mailBoxes[ALICEPORT].append('*'+data[data.find(',')+1:])
                    print('got message for alice: '+ data[:data.find(',')])

                    # sendTo(ALICEPORT,writable,encodeSock('*'+data[data.find(',')+1:],ALICEPORT))
                    # print('to alice: ' + data)

                if ('bob' in data[1:data.find(',')]):
                    mailBoxes[BOBPORT].append('*'+data[data.find(',')+1:])
                    print('got message for bob: '+ data[:data.find(',')])

                    # sendTo(BOBPORT,writable,encodeSock('*'+data[data.find(',')+1:],BOBPORT))
                    # print('to bob: ' + data)

                if ('manager' in data[1:data.find(',')]):
                    mailBoxes[MANAGERPORT].append('*'+data[data.find(',')+1:])
                    print('got message for manager: '+ data[:data.find(',')])

                    # sendTo(MANAGERPORT,writable,encodeSock('*'+data[data.find(',')+1:],MANAGERPORT))
                    # print('to manager: ' + data)

                if ('' == data[1:data.find(',')]): #if it is a broadcast
                    if receivePort == ALICEPORT:
                        mailBoxes[BOBPORT].append('*'+data[data.find(',')+1:])
                        print('got broadcast message for bob: '+ data[:data.find(',')])
                        mailBoxes[MANAGERPORT].append('*'+data[data.find(',')+1:])
                        mailBoxes[ALICEPORT].append('*'+data[data.find(',')+1:])

                        # sendTo(BOBPORT,writable,encodeSock('*'+data[data.find(',')+1:],BOBPORT))
                        # print('bc from alice: ' + data)

                    if receivePort == BOBPORT:
                        mailBoxes[ALICEPORT].append('*'+data[data.find(',')+1:])
                        print('got broadcast message for alice: '+ data[:data.find(',')])
                        mailBoxes[MANAGERPORT].append('*'+data[data.find(',')+1:])
                        mailBoxes[BOBPORT].append('*'+data[data.find(',')+1:])

                        # sendTo(ALICEPORT,writable,encodeSock('*'+data[data.find(',')+1:],ALICEPORT))
                        # print('bc from bob: ' + data )

            else:
                print("un-message: " + data)

    for w in writable:
        wPort = w.getsockname()[1]
        if(wPort in localPorts) and (len(mailBoxes[wPort])>0):
            sendTo(w,encodeSock(mailBoxes[wPort].popleft(),wPort))

    time.sleep(0.001)
#copter.close()
