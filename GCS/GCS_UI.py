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
import threading
import time
import math

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
    'Heading:', 'Software State:', 'Picture count']

# units for each label
units = ['seconds', 'packets', 'meters', 'pascals', 'meters/s', 'deg. C', 'volts', 'degrees', 'state', 'Pictures']

# glider telem values
glider_vals = []

# canister telem values
container_vals = []

#telem labels
glider_labels = []
canister_labels = []

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
        self.xbee = None
        self.initXbee()
        self.last_packet_time = 0
        self.lastX = 0
        self.lastY = 0
        self.ground_pressure = 0
        self.ground_pressure_count = 0
        self.lines_written = 0
 
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
        glider = Label(self, text='Glider', font = fonty, width=5, bg="#FFFFFF")
        canister = Label(self, text='Canister', font = fonty, width=7, bg="#FFFFFF")
        unit = Label(self, text='Units', font=fonty, width=5, bg="#FFFFFF")
        title.place(x=5, y=5)
        glider.place(x=175, y=5)
        canister.place(x=250, y=5)
        unit.place(x=345, y=5)

        # Create labels
        dataArray  = []
        x = 5
        y = 50
           
        for i in range (0, len(labelText)):
            glider_vals.append(tk.StringVar())
            container_vals.append(tk.StringVar())
            glider_vals[i].set('0.000')
            container_vals[i].set('0.000')
            glider_labels.append(Label(self, textvariable=glider_vals[i], font=labelfont, width=5, bg='#ffffff'))
            canister_labels.append(Label(self, textvariable=container_vals[i], font=labelfont, width=5, bg='#ffffff'))
            l = Label(self, text=labelText[i], font=labelfont, width=13, bg="#FFFFFF", anchor='w')
            if i != len(labelText):
                u = Label(self, text=units[i], font=labelfont, width=10, bg="#FFFFFF", anchor='w')
                u.place(x=x+345, y=y)
            glider_labels[i].place(x=x+175, y=y)
            canister_labels[i].place(x=x+250, y=y)
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
        if self.xbee != None:
            self.xbee.write("releasing".encode())
        else:
            print("ERROR !!NO XBEE!!")

    def update_plot(self):
        self.set_coords()
        self.line.set_data(self.xList, self.yList)
        self.parent.after(500, self.update_plot)
        self.canvas.draw()

    def set_coords(self):
        self.x = self.x + math.cos(float(glider_vals[7].get())) * float(glider_vals[4].get())
        self.y = self.y + math.sin(float(glider_vals[7].get())) * float(glider_vals[4].get())
        self.xList.append(self.x + 10)
        self.yList.append(self.y + 10)

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
            serial_port = serial.Serial(result[0], 19200)
            self.xbee = serial_port;
            thread = threading.Thread(target = self.xbee_read, args=(serial_port,))
            thread.start()

    def update_telem(self, data):
        print(data)
        split_vals = data.split(",")
        file_str = strftime("%d.%m.%y")
        ## 4234, GLIDER, 
        if (split_vals[1] == 'GLIDER'):
            #do glider telem
            glider_vals[0].set(split_vals[2]) #mission time
            glider_vals[1].set(split_vals[3]) #packet count
            glider_vals[2].set(split_vals[4])   #altitude
            glider_vals[3].set(split_vals[5])   #pressure
            glider_vals[4].set(split_vals[6])   #speed
            glider_vals[5].set(split_vals[7])   #Temperature
            glider_vals[6].set(split_vals[9])   #voltage
            heading = str(math.radians(float(split_vals[10])))
            glider_vals[7].set(heading)  #heading
            glider_vals[8].set(split_vals[11])  #state
            glider_vals[9].set(split_vals[12])  #picture count

            file_str += ".glider"

        elif (split_vals[1] == 'CONTAINER'):
            #do container telem
            container_vals[0].set(split_vals[2]) #mission time
            container_vals[1].set(split_vals[3]) #packet count
            container_vals[2].set(split_vals[4])   #altitude
            # get ground pressure
            if self.ground_pressure_count < 6:
               self.ground_pressure *= self.ground_pressure_count
               self.ground_pressure += float(split_vals[4])
               self.ground_pressure_count += 1
               self.ground_pressure /= self.ground_pressure_count      

            container_vals[3].set(split_vals[5])   #pressure
            container_vals[4].set(split_vals[6])   #speed
            container_vals[5].set(split_vals[7])   #Temperature
            container_vals[6].set(split_vals[9])   #voltage
            heading = str(math.radians(float(split_vals[10])))
            container_vals[7].set(heading)  #heading
            container_vals[8].set(split_vals[11])  #state
            
            file_str += '.container'

        else:
            pass
            #bad value

        with open('telem.' + file_str + '.csv', 'a', newline='') as csvfile:
                 csvwriter = csv.writer(csvfile)
                 csvwriter.writerow(split_vals)

    def xbee_read(self):
        stale_telem = False

        if self.xbee != None:
            reading = self.xbee.readline().decode()
            self.last_packet_time = int(round(time.time() * 1000))
            update_telem(reading)
        #open csv and write new data
        self.parent.after(250, self.xbee_read)

def main():
    root = Tk() 
    labelfont = font.Font(root=root, family="Times", size=16)
    root.geometry("1050x600+100+100")
    app = Example(root)
    root.after(1000, app.update_plot)
    root.after(1000, app.xbee_read)
    root.mainloop()

#update plot if no new data has come in

def age_out():
    stale_telem = True

if __name__ == '__main__':
    main()