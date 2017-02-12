#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
ZetCode Tkinter tutorial

This script shows a simple window
on the screen.

Author: Jan Bodnar
Last modified: November 2015
Website: www.zetcode.com
"""
try:
    from Tkinter import Tk, Frame, BOTH
except ImportError:
        from tkinter import *

from xbee import XBee, ZigBee
import serial

root = Tk()
x = StringVar()
xval = 0

x.set("testing")
text = Label(root, textvariable=x)

class Example(Frame):
    def __init__(self, parent):
        Frame.__init__(self, parent, background="white")   
        self.parent = parent
        self.initUI()
    
    def initUI(self):
        global x
        text = Label(root, textvariable=x)
        text.place(x=5, y=5)
        self.parent.title("Simple")
        self.pack(fill=BOTH, expand=1)

def loop():
    global x
    global xval
    x.set(str(xval))
    xval = xval + 1
    root.after(2000, loop)


# read a data frame
def xbee_read(): 
    pass

serial_port = serial.Serial('/dev/ttyUSB0', 9600)
xbee = XBee(serial_port, callback=xbee_read)

def main():
    root.geometry("250x150+300+300")
    app = Example(root)
    root.after(2000, loop)
    root.mainloop()  


if __name__ == '__main__':
    main()