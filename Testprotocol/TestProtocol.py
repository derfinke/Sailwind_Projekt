import glob
import os
import sys

import serial


def main():
    try:
        ser1 = serial.Serial(serial_ports()[0], 115200)
        start_menu(ser1)
        ser1.close()
    except IndexError:
        print("no serial port available!")


def test_menu(ser: serial.Serial):
    os.system("cls")
    test_menu_str = """choose and enter Test ID

0	    - back to start menu

1	    - LED Test

2x	    - set motor function x {0...7}

3xxxx	- set motor rpm xxxx {0000...3000}

4	    - Endswitch Test

5	    - Motor Test
511	    - OUT1: start rpm measurement
512	    - OUT1: get rpm value
52	    - OUT2: get motor error
53	    - OUT3: get motor direction

6	    - Button Test

7	    - FRAM Test
Selection: """
    selection = "0"
    try:
        selection = input(test_menu_str).zfill(5)
        if int(selection) == 0:
            return
    except ValueError:
        test_menu(ser)
    ser.write(bytes(f"{selection}\r\n", "utf-8"))
    os.system("cls")
    print("wait for response...")
    while True:
        response = ser.readline().decode("utf-8")
        print(response)
        if f"Test {int(selection)} done" in response:
            input("Press Enter to leave test..")
            break
    test_menu(ser)


def select_com_port(ser):
    try:
        ser.close()
        new_ser = serial.Serial(f"COM{input(f'select COM port {serial_ports()}: ')}", ser.baudrate)
        return new_ser
    except serial.SerialException:
        print("COM Port not available")
        return select_com_port(ser)


def start_menu(ser: serial.Serial):
    os.system("cls")
    start_menu_str = """choose and enter option:
1 - change COM Port
2 - change Baud rate
3 - start test
4 - EXIT
Selection: """
    print(f"{ser}\n")
    selection = int(input(start_menu_str))
    os.system("cls")
    try:
        if selection == 1:
            ser = select_com_port(ser)
        elif selection == 2:
            ser.baudrate = int(input("enter Baud rate: "))
        elif selection == 3:
            test_menu(ser)
        elif selection == 4:
            return
    except ValueError:
        pass

    start_menu(ser)


def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


main()
