import socket
import select
from threading import Thread
import Queue
import time
import sys

class ESP(Thread):
    def __init__(self, port, name, timeout=0.1):
        Thread.__init__(self)
        self.setDaemon(True)

        self._inbox = Queue.Queue()
        self._port = port
        self._name = name
        self._ID = port
        self.interface = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.interface.connect(('localhost', port))

        self.buff=''
        self.lastTime = time.time()

        self.start()

    def run(self):
        while True:
            if time.time()-self.lastTime > 15:
                print('sending request')
                sys.stdout.flush()
                self.send('connListRequest')
                self.lastTime = time.time()
            msg, self.buff = self._readlines(self.interface,self.buff)
            if len(msg)>0:
                self._inbox.put(msg[:-1])
            time.sleep(0.01)

    def _encode(self, msg):
        return msg + '\n'

    def send(self, msg):
        self.interface.sendall(self._encode(msg))

    def read(self):
        try:
            return self._inbox.get(False)
        except Queue.Empty:
            return ''

    def getID(self):
        return self._ID

    def _readlines(self, sock, extBuff, recv_buffer=16384, delim='\n'):
        data = True
        buff = extBuff
        line = ''
        readable, _, _ = select.select([sock],[], [], 0.1)
        for s in readable:
            while data:
                data = sock.recv(recv_buffer)
                buff += data
                while buff.find(delim) != -1:
                    line, buff = buff.split('\n', 1)
                    return line+'\n', buff
        return line, buff
