import serial

ser = serial.Serial("COM3", 19200)
import threading

def xbee_read(data):
	while True:
		print(ser.readline().decode())

thread = threading.Thread(target = xbee_read, args=(ser,))
thread.start()
while True:
	text = input("Enter text: ")
	ser.write(text.strip().encode())


