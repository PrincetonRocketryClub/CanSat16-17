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

#labelfont
labelfont = 0

## telem label text
labelText = ['Mission Time:', 'Packet Count:', 'Alt. Sensor:', 'Pressure:', 'Speed:', 'Temperature:', 'Voltage:',
    'Heading:', 'Software State:']
# telem values
vals = []

#telem labels
labels = []

class Example(Frame):
  
    def __init__(self, parent):
        Frame.__init__(self, parent, background="white")   
         
        self.parent = parent
        
        self.initUI()
        
    
    def initUI(self):
      
        self.parent.title("Flying Tigers Ground Control Station")
        self.pack(fill=BOTH, expand=1)

        #Manual Release Button
        quitButton = Button(self, text="Manual Release", padx=5, pady=5, font=labelfont,
            command=self.quit, bg="#ff5e5e")
        quitButton.place(x=10, y=500)

        #Title and such
        fonty = font.Font(underline=True, family="Times", size=16, weight="bold")
        title = Label(self, text="Telemetry Field", font=fonty, width=12)
        dat = Label(self, text='Value', font = fonty, width=5)
        unit = Label(self, text='Units', font=fonty, width=5)
        title.place(x=5, y=5)
        dat.place(x=155, y=5)
        unit.place(x=235, y=5)

        # Create labels
        dataArray  = []
        x = 5
        y = 50
        for i in range (0, len(labelText)):
            l = Label(self, text=labelText[i], font=labelfont, width=12)
            d = Label(self, text=vals[i], font=labelfont, width=5)
            u = Label(self, text='units', font=labelfont, width=5)
            l.place(x=x, y=y)
            #d.place(x=x+155, y=y)
            u.place(x=x+230, y=y)
            #labels.append(l)
            dataArray.append(d)
            y = y + 50

        #canvas
        canvas = Canvas(self, width=500, height=500, borderwidth=2)
        canvas.create_line(250, 0, 250, 500)
        canvas.create_line(0, 250, 500, 250)
        canvas.place(x=350, y=10)
        devLabel = Label(self, text='Displacement:', font=labelfont)
        devLabel.place(x=400, y=520)
        xPosLabel = Label(self, text='x:', font=labelfont)
        xPos = Label(self, text='0m E', font=labelfont)
        yPosLabel = Label(self, text='y:', font=labelfont)
        yPos = Label(self, text='500m N', font=labelfont)    
        xPosLabel.place(x=550, y=520)
        yPosLabel.place(x=650, y=520)
        xPos.place(x=570, y=520)
        yPos.place(x=670, y=520)

        radius = 5
        canvas.create_oval(245, 128, 255, 137, fill='red', outline='red')
        canvas.create_oval(247, 247, 252, 252, fill='blue', outline='blue')
        canvas.create_oval(260, 250, 265, 245, fill='blue', outline='blue')
        canvas.create_oval(270, 240, 275, 245, fill='blue', outline='blue')
        canvas.create_oval(275, 230, 280, 235, fill='blue', outline='blue')
        canvas.create_oval(278, 225, 283, 220, fill='blue', outline='blue')
        canvas.create_oval(280, 210, 285, 205, fill='blue', outline='blue')
        canvas.create_oval(278, 195, 283, 190, fill='blue', outline='blue')
        canvas.create_oval(273, 170, 278, 175, fill='blue', outline='blue')
        canvas.create_oval(265, 150, 270, 155, fill='blue', outline='blue')

def main():
    root = Tk()
    init(root) 
    root.geometry("900x600+100+100")
    app = Example(root)
    root.mainloop()
    for i in range(6):
        #vals[0] += 1
        root.update_idletasks()

def init(root):
    labelfont = font.Font(root=root, family="Times", size=16)
    for i in range (0, len(labelText)):
        vals.append(tk.StringVar())
        vals[i].set('test')
    updateTelem(root)

def updateTelem(root):
    x = 5
    y = 50      
    for i in range(0, len(labelText)):
        labels.append(Label(root, text="test", font=labelfont, width=5))
        labels[i].place(x=x, y=y)
        y += 50

if __name__ == '__main__':
    main()