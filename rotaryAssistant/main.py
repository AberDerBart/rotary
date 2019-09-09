#!/usr/bin/python3
import argparse
import rotary_serial
import tabulate

def readSpeedDial(ser):
    speedDial = ser.getSpeedDial()
    printable = [(index, data[0], data[1]) for index, data in speedDial.items()]
    print(tabulate.tabulate(printable, tablefmt='plain'))

def printStatus(ser):
    batt = ser.batteryPercent()

    data = []
    data.append(('Battery:', '{}%'.format(batt)))

    print(tabulate.tabulate(data, tablefmt='plain'))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port","-p", help="the port to connect to",
      default = "/dev/ttyACM0")

    subparsers = parser.add_subparsers(dest = "command")
    subparsers.required = False

    statusParser = subparsers.add_parser("status",
      help="displays status information")
    statusParser.add_argument('--follow', '-w',
      help="display updates until termination")

    setParser = subparsers.add_parser("set",
      help="sets a speed dial entry")
    setParser.add_argument("index",
      help="the speed dial index to set")
    setParser.add_argument("number",
      help="the number to set as speed dial")
    setParser.add_argument("--name",
      help="the name for the phonebook entry")

    readParser = subparsers.add_parser("read",
      help="displays the speed dial entries")

    args = parser.parse_args()

    ser = rotary_serial.RotarySerial(args.port)

    if args.command == "read":
        readSpeedDial(ser)
    if args.command == "set":
        ser.setSpeedDial(args.index, args.number, args.name)
    if args.command is None or args.command == "status":
        printStatus(ser)

if __name__ == "__main__":
    main()
