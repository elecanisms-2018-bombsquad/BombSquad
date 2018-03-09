#!/usr/bin/env python

import os, sys
import Tkinter as tk
import tkFileDialog
import bootloader

class bootloadergui:

    class pic:

        def __init__(self, name = None, lastpage = None):
            self.name = name
            self.lastpage = lastpage

        def __str__(self):
            return "(name = '{0:s}', lastpage = 0x{1:X})".format(self.name, self.lastpage)

        def __repr__(self):
            return "(name = '{0:s}', lastpage = 0x{1:X})".format(self.name, self.lastpage)

    def __init__(self):
        self.flash = []
        for i in range(0x2AC00):
            if i%2==0:
                self.flash.append(0xFFFF)
            else:
                self.flash.append(0xFF)
        self.lastpage = 0x2A800

        self.pic_table = {}
        self.pic_table['4100'] = self.pic('PIC24FJ128GB206', 0x15400)
        self.pic_table['4102'] = self.pic('PIC24FJ128GB210', 0x15400)
        self.pic_table['4104'] = self.pic('PIC24FJ256GB206', 0x2A800)
        self.pic_table['4106'] = self.pic('PIC24FJ256GB210', 0x2A800)
        self.pic_table['4203'] = self.pic('PIC24FJ32GB002', 0x5400)
        self.pic_table['4207'] = self.pic('PIC24FJ64GB002', 0xA800)
        self.pic_table['420B'] = self.pic('PIC24FJ32GB004', 0x5400)
        self.pic_table['420F'] = self.pic('PIC24FJ64GB004', 0xA800)
        self.pic_table['D001'] = self.pic('PIC24FJ64GB106', 0xA800)
        self.pic_table['D003'] = self.pic('PIC24FJ64GB108', 0xA800)
        self.pic_table['D007'] = self.pic('PIC24FJ64GB110', 0xA800)
        self.pic_table['D009'] = self.pic('PIC24FJ128GB106', 0x15400)
        self.pic_table['D00B'] = self.pic('PIC24FJ128GB108', 0x15400)
        self.pic_table['D00F'] = self.pic('PIC24FJ128GB110', 0x15400)
        self.pic_table['D011'] = self.pic('PIC24FJ192GB106', 0x20800)
        self.pic_table['D013'] = self.pic('PIC24FJ192GB108', 0x20800)
        self.pic_table['D017'] = self.pic('PIC24FJ192GB110', 0x20800)
        self.pic_table['D019'] = self.pic('PIC24FJ256GB106', 0x2A800)
        self.pic_table['D01B'] = self.pic('PIC24FJ256GB108', 0x2A800)
        self.pic_table['D01F'] = self.pic('PIC24FJ256GB110', 0x2A800)

        self.root = tk.Tk()
        self.root.title('PIC24FJ USB Bootloader GUI')

        self.write_bootloader_on_export = tk.BooleanVar()
        self.write_bootloader_on_export.set(0)

        self.verify_on_write = tk.BooleanVar()
        self.verify_on_write.set(1)

        self.clear_buffers_on_erase = tk.BooleanVar()
        self.clear_buffers_on_erase.set(1)
        
        self.display_bootloader = tk.BooleanVar()
        self.display_bootloader.set(0)

        self.menubar = tk.Menu(self.root)
        self.filemenu = tk.Menu(self.menubar, tearoff=0)
        self.filemenu.add_command(label='Import Hex...', command=self.import_hex)
        self.filemenu.add_command(label='Export Hex...', command=self.export_hex)
        self.filemenu.add_separator()
        self.filemenu.add_checkbutton(label='Write Bootloader on Export Hex', variable=self.write_bootloader_on_export)
        self.filemenu.add_separator()
        self.filemenu.add_command(label='Quit', command=self.exit)
        self.menubar.add_cascade(label='File', menu=self.filemenu)
        self.bootloadermenu = tk.Menu(self.menubar, tearoff=0)
        self.bootloadermenu.add_command(label='Read Device', command=self.read_device)
        self.bootloadermenu.add_command(label='Write Device', command=self.write_device)
        self.bootloadermenu.add_command(label='Verify', command=self.verify)
        self.bootloadermenu.add_command(label='Erase', command=self.erase)
        self.bootloadermenu.add_command(label='Blank Check', command=self.blank_check)
        self.bootloadermenu.add_separator()
        self.bootloadermenu.add_checkbutton(label='Verify on Write', variable=self.verify_on_write)
        self.bootloadermenu.add_checkbutton(label='Clear Memory Buffers on Erase', variable=self.clear_buffers_on_erase)
        self.bootloadermenu.add_separator()
        self.bootloadermenu.add_command(label='Connect', command=self.connect)
        self.bootloadermenu.add_command(label='Disconnect/Run', command=self.disconnect)
        self.menubar.add_cascade(label='Bootloader', menu=self.bootloadermenu)
        self.root.config(menu=self.menubar)
        
        self.statusdisplay = tk.Text(self.root, height=2, width=70, font=('Courier', 12), background='#71FF71', state=tk.DISABLED, relief=tk.SUNKEN, border=2)
        self.statusdisplay.pack(anchor=tk.NW, expand=tk.TRUE, fill=tk.X, padx=10, pady=10)

        self.progressbarframe = tk.Frame(self.root, relief=tk.SUNKEN, border=2)
        self.progressbar = tk.Canvas(self.progressbarframe, height=10, background='#FFFFFF', highlightbackground='#FFFFFF', highlightthickness=0)
        self.progressbar.pack(expand=tk.TRUE, fill=tk.X)
        self.progressbarframe.pack(anchor=tk.NW, expand=tk.TRUE, fill=tk.X, padx=10, pady=5)

        self.buttonframe = tk.Frame(self.root)
        self.readbutton = tk.Button(self.buttonframe, text='Read', command=self.read_device)
        self.readbutton.pack(side=tk.LEFT, padx=5)
        self.writebutton = tk.Button(self.buttonframe, text='Write', command=self.write_device)
        self.writebutton.pack(side=tk.LEFT, padx=5)
        self.verifybutton = tk.Button(self.buttonframe, text='Verify', command=self.verify)
        self.verifybutton.pack(side=tk.LEFT, padx=5)
        self.erasebutton = tk.Button(self.buttonframe, text='Erase', command=self.erase)
        self.erasebutton.pack(side=tk.LEFT, padx=5)
        self.blankchkbutton = tk.Button(self.buttonframe, text='Blank Check', command=self.blank_check)
        self.blankchkbutton.pack(side=tk.LEFT, padx=5)
        self.connectbutton = tk.Button(self.buttonframe, text='Connect', command=self.connect)
        self.connectbutton.pack(side=tk.LEFT, padx=5)
        self.disconnectbutton = tk.Button(self.buttonframe, text='Disconnect/Run', command=self.disconnect)
        self.disconnectbutton.pack(side=tk.LEFT, padx=5)
        self.buttonframe.pack(side=tk.TOP, anchor=tk.NW, padx=5, pady=5)
        
        self.flashdispframe = tk.LabelFrame(self.root, text='Program Memory', padx=5, pady=5)
        self.flashdispframe.pack(anchor=tk.NW, expand=tk.TRUE, fill=tk.BOTH, padx=10, pady=10)
        self.flashdisplaybootloader = tk.Checkbutton(self.flashdispframe, text='Display Bootloader', variable=self.display_bootloader, command=self.update_flash_display)
        self.flashdisplaybootloader.pack(side=tk.TOP, anchor=tk.NW)
        self.flashdisplay = tk.Frame(self.flashdispframe, relief=tk.SUNKEN, border=2)
        self.flashtext = tk.Text(self.flashdisplay, height=32, width=50, font=('Courier', 12))
        self.flashscrollbar = tk.Scrollbar(self.flashdisplay, command=self.flashtext.yview)
        self.flashtext.configure(yscrollcommand=self.flashscrollbar.set)
        self.flashtext.pack(side=tk.LEFT, expand=tk.TRUE, fill=tk.BOTH)
        self.flashscrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.flashdisplay.pack(expand=tk.TRUE, fill=tk.BOTH)
        self.update_flash_display()
        
        self.connected = False
        self.connect()
        self.display_progress()

    def bootloadermenu_disconnected(self):
        self.bootloadermenu.entryconfig(0, state=tk.DISABLED)
        self.bootloadermenu.entryconfig(1, state=tk.DISABLED)
        self.bootloadermenu.entryconfig(2, state=tk.DISABLED)
        self.bootloadermenu.entryconfig(3, state=tk.DISABLED)
        self.bootloadermenu.entryconfig(4, state=tk.DISABLED)
        self.bootloadermenu.entryconfig(9, state=tk.NORMAL)
        self.bootloadermenu.entryconfig(10, state=tk.DISABLED)
        
        self.readbutton.config(state=tk.DISABLED)
        self.writebutton.config(state=tk.DISABLED)
        self.verifybutton.config(state=tk.DISABLED)
        self.erasebutton.config(state=tk.DISABLED)
        self.blankchkbutton.config(state=tk.DISABLED)
        self.connectbutton.config(state=tk.NORMAL)
        self.disconnectbutton.config(state=tk.DISABLED)

    def bootloadermenu_connected(self):
        self.bootloadermenu.entryconfig(0, state=tk.NORMAL)
        self.bootloadermenu.entryconfig(1, state=tk.NORMAL)
        self.bootloadermenu.entryconfig(2, state=tk.NORMAL)
        self.bootloadermenu.entryconfig(3, state=tk.NORMAL)
        self.bootloadermenu.entryconfig(4, state=tk.NORMAL)
        self.bootloadermenu.entryconfig(9, state=tk.DISABLED)
        self.bootloadermenu.entryconfig(10, state=tk.NORMAL)

        self.readbutton.config(state=tk.NORMAL)
        self.writebutton.config(state=tk.NORMAL)
        self.verifybutton.config(state=tk.NORMAL)
        self.erasebutton.config(state=tk.NORMAL)
        self.blankchkbutton.config(state=tk.NORMAL)
        self.connectbutton.config(state=tk.DISABLED)
        self.disconnectbutton.config(state=tk.NORMAL)

    def display_message(self, message, clear_display = True):
        self.statusdisplay.config(state=tk.NORMAL)
        if clear_display==True:
            self.statusdisplay.delete(1.0, tk.END)
        self.statusdisplay.config(background='#71FF71')
        self.statusdisplay.insert(tk.END, message)
        self.statusdisplay.config(state=tk.DISABLED)
        self.statusdisplay.update()
    
    def display_warning(self, warning, clear_display = True):
        self.statusdisplay.config(state=tk.NORMAL)
        if clear_display==True:
            self.statusdisplay.delete(1.0, tk.END)
        self.statusdisplay.config(background='#FFFF71')
        self.statusdisplay.insert(tk.END, warning)
        self.statusdisplay.config(state=tk.DISABLED)
        self.statusdisplay.update()
    
    def display_error(self, error, clear_display = True):
        self.statusdisplay.config(state=tk.NORMAL)
        if clear_display==True:
            self.statusdisplay.delete(1.0, tk.END)
        self.statusdisplay.config(background='#FF7171')
        self.statusdisplay.insert(tk.END, error)
        self.statusdisplay.config(state=tk.DISABLED)
        self.statusdisplay.update()

    def display_progress(self, fraction = 0.):
        self.progressbar.delete('all')
        if fraction>0.:
            width = fraction*self.progressbar.winfo_width()
            self.progressbar.create_rectangle([0., 0., width, 10.], outline='', fill='#71FF71')
        self.progressbar.update()

    def exit(self):
        sys.exit(0)

    def write_device(self):
        self.display_warning('Erasing program memory...')
        for address in range(0x1000, self.lastpage, 0x400):
            self.bootloader.erase_flash(address)
            self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
        self.display_warning('Writing program memory...')
        for address in range(0x1000, self.lastpage, 32):
            bytes = []
            for i in range(32):
                bytes.append((self.flash[address+i])&0xFF)
                bytes.append((self.flash[address+i])>>8)
            for i in range(64):
                if ((i%4)!=3) and (bytes[i]!=0xFF):
                    break
            else:
                continue
            self.bootloader.write_flash(address, bytes)
            if (address%512)==0:
                self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
        if self.verify_on_write.get()==1:
            if self.verify()==0:
                self.display_message('Write completed successfully.')
        else:
            self.display_warning('Write completed, but not verified.')
        self.display_progress()
    
    def read_device(self):
        self.display_warning('Reading program memory...')
        for address in range(0x0000, self.lastpage, 32):
            bytes = self.bootloader.read_flash(address, 64)
            for j in range(len(bytes)/2):
                self.flash[address+j] = bytes[2*j]+256*bytes[2*j+1]
            if (address%512)==0:
                self.display_progress(float(address)/float(self.lastpage))
        self.display_message('Read device completed successfully.')
        self.update_flash_display()
        self.display_progress()
    
    def verify(self):
        self.display_warning('Verifying program memory...')
        for address in range(0x1000, self.lastpage, 64):
            if (address%512)==0:
                self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
            bytes = self.bootloader.read_flash(address, 64)
            for i in range(32):
                if (bytes[2*i]+256*bytes[2*i+1])!=self.flash[address+i]:
                    break
            else:
                continue
            self.display_error('Verification failed.\nRead 0x{0:04X} at location 0x{1:05X} in program memory, expecting 0x{2:04X}.'.format(bytes[2*i]+256*bytes[2*i+1], address+i, self.flash[address+i]))
            self.display_progress()
            return -1
        self.display_message('Verification succeeded.')
        self.display_progress()
        return 0

    def erase(self):
        self.display_warning('Erasing program memory...')
        for address in range(0x1000, self.lastpage, 0x400):
            self.bootloader.erase_flash(address)
            self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
        self.display_message('Device erased sucessfully.')
        if self.clear_buffers_on_erase.get()==1:
            self.clear_flash()
            self.update_flash_display()
        self.display_progress()

    def blank_check(self):
        self.display_warning('Checking program memory...')
        for address in range(0x1000, self.lastpage, 32):
            if (address%512)==0:
                self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
            bytes = self.bootloader.read_flash(address, 64)
            for i in range(64):
                if ((i%4)!=3) and (bytes[i]!=0xFF):
                    break
            else:
                continue
            self.display_error('Device is not blank.\nRead 0x{0:04X} at location 0x{1:05X} in program memory.'.format(bytes[i/2]+256*bytes[(i/2)+1], address+i/2))
            self.display_progress()
            return
        self.display_message('Device is blank.')
        self.display_progress()

    def connect(self):
        self.bootloader = bootloader.bootloader()
        if self.bootloader.dev>=0:
            self.connected = True
            ret = self.bootloader.read_flash(0xFF0000, 2)
            key = '{:02X}{:02X}'.format(ret[1], ret[0])
            self.lastpage = self.pic_table[key].lastpage
            self.display_message('Connected to a PIC24FJ USB bootloader device ({:s}).'.format(self.pic_table[key].name))
            self.bootloadermenu_connected()
        else:
            self.connected = False
            self.display_error('Could not connect to a PIC24FJ USB bootloader device.\nPlease connect one and then select Bootloader > Connect to proceed.')
            self.bootloadermenu_disconnected()
    
    def disconnect(self):
        self.bootloader.start_user()
        self.bootloader.close()
        self.connected = False
        self.bootloadermenu_disconnected()
        self.display_message('Disconnected from PIC24FJ USB bootloader device.\nStarting user code.')

    def clear_flash(self):
        for i in range(0x2AC00):
            if i%2==0:
                self.flash[i] = 0xFFFF
            else:
                self.flash[i] = 0xFF

    def update_flash_display(self):
        self.flashtext.config(state=tk.NORMAL)
        self.flashtext.delete(1.0, tk.END)
        if self.display_bootloader.get()==1:
            start = 0x0000
        else:
            start = 0x1000
        first = True
        for address in range(start, self.lastpage, 8):
            if first==True:
                first = False
                line = ''
            else:
                line = '\n'
            line = line + '{0:05X}: '.format(address)
            for i in range(0, 8, 2):
                line = line + '{0:02X}{1:02X}{2:02X} '.format(self.flash[address+i]&0xFF, self.flash[address+i]>>8, self.flash[address+i+1]&0xFF)
            line = line + ' '
            for i in range(0, 8, 2):
                for byte in (self.flash[address+i]&0xFF, self.flash[address+i]>>8, self.flash[address+i+1]&0xFF):
                    if (byte>=32) and (byte<127):
                        line = line + '{0}'.format(chr(byte))
                    else:
                        line = line + '.'
            self.flashtext.insert(tk.END, line)
        self.flashtext.config(state=tk.DISABLED)
    
    def import_hex(self):
        filename = tkFileDialog.askopenfilename(parent=self.root, title='Import Hex File', defaultextension='.hex', filetypes=[('HEX file', '*.hex'), ('All files', '*')])
        if filename=='':
            return
        hexfile = open(filename, 'r')
        self.clear_flash()
        address_high = 0
        for line in hexfile:
            line = line.strip('\n')
            byte_count = int(line[1:3], 16)
            address_low = int(line[3:7], 16)
            record_type = int(line[7:9], 16)
            if record_type==0:
                address = ((address_high<<16)+address_low)>>1
                for i in range(0, byte_count, 2):
                    if (address>=0x1000) and (address<self.lastpage):
                        self.flash[address] = int(line[9+2*i:11+2*i], 16)+(int(line[11+2*i:13+2*i], 16)<<8)
                    address = address+1
            elif record_type==4:
                address_high = int(line[9:13], 16)
        hexfile.close()
        self.update_flash_display()

    def export_hex(self):
        filename = tkFileDialog.asksaveasfilename(parent=self.root, title='Export Hex File', defaultextension='.hex', filetypes=[('HEX file', '*.hex'), ('All files', '*')])
        if filename=='':
            return
        hexfile = open(filename, 'w')
        hexfile.write(':020000040000FA\n')
        if self.write_bootloader_on_export.get()==1:
            for address in range(0x0000, 0x1000, 8):
                for i in range(7, -1, -1):
                    if ((i%2)==0) and (self.flash[address+i]!=0xFFFF):
                        end = i
                        break
                    elif ((i%2)==1) and (self.flash[address+i]!=0xFF):
                        end = i
                        break
                else:
                    continue
                for i in range(8):
                    if ((i%2)==0) and (self.flash[address+i]!=0xFFFF):
                        start = i
                        break
                    elif ((i%2)==1) and (self.flash[address+i]!=0xFF):
                        start = i
                        break
                line = ':{0:02X}{1:04X}00'.format(2*(end-start+1), 2*(address+start))
                checksum = 2*(end-start+1) + ((2*(address+start))>>8) + ((2*(address+start))&0xFF)
                for i in range(start, end+1):
                    line = line + '{0:02X}{1:02X}'.format(self.flash[address+i]&0xFF, self.flash[address+i]>>8)
                    checksum = checksum + (self.flash[address+i]&0xFF) + (self.flash[address+i]>>8)
                line = line + '{0:02X}\n'.format(0x100-(checksum&0xFF))
                hexfile.write(line)
        for address in range(0x1000, 0x8000, 8):
            for i in range(7, -1, -1):
                if ((i%2)==0) and (self.flash[address+i]!=0xFFFF):
                    end = i
                    break
                elif ((i%2)==1) and (self.flash[address+i]!=0xFF):
                    end = i
                    break
            else:
                continue
            for i in range(8):
                if ((i%2)==0) and (self.flash[address+i]!=0xFFFF):
                    start = i
                    break
                elif ((i%2)==1) and (self.flash[address+i]!=0xFF):
                    start = i
                break
            line = ':{0:02X}{1:04X}00'.format(2*(end-start+1), 2*(address+start))
            checksum = 2*(end-start+1) + ((2*(address+start))>>8) + ((2*(address+start))&0xFF)
            for i in range(start, end+1):
                line = line + '{0:02X}{1:02X}'.format(self.flash[address+i]&0xFF, self.flash[address+i]>>8)
                checksum = checksum + (self.flash[address+i]&0xFF) + (self.flash[address+i]>>8)
            line = line + '{0:02X}\n'.format(0x100-(checksum&0xFF))
            hexfile.write(line)
        hexfile.write(':020000040001F9\n')
        for address in range(0x8000, self.lastpage, 8):
            for i in range(7, -1, -1):
                if ((i%2)==0) and (self.flash[address+i]!=0xFFFF):
                    end = i
                    break
                elif ((i%2)==1) and (self.flash[address+i]!=0xFF):
                    end = i
                    break
            else:
                continue
            for i in range(8):
                if ((i%2)==0) and (self.flash[address+i]!=0xFFFF):
                    start = i
                    break
                elif ((i%2)==1) and (self.flash[address+i]!=0xFF):
                    start = i
                break
            line = ':{0:02X}{1:04X}00'.format(2*(end-start+1), (2*(address+start))&0xFFFF)
            checksum = 2*(end-start+1) + (((2*(address+start))>>8)&0xFF) + ((2*(address+start))&0xFF)
            for i in range(start, end+1):
                line = line + '{0:02X}{1:02X}'.format(self.flash[address+i]&0xFF, self.flash[address+i]>>8)
                checksum = checksum + (self.flash[address+i]&0xFF) + (self.flash[address+i]>>8)
            line = line + '{0:02X}\n'.format(0x100-(checksum&0xFF))
            hexfile.write(line)
        hexfile.write(':00000001FF\n')
        hexfile.close()

if __name__=='__main__':
    boot = bootloadergui()
    boot.root.mainloop()
