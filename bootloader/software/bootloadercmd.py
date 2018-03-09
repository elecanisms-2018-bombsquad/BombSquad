#!/usr/bin/env python

import os, sys
import bootloader

class bootloadercmd:

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

        self.write_bootloader_on_export = False
        self.verify_on_write = True
        self.clear_buffers_on_erase = True
        self.display_bootloader = False

        self.connected = False
        self.connect()

    def display_progress(self, fraction = 0., width = 30, ch = '#'):
        barwidth = int(fraction*width+0.5)
        progressbar = '['+ch*barwidth+' '*(width-barwidth)+'] {0:3d}%'.format(int(100*fraction+0.5))
        sys.stdout.write('\r')
        sys.stdout.write(progressbar)
        sys.stdout.flush()

    def write_device(self):
        if self.connected==True:
            print 'Erasing program memory...'
            for address in range(0x1000, self.lastpage, 0x400):
                self.bootloader.erase_flash(address)
                self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
            print '\nWriting program memory...'
            for address in range(0x1000, self.lastpage, 32):
                if (address%512)==0:
                    self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
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
            sys.stdout.write('\n')
            if self.verify_on_write==True:
                if self.verify()==0:
                    print 'Write completed successfully.'
            else:
                print 'Write completed, but not verified.'
        else:
            print 'Could not write device.\nNo connection to a PIC24FJ USB bootloader device.'
    
    def read_device(self):
        if self.connected==True:
            print 'Reading program memory...'
            for address in range(0x0000, self.lastpage, 32):
                bytes = self.bootloader.read_flash(address, 64)
                for j in range(len(bytes)/2):
                    self.flash[address+j] = bytes[2*j]+256*bytes[2*j+1]
                if (address%512)==0:
                    self.display_progress(float(address)/float(self.lastpage))
            print '\nRead device completed successfully.'
        else:
            print 'Could not read device.\nNo connection to a PIC24FJ USB bootloader device.'

    def verify(self):
        if self.connected==True:
            print 'Verifying program memory...'
            for address in range(0x1000, self.lastpage, 64):
                if (address%512)==0:
                    self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
                bytes = self.bootloader.read_flash(address, 64)
                for i in range(32):
                    if (bytes[2*i]+256*bytes[2*i+1])!=self.flash[address+i]:
                        break
                else:
                    continue
                print '\nVerification failed.\nRead 0x{0:04X} at location 0x{1:05X} in program memory, expecting 0x{2:04X}.'.format(bytes[2*i]+256*bytes[2*i+1], address+i, self.flash[address+i])
                return -1
            print '\nVerification succeeded.'
            return 0
        else:
            print 'Could not verify device.\nNo connection to a PIC24FJ USB bootloader device.'
            return -2

    def erase(self):
        if self.connected==True:
            print 'Erasing program memory...'
            for address in range(0x1000, self.lastpage, 0x400):
                self.bootloader.erase_flash(address)
                self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
            print '\nDevice erased sucessfully.'
            if self.clear_buffers_on_erase==True:
                self.clear_flash()
        else:
            print 'Could not erase device.\nNo connection to a PIC24FJ USB bootloader device.'

    def blank_check(self):
        if self.connected==True:
            print 'Checking program memory...'
            for address in range(0x1000, self.lastpage, 32):
                if (address%512)==0:
                    self.display_progress(float(address-0x1000)/float(self.lastpage-0x1000))
                bytes = self.bootloader.read_flash(address, 64)
                for i in range(64):
                    if ((i%4)!=3) and (bytes[i]!=0xFF):
                        break
                else:
                    continue
                print '\nDevice is not blank.\nRead 0x{0:04X} at location 0x{1:05X} in program memory.'.format(bytes[i/2]+256*bytes[(i/2)+1], address+i/2)
                return
            print '\nDevice is blank.'
        else:
            print 'Could not blank check device.\nNo connection to a PIC24FJ USB bootloader device.'

    def connect(self):
        self.bootloader = bootloader.bootloader()
        if self.bootloader.dev>=0:
            self.connected = True
            ret = self.bootloader.read_flash(0xFF0000, 2)
            key = '{:02X}{:02X}'.format(ret[1], ret[0])
            self.lastpage = self.pic_table[key].lastpage
            print 'Connected to a PIC24FJ USB bootloader device ({:s}).'.format(self.pic_table[key].name)
        else:
            self.connected = False
            print 'Could not connect to a PIC24FJ USB bootloader device.\nPlease connect one and try again.'

    def clear_flash(self):
        for i in range(0x2AC00):
            if i%2==0:
                self.flash[i] = 0xFFFF
            else:
                self.flash[i] = 0xFF

    def dump_flash(self, filename = ''):
        if filename=='':
            return
        print "Dumping flash buffer to file '{0}'.".format(filename)
        outfile = open(filename, 'w')
        if self.display_bootloader==True:
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
            outfile.write(line)
        outfile.close()
    
    def import_hex(self, filename = ''):
        if filename=='':
            return
        print "Importing hex file '{0}' to flash buffer.".format(filename)
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

    def export_hex(self, filename = ''):
        if filename=='':
            return
        print "Exporting flash buffer to hex file '{0}'.".format(filename)
        hexfile = open(filename, 'w')
        hexfile.write(':020000040000FA\n')
        if self.write_bootloader_on_export==True:
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

def display_help():
    print '''

NAME

    bootloadercmd.py - Command-line interface to PIC24FJ USB bootloader device

SYNOPSIS

    bootloadercmd.py [-B|+B] [-V|+V] [-i <hex_file>]
                     [-e] [-b] [-r] [-w] [-v]
                     [-x <hex_file>] [-d <dump_file>]
                     [-h|--help]

OPTIONS

    -B    Exclude the bootloader segment of flash from an exported hex or dump 
          file (default behavior).

    +B    Include the bootloader segment of flash from an exported hex or dump 
          file.

    -V    Do not verify the connected PIC24FJ USB bootloader device on write.

    +V    Verify the connected PIC24FJ USB bootloader device on write (default 
          behavior).

    -i <hex_file>
          Import hex file specified by <hex_file> into the flash memory buffer. 

    -e    Erase the program memory of the connected PIC24FJ USB bootloader 
          device.

    -b    Blank check the connected PIC24FJ USB bootloader device.

    -r    Read the program memory of the connected PIC24FJ USB bootloader 
          device.

    -w    Erase the program memory of the connected PIC24FJ USB bootloader 
          device, write the contents of the flash memory buffer, and verify 
          the device if verify on write is enabled.

    -v    Verify that the contents of program memory of the connected PIC24FJ 
          USB bootloader device match those of the flash memory buffer.

    -x <hex_file>
          Export the contents of the flash memory buffer to a hex file 
          specified by <hex_file>.

    -d <dump_file>
          Dump the contents of the flash memory buffer to a text file 
          specified by <dump_file>.

    -h    Display this help.
    '''

def main(argv):
    if ('-h' in argv) or ('--help' in argv):
        display_help()
    boot = bootloadercmd()
    if boot.connected==False:
        return 1
    i = 1
    while i<len(argv):
        if argv[i]=='-B':
            boot.write_bootloader_on_export = False
            boot.display_bootloader = False
        elif argv[i]=='+B':
            boot.write_bootloader_on_export = True
            boot.display_bootloader = True
        elif argv[i]=='-V':
            boot.verify_on_write = False
        elif argv[i]=='+V':
            boot.verify_on_write = True
        elif argv[i]=='-i':
            i = i+1
            boot.import_hex(argv[i])
        elif argv[i]=='-x':
            i = i+1
            boot.export_hex(argv[i])
        elif argv[i]=='-d':
            i = i+1
            boot.dump_flash(argv[i])
        elif argv[i]=='-w':
            boot.write_device()
        elif argv[i]=='-e':
            boot.erase()
        elif argv[i]=='-r':
            boot.read_device()
        elif argv[i]=='-v':
            boot.verify()
        elif argv[i]=='-b':
            boot.blank_check()
        elif argv[i]=='-h' or argv[i]=='--help':
            pass
        else:
            print "Unrecognized command line argument '{0}'.".format(argv[i])
            return 2
        i = i+1
    return 0

if __name__=='__main__':
    sys.exit(main(sys.argv))
