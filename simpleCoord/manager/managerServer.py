import socket
import os
import sys
import time
import select
import re
import math
from collections import deque
from ESPSim import ESP
#MAVLINK RELATED IMPORTS
from dronekit import *

#SERVER
####constants####
JAVAPORT = 6971
#DRONEPORT = '127.0.0.1:14560' #environment simulation
#POSSIBLE DRONEPORTS:
## '127.0.0.1:14550' #environment simulation PC-PC
## '192.168.7.2:14550' #environment simulation Beagle-PC
## '/dev/ttyACM0' #real application via USB Beagle-PixHawk
## '/dev/ttyO4' #real application via serial Beagle-PixHawk.
### Requires baud-rate 57600
ESPPORT = 4447

MIN_DISTANCE = 0.5 #minimum distance to consider as change in position
MIN_ALT = 5
R = 6371; # Radius of the earth

####methods####
def decodeSock(msg, port):
    if port == JAVAPORT:
        return msg[:-1]
    if port == DRONEPORT:
        return re.split('\)',msg)[-2]+')' #get only the last term
    else: return msg

def encodeSock(msg, port):
    if port == JAVAPORT:
        return msg + '\n'
    else: return msg

def createSocket(TCP_PORT):
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    #serverSocket.setblocking(0)
    serverSocket.bind(('localhost', TCP_PORT))
    serverSocket.listen(5)

    return serverSocket

def buildPercept():
    percept = 'pos('+str(pos[0])+ ',' +str(pos[1])+ ',' +str(pos[2])+');' + \
                'status('+status+');'
    for name in connectedAgents:
        percept = percept + 'connected('+name+');'
    return percept

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
localPorts = [JAVAPORT]
sockList = [] #sockets accepted
lastTime = time.time()
buff=''

####System state variables####
pos = [0, 0, 0]
status = 'Ready'
ag_name = ''
agentNameAndID = {}
connectedAgents = []

#Connect to all sub-systems (sockets) before anything
for p in localPorts:
    sock = createSocket(p)
    clientSock, address = sock.accept()
    sockList.append(clientSock)

#Get agent name
while ag_name == '':
    readable, writable, exceptional = select.select(sockList, sockList, sockList, 0.1)
    for s in readable: #for every socket object
        _, receivePort = s.getsockname(); #receivePort = port_number
        data = s.recv(1024)
        if data != '': #if it's not a null message
            decodeData = decodeSock(data, receivePort)
            if receivePort == JAVAPORT: #if it was received from the Agent
                ag_name = decodeData

print(ag_name)
####Application####
#setup ESP
esp = ESP(ESPPORT, ag_name) #send agent name to the ESP
print('ESP setup done: ' + str(esp._ID))
#setup VANT
# copter = connect(DRONEPORT, wait_ready=True) #if ttyO4, set baud=57600
# copter.mode = VehicleMode("GUIDED")
# while not copter.armed:
#     copter.armed= True
#     print(" Waiting for arming...")
#     time.sleep(5)

#initial position
#initial_pos = [float(copter.location.global_relative_frame.lon), float(copter.location.global_relative_frame.lat), float(copter.location.global_relative_frame.alt)]

while True:
    #Check which sockets are readable or writable
    readable, writable, exceptional = select.select(sockList, sockList, sockList, 0.1)

    #percepts update
    if(time.time()-lastTime > 1): #send percepts every second
        lastTime = time.time()
        percept = buildPercept()
        for w in writable:
            if w.getsockname()[1]==JAVAPORT:
                sendTo(w,encodeSock(percept,JAVAPORT))

    #Interpret received messages
    for s in readable: #for every socket object
        _, receivePort = s.getsockname(); #receivePort = port_number

        data, buff = readlines(s,buff)
        #print(data)
        if data != '': #if it's not a null message
            decodeData = decodeSock(data, receivePort)

            if receivePort == JAVAPORT: #if it was received from the Agent
                if '!' in decodeData[0]: #if it's and action
                    if 'launch' in decodeData:
                        print('launching')
                        #copter.simple_takeoff(MIN_ALT)

                    if 'land' in decodeData:
                        print('landing')
                        #copter.mode  = VehicleMode("RTL")

                    if 'setWaypoint' in decodeData:
                        print('setting wp')
                        #_, latwp, lonwp, altwp, _ = re.split('\(|\)|,', decodeData)
                        #wp = LocationGlobalRelative(-30,150,20)
                        #wp = LocationGlobalRelative(float(latwp), float(lonwp), float(altwp))
                        #print(wp)
                        #print(pos)
                        #copter.simple_goto(wp)
                if '*' in decodeData[0]:
                    recipient = decodeData[1:decodeData.find(',')]
                    print('sending msg')

                    for ID, name in agentNameAndID.iteritems():
                        if name == recipient:
                            recipient = ID
                            break

                    esp.send(str(recipient) + ';*' + decodeData[decodeData.find(',')+1:])
                    print(str(recipient) + ' ; ' + decodeData[decodeData.find(',')+1:])


    #serial messages
    mail = esp.read()
    if mail != '':
        pass

        if 'connectedList' in mail:
            connections = re.split(':|;|\[|\]|,',mail)
            connections = set([int(x) for x in connections[2:-1]]) #take 'connectedList' out
            connectionNames = []
            print(str(connections))
            for ID in connections:
                if (ID != '' and ID != esp.getID()):
                    if ID in agentNameAndID:
                        connectionNames.append(agentNameAndID[ID])
                    else:
                        esp.send(str(ID) + ';' + str(esp.getID()) + ',?who?')
            if connectedAgents!=connectionNames:
                print(connectionNames)
            connectedAgents = connectionNames

        else:
            if 'connected' in mail:
                print(mail)
                _, connectionName, connectionID = re.split(':|,',mail)
                print('connected to ' + connectionName)
                agentNameAndID.update({int(connectionID) : connectionName})

            else:
                if '?who?' in mail:
                    sender, _ = re.split(',',mail)
                    esp.send(sender + ';connected:' + ag_name + ',' + str(esp.getID()))

                else:
                    if len(mail)>50:
                        print('u got mail: ' + mail[:50])
                    else:
                        print('u got mail: ' + mail)
                    #pass
                    for w in writable:
                        if w.getsockname()[1]==JAVAPORT:
                            sendTo(w,encodeSock(mail,JAVAPORT))

    #else:
        #s.close()
        #if s in sockList:
            #sockList.remove(s)

    time.sleep(0.01)
#copter.close()
