Enable Firemail Module
======================
To enabel this module check following options in make menuconfig:

-------------------------
Applications
    -> [X] Firemail
-------------------------

But first you have do load information for the board you are
using:

-------------------------
Load Default Configuration
    -> [X] Pollin AVR Net-IO
-------------------------

Before you can enable the firemail module, make sure the 
following options/modules are checked:

-------------------------
General Setup
    -> [X] Ethernet (ENC28J60) support
    -> Gateway (the IP of your local/global Gateway)

Network Protocols
    -> [X] TCP support
    -> [X] UDP support
    -> [X] DNS support (the IP of your local/global DNS server)

Applications
    -> [X] ECMD (Etherrape Control Interface) support
-------------------------

Debug Output
============
To enable debug output, you must first enable following options:

-------------------------
General
    -> [X] Enable (Serial Line) Debugging
-------------------------

FAQ - Frequently asked Questions
================================
Image too big? 
Lots of modules can be disabled:

-------------------------
I/O Support
    -> [ ] ADC input
    -> [ ] Onewire support

Applications
    -> [ ] HTTP Server
-------------------------

How to get the Debug output?
Connect the serial output of the Atmega to a PC and start a terminal application like
 * HTerm (Windows)
 * minicom (Console)
Connection settings should be 115200 8N1

