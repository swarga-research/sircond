
## Overview
Sircond is a management daemon that interfaces with a SiriusConnect
satellite radio receiver. SiriusConnect is a standard interface protocol 
"spoken" by Sirius satellite radio receivers which are capable of external
control (including "Sirius-Ready" car stereo units). Some SiriusConnect
receivers, such as the SCH2P, have an RS-232 serial port. Others, such as
the SC-H1, provide an 8-pin mini-DIN connector and require additional interface
circuitry to interface with a standard RS-232 or USB port. A third type of 
interface, the TTS-100 from TimeTrax, interfaces a SiriusConnect car stereo 
unit to a standard USB port. Sircond supports all of these variants.

In addition to allowing user control of a SiriusConnect radio, Sircond also 
acts as an arbiter of that control, allowing simultaneous "read-only" access by 
multiple network clients while granting "read/write" access to only one client
at a time. This allows multiple clients to receive metadata updates and status
information while preventing conflicting actions (e.g. one user changing channels 
while another user is recording a program). 

While intended for use on Linux systems, Sircond can also be compiled and run 
on Win32 machines.

