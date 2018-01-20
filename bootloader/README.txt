The host software that interfaces with the Elecanisms bootloader is written 
in Python and makes use of the pyusb module with libusb-1.0 as the underlying 
low-level backend.  To successfully run this software to interface with 
the device, you will need to make sure that you have all of the relevant 
software packages intalled and configured on your computer.  You should be able 
to talk to the bootloader from Windows, Mac OSX, or Linux.  The installation 
process is a little different in each case, as outlined below.

Installation Under Windows
--------------------------
1. Check to make sure that you have a Python (v. 2.6.x or 2.7.x) 
   installation.  On a Windows machine, these will typically be 
   located in C:\Python26 or C:\Python27.  If you do not have a 
   Python installation, you should download the latest version of 
   the Windows Python 2.7 installer (you probably want the 64-bit
   version) from http://www.python.org and install it.

2. Check to make sure that Python is in your search path.  To do so 
   on a Windows machine, go to the start menu, and in the search/run 
   box, type cmd to launch a command shell.  Type python at the prompt 
   and hit return.  If the interactive Python shell runs, then the 
   Python directory is already on your path.  If you get a message like 

      'python' is not recognized as an internal or external command,
      operable program or batch file.

   you will need to add C:\Python27 or C:\Python26 to your Path environment 
   variable.  To do this in Windows, right click on the icon representing 
   your computer and select the "Properties" menu option.  Click on "Advanced 
   system settings."  That should bring up the "System Properties" dialog.  
   Near the bottom of the dialog, click on the "Environment Variables" button, 
   which will bring up the "Environment Vairables" dialog box.  In the "System 
   Variables" list box in the bottom half of this dialog, find the "Path" 
   variable.  Select it and click the "Edit" button.  The entire contents of 
   the value of the variable will be selected in an "Edit System Variable" 
   dialog (don't press the "Delete" key!).  Press the right arrow to move the 
   isertion point to the end of the current value of the Path variable and 
   type ;C:\Python27 or ;C:\Python26 to add whichever is appropriate to your 
   particular Python installation to the end of your Path variable and click
   on the "OK" button.  Close all of the dialogs and the cmd window.  Relaunch 
   cmd and try typing "python" at the command prompt again.  If you succeeded 
   in adding the right directory to the Path, the interactive command shell 
   should appear.

3. Obtain the latest version of pyusb from 

       http://sourceforge.net/projects/pyusb

   It will come in a zip archive.  Unzip the archive somewhere convenient.  
   Toi install pyusb, launch a command shell and navigate to the pyusb directory 
   using the cd command.  From that directory, you should be able to install 
   pyusb by running "python setup.py install" at the command prompt.

4. Unzip the Elecanisms bootloader zip archive to some convenient location on 
   your computer.  In it, you will find a DLL file (libusb-1.0.dll), and three 
   subdirectories: driver, firmware, and software.  You will need to copy the 
   libusb-1.0.dll file to C:\Windows, which is by default a hidden directory 
   and requires Administrator privilages to do.  This will install the 
   libusb-1.0 backend so that pyusb can find it.

5. When you plug the Elecanisms board into a USB port on your computer for the 
   first time with SW1 depressed, which runs the bootloader, Windows will try 
   to find a driver to install for the bootloader device.  It will fail to 
   locate an appropriate driver.  To install the driver, bring up your Windows 
   device manager.  Click on the device called "PIC24FJ USB Bootloader" and 
   select "Update Driver".  In the dialog that appears, click on the option 
   that allows you to manually search for a driver.  Browse to the "driver" 
   subdirectory that came with the imulogger software, check the box that says 
   "include subdirectories" and click on the "Search" button.  Another dialog 
   will appear warning you that the driver has not been signed and asking 
   whether you want to proceed with the installation anyway.  Go ahead with 
   the installation.

6. To run the bootloader host software, which is located in the "software" 
   subdirectory, you can either type "python bootloadergui.py" at the command 
   prompt (assuming you are in the directory that contains the file) or you 
   should also be able to double click on the bootloadergui.py icon in a file 
   explorer window.  There is also a command-line application called 
   "bootloadercmd.py"; to learn how to use it, run it with the -h or --help 
   command-line argument.

Installation Under Mac OSX
--------------------------
1. Obtain the latest version of pyusb from 

       http://sourceforge.net/projects/pyusb

   It will come in a zip archive.  Unzip the archive somewhere convenient.  
   Toi install pyusb, launch a command shell and navigate to the pyusb directory 
   using the cd command.  From that directory, you should be able to install 
   pyusb by running "python setup.py install" at the command prompt.

2. Obtain the source code for the latest version of libusb-1.0 from

       https://github.com/libusb/libusb

   and build it.  If you have the Xcode command-line tools installed, this 
   should be as simple as entering the top-level directory containing the 
   source code and performing the typical sequence

       ./configure
       make
       sudo make install

   in a terminal window.

3. To run the bootloader host software, which is located in the "software" 
   subdirectory of the bootloader zip archive, type "python bootloadergui.py" 
   at the command prompt.  There is also a command-line application called 
   "bootloadercmd.py"; to learn how to use it, run it with the -h or --help 
   command-line argument.

Installation Under Linux
------------------------
1. Obtain the latest version of pyusb from 

       http://sourceforge.net/projects/pyusb

   It will come in a zip archive.  Unzip the archive somewhere convenient.  
   Toi install pyusb, launch a command shell and navigate to the pyusb directory 
   using the cd command.  From that directory, you should be able to install 
   pyusb by running "python setup.py install" at the command prompt.

2. At this point, you should be able to run the bootloader host software using 
   sudo.  Because our bootloader device is a custom, vendor-specific USB device, 
   Linux does not permit access to it for all users by default.  To allow 
   general access, create a file in /etc/udev/rules.d/ using your favorite text 
   editor called usb_prototype_devices.rules.  Note that you will probably need 
   to do this using sudo.  Type or paste the following text into the file:

# Prototype vendor specific devices
SUBSYSTEM=="usb", ATTRS{idVendor}=="6666", ATTRS{idProduct}=="4321", MODE=="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="6666", ATTRS{idProduct}=="0003", MODE=="0666"

3. To run the bootloader host software, which is located in the "software" 
   subdirectory of the bootloader zip archive, type "python bootloadergui.py" 
   at the command prompt.  There is also a command-line application called 
   "bootloadercmd.py"; to learn how to use it, run it with the -h or --help 
   command-line argument.
