## Miniproject 0

**Alex Hoppe Elecanisms SP 2018**

Given an Elecanisms board, in order to get my code running a user would need to configure the build environment, clone my repository, compile my source code, and flash it onto the board. Also a simple circuit must be constructed on a breadboard. The first step is to configure the environment.

### Configuring the environment
1. Follow the installation instructions for SConstruct and MPLABX detailed in the setup guide [here](http://elecanisms.olin.edu/handouts/1.1_BuildTools.pdf).

### Cloning the Repository
2. Clone [my repository](www.github.com/aehoppe/elecanisms2018) into a local folder of your choice. For the purposes of these instructions, this folder will be referred to as `<dir>`.

### Compiling Source Code
3. Open a terminal and navigate to `<dir>/blinkhoppe/`.
4. If in a Linux environment, edit the first line of the `SConstruct` file, changing
```
env.PrependENVPath('PATH', '/Applications/microchip/xc16/v1.33/bin')
```
to
```
env.PrependENVPath('PATH', '/opt/microchip/xc16/v1.33/bin')
```
to reflect the default install location of MPLABX.
5. Type `scons` to run SConstruct and build the source code.

### Flashing Binary onto Hardware
6. Connect the Elecanisms board to your laptop while holding down the button marked **SW1** to enter bootloader mode.
7. Type `python <dir>/bootloader/software/bootloadercmd.py -i blink.hex -w`
 to flash the code in `blink.hex` onto the board. The command line output should resemble the following:
 ```
 Connected to a PIC24FJ USB bootloader device (PIC24FJ128GB206).
Importing hex file 'blink.hex' to flash buffer.
Erasing program memory...
[##############################]  99%
Writing program memory...
[##############################]  99%
Verifying program memory...
[##############################]  99%
Verification succeeded.
Write completed successfully.
```

For efficiency, consider adding `alias btld="python <dir>/bootloader/software/bootloadercmd.py"` to your `.bashrc`, so that the bootloader command line utility can be accessed from any folder in the repository by the shortcut `btld`.

8. Unplug the Elecanisms board from the computer.

### Building the Breadboard Circuit
9. Connect one side of a tactile switch to the ground rail of a breadboard, and connect the other to power with a 1k pull-up resistor
10. Connect a jumper between the pulled-up node and pin **D9** on the Elecanisms board.
11. Use jumpers to connect power and ground rails on the breadboard to the Elecanisms board.
12. Plug the Elecanisms board back into a laptop or other power source, and use the button to switch between blinking the red LED and the green LED on the Elecanisms board. 
