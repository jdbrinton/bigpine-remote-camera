# Setting Up the Development Environment

## Configure GitLab

### Generate SSH keys

```bash
ssh-keygen -t ed25519 -C "your.email@aware.us"
```

### Copy the public key to the clipboard

#### Linux

```bash
xclip -sel clip < ~/.ssh/id_ed25519.pub
```

#### Windows
```bat
type %userprofile%\.ssh\id_ed25519.pub |clip
```

### Import the public key to GitLab

Add your public SSH key to your GitLab account by clicking your **avatar** in the upper right corner and selecting **Settings**. From there on, navigate to **SSH Keys** and paste your public key in the “Key” section. If you created the key with a comment, this will appear under “Title”. If not, give your key an identifiable title like **Aware Dell XPS 15 Laptop**, and click **Add key**.

## ARM MBED Toolchain

### Prerequisites

#### Windows

- Install [Tera Term](https://osdn.net/projects/ttssh2/releases/) for serial terminal.
- Install [LLVM](http://releases.llvm.org/download.html) to use clangt for IDE language support. 

#### Linux

On Linux install: git, mercurial (hg), python 2.7, screen and clang using your favorite package manager.

Download [GNU ARM Embedded Compiler 6-2017 Q2 update](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads/6-2017-q2-update) and extract it to the /opt folder. Add the bin folder to your PATH in your .profile.

### Install ARM MBED CLI

Follow [the installation instructions](https://os.mbed.com/docs/v5.11/tools/installation-and-setup.html) for your platform to setup ARM MBED CLI.

### Clone the source code

Clone the sensor-firmware source code. You can do this directly with the MBED CLI

```bash
mbed import git@gitlab.com:aware-sensing/sensor-firmware.git
```

This will checkout the sensor-firmware project using git. Now configure the project to use GCC toolchain and NUCLEO F767ZI board.

```bash
cd sensor-firmware
mbed target NUCLEO_F767ZI
mbed toolchain GCC_ARM
```

**Note:** You use ``mbed import`` instead of ``git clone`` to clone an ARM MBED project from a repository to your computer. If you use ``git clone`` instead, you will have to run ``mbed deploy`` to download dependencies and ``mbed new .`` in the project folder.  

### Compile Sensor Firmware Application

You should now be ready to compile the sensor-firmware application. To compile execute the following command:

```bash
mbed compile
```

This will compile the application and create a BUILD folder under which the firmware image is written to. The firmware image name displayed at the end of the compilation output.

Now connect your development board to your computer using a USB cable. If the board debug firmware has not been updated to use J-Link (see below), the board opens up as an external USB drive. You can now drag and drop the generated firmware image file (.bin) to the USB drive. The firmware will be flashed to the microcontroller and the board with be reset. If everything went well, you should now see the example firmware running on the board that will blink one of the LEDs.

Remove the generated ``BUILD`` folder at the end to avoid confusion in the future as we are going to set up the BUILD from Visual Studio Code to run a little differently.

## Setup JTAG Emulator

For programming and debugging the microcontroller boards we use J-Link JTAG emulator.

### Installing J-Link Software

To use J-Link you need to install the J-Link Software Package on your computer. You can get one for your system [here](https://www.segger.com/downloads/jlink).

On Windows J-Link software is not automatically added to the PATH, you have to do this manually. Open **Control Panel\System and Security\System** and select **Advanced system settings** from the side bar. Click on **Environment Variables...** button and add the J-Link installation folder to either user or system path. For the current version of the J-Link software package the installation folder is ``C:\Program Files (x86)\SEGGER\JLink_V640``.

### J-Link Debugging for Custom PCB

At the moment we're using the [Segger J-Link JTAG emulator](https://www.digikey.com/product-detail/en/segger-microcontroller-systems/8.08.90-J-LINK-EDU/899-1008-ND/2263130) for debugging and programming our custom PCB boards. If you are using Nucle development board, you don't need Segger J-Link JTAG emulator as the development board has built-in JTAG emulator. See below for details.

### J-Link Debugging for STM32F Nucleo Development Board

If you find yourself using an STM32F Nucleo development board such as Nucleo F767ZI, you can convert the built-in ST-Link debugger to a J-Link compatible debugger using [this process](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/).

After the debug firmware conversion, the board will no longer work as a STLink board and the firmware can no longer be flashed by dragging and dropping it from the Windows explorer.

### Restoring STLink Firmware onto STM32F Nucleo Development Board

Skip this step unless you want to revert your board to use STLink again. 

Once you have replaced the debug firmware with J-Link compatible firmware, you can restore the firmware back to STLink based firmware, if you at any point want to use STLink again. To restore the STLink debug firmware, use the same [tool](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/) that you used in converting the debug firmware to be J-Link compatible. This restores a STLink compatible debug firmware to the board, however it's not the firmware version we want to use. After reconverting the board back to STLink using the above mentioned tool, reflash the latest Nucleo board firmware to the drive. The latest firmware for Nucleo F767ZI can be found [here](https://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-nucleo/nucleo-f767zi.html#sw-tools-scroll). Use the tool you downloaded to flash the latest STLink firmware to the probe portion of your development board.

## Setup Visual Studio Code

### Install VS Code

Download [Visual Studio Code](https://code.visualstudio.com/download) for your platform and install it for your computer.

### Install VS Code Extensions

Install the following add-ons (if not installed):
- [C/C++ for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [Cortex Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)
- [ARM assembly highlighting](https://marketplace.visualstudio.com/items?itemName=dan-c-underwood.arm)

### Open Sensor Firmware in VS Code

Open VS Code and open sensor-firmware folder from the IDE or open terminal and type: 

```bash
cd sensor-firmware
code .
```

to open the project in VS Code.

### Build The Sensor-Firmware Application

The **Terminal > Run Task...** menu item should show options *Debug*, *Develop* and *Release*. You can build the project with **Terminal > Run Build Task...** to run the default Debug build or build a specific type of release by **Terminal > Run Task...** and selecting the kind of build you want.

The builds will be located in *\BUILD\{build-type} folder*

### Debug The Sensor-Firmware Application

You can deploy the different builds of the example application to your development board from the *Debug* view of VS Code. To show the Debug view, click on the bug icon on the left hand side menu bar. In the **Debug** view select the build type from the drop down menu on top-left and click the **Run** arrow or F5 to start debugging. To start without debugging click Ctrl + F5 (not sure how this is actually different if at all). 

**Note:** You have to build the build before starting the debug session, VS Code doesn't automatically build the software.

## Deploy from Command Line

You can deploy a build from command line using the ``JLink`` tool. You should use this when deploying the firmware to production version.

```bash
JLink
```

This opens JLink command prompt.


Give command connect to connect to the device.

```
J-Link>connect
```

Select default device type by pressing enter.

```
Please specify device / core. <Default>: STM32F767ZI
Type '?' for selection dialog
Device>
```

Select SWD interface by entering ``S``

```
Please specify target interface:
  J) JTAG (Default)
  S) SWD
  T) cJTAG
TIF>S
```

Select default speed by pressing enter.

```
Specify target interface speed [kHz]. <Default>: 4000 kHz
Speed>
```

Upload firmware using loadbin command

```
J-Link>loadbin sensor-firmware.bin, 0x08000000
```

You can exit the tool by typing exit

```
J-Link>exit
```

