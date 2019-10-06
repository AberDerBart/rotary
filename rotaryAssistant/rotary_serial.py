import serial
import time
import re

class RotarySerial:
    _ANSWERS = {
        'OK': True,
        '\+CME ERROR:.*': False,
        'ERROR': False,
    }

    def __init__(self, port='/dev/ttyACM0', baudrate=115200, timeout=2):
        self.conn = serial.Serial(port, baudrate, timeout=timeout)

    def _read_till_response(self, timeout = None):

        # TODO: implement adjustment of timeout
        resp = []
        line = self.conn.readline()

        while(line):
            line = self.conn.readline().decode('utf-8')

            resp.append(line)

            for a in RotarySerial._ANSWERS.keys():
                if re.match(a, line):
                    return resp
        return resp

    def evalResponse(self, resp):
        for line in resp:
            for a in RotarySerial._ANSWERS.keys():
                if re.match(a, line):
                    return RotarySerial._ANSWERS[a]
        return None

    def processCmd(self, cmd, timeout = None):
        """sends the given AT command to rotary and returns the response"""
        self.conn.write('{}\r\n'.format(cmd).encode('utf-8'))
        return self._read_till_response(timeout)

    def batteryPercent(self):
        """reads the battery percentage from rotary"""
        resp = self.processCmd('AT+CBC')
        for line in resp:
            match = re.match('\+CBC: ([012]),(\d+),(\d+).*', line)
            if match:
                return int(match.group(2))
        return None

    def setSpeedDial(self, index, number, name = None):
        """writes a number to roatrys speed dial"""
        numberType = 145 if '+' in number else 129
        if name is None:
            name = 'speeddial{}'.format(index)

        cmd = 'AT+CPBW={},"{}",{},"{}"'.format(index,
          number, numberType, name)

        resp = self.processCmd(cmd)

        return self.evalResponse(resp)

    def getSpeedDial(self):
        """reads the speed dial numbers from rotary"""
        resp = self.processCmd('AT+CPBR=1,7', timeout = 30)

        speeddial = {}

        for line in resp:
            match = re.match('\+CPBR: (\d+),"([\+0-9]+)",\d+,"(.*)".*', line)
            if match:
                index = match.group(1)
                number = match.group(2)
                name = match.group(3)
                speeddial[index] = (number, name)

        return speeddial


