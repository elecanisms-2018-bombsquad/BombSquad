
import usb.core

class bootloader:

    def __init__(self):
        self.READ_FLASH = 2
        self.WRITE_FLASH = 3
        self.ERASE_FLASH = 4
        self.START_USER = 6
        self.dev = usb.core.find(idVendor = 0x6666, idProduct = 0x4321)
        if self.dev is None:
            print "No USB device found matching idVendor = 0x6666 and idProduct = 0x4321."
        else:
            self.dev.set_configuration()

    def close(self):
        self.dev = None

    def read_flash(self, address, num_bytes):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.READ_FLASH, address>>16, address&0xFFFF, num_bytes)
        except usb.core.USBError:
            print "Unable to send READ_FLASH vendor request."
        else:
            bytes = []
            for i in range(len(ret)):
                bytes.append(int(ret[i]))
            return bytes

    def write_flash(self, address, bytes):
        try:
            self.dev.ctrl_transfer(0x40, self.WRITE_FLASH, address>>16, address&0xFFFF, bytes)
        except usb.core.USBError:
            print "Unable to send WRITE_FLASH vendor request." 

    def erase_flash(self, address):
        try:
            self.dev.ctrl_transfer(0x40, self.ERASE_FLASH, address>>16, address&0xFFFF)
        except usb.core.USBError:
            print "Unable to send ERASE_FLASH vendor request."

    def start_user(self):
        try:
            self.dev.ctrl_transfer(0x40, self.START_USER, 0, 2)
        except usb.core.USBError:
            print "Unable to send START_USER vendor request."
