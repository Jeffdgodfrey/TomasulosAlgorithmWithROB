*****************************************************
**************** TOMASULO'S ALGORITHM ***************
***************** WITH REORDER BUFFER ***************
*****************************************************
                Author : Jeff D Godfrey
-----------------------------------------------------

SPECIFICATIONS:
This program simulates Tomasulo's Algorithm. It 
includes an instruction queue with a maximum size of
ten instructions, five reservation stations (three for
add/sub instructions, two for multiply/divide instru-
ctions), two arithmetic units (one for add/sub, one
for multiply/divide), a register allocation table with
eight slots, a register file with eight slots, and a
reorder buffer with six slots.

PRECAUTIONS:
This simulation will only allow ten instructions in a
single run. It also will take the first (sequentially)
reservation station as priority (i.e., add/sub reser-
vation station one will always be run before two and
three and multiply/divide reservation station four will
always be run before five).

INSTRUCTIONS:
First, place a file named "Input.txt" in the same
folder as the program. This file will contain the inst-
ructions to be run in the program, and all input must
be whitespace delimited. In the first line, place the
number of instructions. In the second line, place the 
number of clock cycles to run the program for. The
following lines will contain the instructions (accor-
ding to how many instructions you indicated). 
Instructions have four components, of the form 
(operation destination source_1 source_2). In the 
next eight lines, place the initial values for the 
registers (R0-R7). Finally, open the project in 
Code::Blocks and click "Build and Run." The output
will open in a terminal, and will show the data flow
of the processor for every cycle.

NOTE:
If an entry contains all integer values of (-1) and 
all boolean values of 0 (false), it is considered
empty.

