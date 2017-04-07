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
    from Tkinter import *
    from ttk import *
except ImportError:
    import tkinter as tk
    from tkinter import *
    from tkinter import ttk
    from tkinter import font

import serial

import csv
from time import sleep, strftime
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use('TkAgg')

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
# implement the default mpl key bindings
from matplotlib.backend_bases import key_press_handler
from matplotlib.figure import Figure

labelfont = 0
stale_telem = False
testint = 0

## telem label text
labelText = ['Mission Time:', 'Packet Count:', 'Alt. Sensor:', 'Pressure:', 'Speed:', 'Temperature:', 'Voltage:',
    'Heading:', 'Software State:']

# units for each label
units = ['seconds', 'packets', 'meters', 'pascals', 'meters/s', 'deg. C', 'volts', 'degrees']
# telem values
vals = []

#telem labels
labels = []

class Example(Frame):
  
    def __init__(self, parent):
        Frame.__init__(self, parent, background="white")   
         
        self.parent = parent
        
        self.xList = [-100]
        self.yList = [-100]       
        self.x = 0
        self.y = 0
        self.count = 0
        self.initUI()
        self.initXbee()
        self.xbee = None

    
    def initUI(self):
      
        self.parent.title("Flying Tigers Ground Control Station")
        self.pack(fill=BOTH, expand=1)

        #Manual Release Button
        quitButton = Button(self, text="Manual Release", padx=5, pady=5, font=labelfont,
            command=self.buttonClick, bg="#ff5e5e")
        quitButton.place(x=10, y=500)
        self.releaseLabel = Label(self, text="Ready to Release", font = labelfont, width=18, bg='#a0ff60')
        self.releaseLabel.place(x=200, y=510)

        #Title and such
        fonty = font.Font(family="Times", size=16, weight="bold")
        title = Label(self, text="Field", font=fonty, width=5, bg="#FFFFFF")
        dat = Label(self, text='Value', font = fonty, width=5, bg="#FFFFFF")
        unit = Label(self, text='Units', font=fonty, width=5, bg="#FFFFFF")
        title.place(x=5, y=5)
        dat.place(x=175, y=5)
        unit.place(x=255, y=5)

        # Create labels
        dataArray  = []
        x = 5
        y = 50
           
        for i in range (0, len(labelText)):
            vals.append(tk.StringVar())
            vals[i].set('0.000')
            labels.append(Label(self, textvariable=vals[i], font=labelfont, width=5, bg='#ffffff'))
            l = Label(self, text=labelText[i], font=labelfont, width=13, bg="#FFFFFF", anchor='w')
            if i != len(labelText) - 1:
                u = Label(self, text=units[i], font=labelfont, width=10, bg="#FFFFFF", anchor='w')
                u.place(x=x+250, y=y)
            labels[i].place(x=x+175, y=y)
            l.place(x=x, y=y)
            y = y + 45

        #canvas 
        my_dpi = 100
        fig = Figure(figsize=(600/my_dpi, 600/my_dpi), dpi=my_dpi)
        self.a = fig.add_subplot(1, 1, 1)
        self.a.set_xlim(-550, 550)
        self.a.set_ylim(-550, 550)
        self.a.grid()
        self.line, = self.a.plot(self.xList, self.yList, 'ro')
        self.canvas  = FigureCanvasTkAgg(fig, master=self)
        self.canvas.show()
        self.canvas.get_tk_widget().pack(side=tk.RIGHT, fill=tk.NONE, expand=0)

    def buttonClick(self):
        self.releaseLabel.config(text='Release Fired', bg='#ff5e5e')

    def update_plot(self):
        self.xList.append(self.xList[-1] + 20)
        self.yList.append(self.yList[-1] + 20)
        self.line.set_data(self.xList, self.yList)
        self.parent.after(500, self.update_plot)
        self.canvas.draw()

    def initXbee(self):
        # find and set xbee serial port
        ports = ['COM%s' % (i + 1) for i in range(256)]
        result = []
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
                print(port)
            except (OSError, serial.SerialException):
                pass

        for port in result:
            print(port)

        if len(result) > 0:
            serial_port = serial.Serial(result[0], 9600)
            self.xbee = XBee(serial_port, callback=self.update_telem)

    def update_telem(self, data):
        print(data)

    def xbee_read(self):
        stale_telem = False
        #open csv and write new data
        with open('telem' + strftime('%d.%m.%y') + '.csv', 'a', newline='') as csvfile:
            csvwriter = csv.writer(csvfile)
            csvwriter.writerow(['test Param', str(self.count)])
        self.parent.after(250, self.xbee_read)

def main():
    root = Tk() 
    labelfont = font.Font(root=root, family="Times", size=16)
    root.geometry("1000x600+100+100")
    app = Example(root)
    root.after(1000, app.update_plot)
    root.after(1000, app.xbee_read)
    root.mainloop()

#update plot if no new data has come in


def age_out():
    stale_telem = True



if __name__ == '__main__':
    main()