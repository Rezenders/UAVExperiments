import serial
from threading import Thread
import Queue
import time

class ESP(Thread):
    def __init__(self, port, name, baud_rate=115200, timeout=0.1):
        Thread.__init__(self)
        self.setDaemon(True)

        self._inbox = Queue.Queue()
        self._port = port
        self._name = name
        self._ID = ''
        self.interface = serial.Serial(port, baud_rate, timeout = timeout)

        self.send('ID_REQ')
        while self._ID == '':
            self._ID = self.interface.readline()[:-1]
        self.send(name)
        ok = ''
        while ok != 'NAME_OK':
            ok = self.interface.readline()[:-1]

        self.start()

    def run(self):
        while True:
            msg = self.interface.readline()[:-1]
            if len(msg)>0:
                self._inbox.put(msg)
            time.sleep(0.01)

    def _encode(self, msg):
        return msg + '\n'

    def send(self, msg):
        self.interface.write(self._encode(msg))

    def read(self):
        try:
            return self._inbox.get(False)
        except Queue.Empty:
            return ''

    def getID(self):
        return self._ID
