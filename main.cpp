#include <iostream>
#include <fstream>
#include <queue>
#include <stdlib.h>

using namespace std;

//Tomasulo's Data Structures

struct arithmetic_unit{     //These units will be used for add/sub and mul/div operations
    bool busy;  //Is unit busy?
    int opcode; //What operation?
    int rd;     //Destination?
    int rs;     //Source 1?
    int rt;     //Source 2?
    int result;     //Result of operation
    bool div_by_zero;       //Was there a divide by zero exception?
    int latency_counter;    //How many clock cycles unit is busy for
};

struct instruction_record{      //Will be used to store instructions in a queue
    int opcode;     //What operation?
    int rd;     //Destination?
    int rs;     //Source 1?
    int rt;     //Source 2?
};

struct reservation_station{     //Will be used for add/sub and mul/div reservation stations
    int opcode;     //What operation?
    int value_1;        //If RS points directly to RF, fill these
    int value_2;
    int tag_1;          //If RS points to RAT, fill these
    int tag_2;
    int dest;       //Where will result end up?
    bool busy;      //Is RS ready for use?
    bool dispatch;      //Is RS ready to dispatch?
};

struct reorder_buffer_entry{    //Will be used for ROB entries
    int opcode;
    int reg;
    int val;
    bool done;
    bool commit;
    bool div_zero_exception;
};

struct reorder_buffer{      //ROB with 6 entries, an issue pointer, and a commit pointer
    reorder_buffer_entry ROB[6];
    int issue;
    int commit;
};

reservation_station add_sub_reservation_station[3];     //Add/sub reservation stations

reservation_station mul_div_reservation_station[2];     //Mul/div reservation stations

int RF[8];      //Register files R0-R7

int RAT[8];     //Register allocation table, RAT0-RAT7, corresponding to register files

queue <instruction_record> instruction_queue;       //Queue used to store instruction records

reorder_buffer ROB;

arithmetic_unit adder;
arithmetic_unit multiplier;

//End Tomasulo's Data Structures

//Tomasulo's Global Variables

int number_of_instructions;     //Stores total number of instructions
int number_of_cycles;       //Stores total number of clock cycles

//End Tomasulo's Global Variables

//Tomasulo's Methods
void data_flow_test()
{
    cout << "\n*****RESERVATION STATIONS*****" << endl;
    for(int i = 0; i < 3; i++)
    {
        cout
            << "RS" << i+1 << ": "
            << " Opcode: " << add_sub_reservation_station[i].opcode
            << " Dest: " << add_sub_reservation_station[i].dest
            << " Value_1: " << add_sub_reservation_station[i].value_1
            << " Value_2: " << add_sub_reservation_station[i].value_2
            << " Tag_1: " << add_sub_reservation_station[i].tag_1
            << " Tag_2: " << add_sub_reservation_station[i].tag_2
            << " Busy: " << add_sub_reservation_station[i].busy
            << " Dispatch: " << add_sub_reservation_station[i].dispatch
            << endl;
    }
    for(int i = 0; i < 2; i++)
    {
        cout
            << "RS" << i+4 << ": "
            << " Opcode: " << mul_div_reservation_station[i].opcode
            << " Dest: " << mul_div_reservation_station[i].dest
            << " Value_1: " << mul_div_reservation_station[i].value_1
            << " Value_2: " << mul_div_reservation_station[i].value_2
            << " Tag_1: " << mul_div_reservation_station[i].tag_1
            << " Tag_2: " << mul_div_reservation_station[i].tag_2
            << " Busy: " << mul_div_reservation_station[i].busy
            << " Dispatch: " << mul_div_reservation_station[i].dispatch
            << endl;
    }

    cout << "\n*****REORDER BUFFER*****" << endl;
    for(int i = 0; i < 6; i++)
    {
        cout
            << "ROB" << i+1 << ": "
            << " Type: " << ROB.ROB[i].opcode
            << " REG: " << ROB.ROB[i].reg
            << " Value: " << ROB.ROB[i].val
            << " Done: " << ROB.ROB[i].done
            << " Exception: " << ROB.ROB[i].div_zero_exception
            << endl;
    }
    cout
        << "\t Issue Pointer: " << ROB.issue
        << " Commit Pointer: " << ROB.commit
        << endl;

    cout << "\n*****REGISTER ALLOCATION TABLE*****" << endl;
    for(int i = 0; i < 8 ; i++)
    {
        cout
            << "RAT" << i << ": "
            << RAT[i]
            << endl;
    }

    cout << "\n*****REGISTER FILE*****" << endl;
    for(int i = 0; i < 8; i++)
    {
        cout
            << "RF" << i << ": "
            << RF[i]
            << endl;
    }

    cout
        << "\n*****ADDER UNIT CONTENTS*****"
        << "\nROB DESTINATION: " << adder.rd
        << " TYPE: " << adder.opcode
        << " SOURCE_1: " << adder.rs
        << " SOURCE_2: " << adder.rt
        << " VALUE: " << adder.result
        << " LATENCY COUNTER: " << adder.latency_counter
        << " BUSY: " << adder.busy
        << endl;

    cout
        << "\n*****MULTIPLIER UNIT CONTENTS*****"
        << "\nROB DESTINATION: " << multiplier.rd
        << " TYPE: " << multiplier.opcode
        << " SOURCE_1: " << multiplier.rs
        << " SOURCE_2: " << multiplier.rt
        << " VALUE: " << multiplier.result
        << " LATENCY COUNTER: " << multiplier.latency_counter
        << " BUSY: " << multiplier.busy
        << " EXCEPTION: " << multiplier.div_by_zero
        << endl;

    cout << "\n\n" << endl;
}

bool populate()
{
    ifstream File("Input.txt");

    File >> number_of_instructions;     //Upload number of instructions

    if(number_of_instructions > 10)     //If number of instructions exceeds maximum size, return false
    {
        cout << "Number of instructions exceeds maximum size!!!" << endl;
        return false;
    }

    File >> number_of_cycles;       //Upload number of clock cycles

    for(int i = 0; i < number_of_instructions; i++)     //Populate instruction queue with instructions
    {
        instruction_record instruction;
        File
            >> instruction.opcode
            >> instruction.rd
            >> instruction.rs
            >> instruction.rt;
        instruction_queue.push(instruction);
    }

    for(int i = 0; i < 8; i++)      //Upload RF files
    {
        File >> RF[i];
    }

    for(int i = 0; i < 8; i++)      //Initialize RAT; value of -1 indicates RAT points to RF, otherwise points to RS
    {
        RAT[i] = (-1);
    }

    for(int i = 0; i < 3; i++)      //Initialize add/sub reservation stations to available, no operations (-1)
    {
        add_sub_reservation_station[i].opcode = (-1);
        add_sub_reservation_station[i].busy = false;
        add_sub_reservation_station[i].dispatch = false;
        add_sub_reservation_station[i].dest = (-1);
        add_sub_reservation_station[i].value_1 = (-1);
        add_sub_reservation_station[i].value_2 = (-1);
        add_sub_reservation_station[i].tag_1 = (-1);
        add_sub_reservation_station[i].tag_2 = (-1);
    }

    for(int i = 0; i < 2; i++)      //Initialize mul/div reservation stations to available, no operations (-1)
    {
        mul_div_reservation_station[i].opcode = (-1);
        mul_div_reservation_station[i].busy = false;
        mul_div_reservation_station[i].dispatch = false;
        mul_div_reservation_station[i].dest = (-1);
        mul_div_reservation_station[i].value_1 = (-1);
        mul_div_reservation_station[i].value_2 = (-1);
        mul_div_reservation_station[i].tag_1 = (-1);
        mul_div_reservation_station[i].tag_2 = (-1);
    }

    for(int i = 0; i < 6; i++)      //Initialize ROB to not done, no divide by zero exceptions
    {
        ROB.ROB[i].div_zero_exception = false;
        ROB.ROB[i].done = false;
        ROB.ROB[i].commit = false;
        ROB.ROB[i].reg = (-1);
        ROB.ROB[i].opcode = (-1);
        ROB.ROB[i].val = (-1);
    }

    ROB.issue = 0;       //Set issue and commit pointers to beginning positions
    ROB.commit = 0;

    adder.latency_counter = (-1);   //Initialize adder
    adder.busy = false;
    adder.div_by_zero = false;
    adder.opcode = (-1);
    adder.rd = (-1);
    adder.result = (-1);
    adder.rs = (-1);
    adder.rt = (-1);

    multiplier.latency_counter = (-1);
    multiplier.busy = false;
    multiplier.div_by_zero = false;
    multiplier.opcode = (-1);
    multiplier.rd = (-1);
    multiplier.result = (-1);
    multiplier.rs = (-1);
    multiplier.rt = (-1);

    return true;        //Population successful
}

void issue_stage()
{
    if(!instruction_queue.empty() && ROB.ROB[ROB.issue].opcode == (-1))
    {
        if(instruction_queue.front().opcode == 0 || instruction_queue.front().opcode == 1)      //If instruction is add/sub
        {
            for(int i = 0; i < 3; i++)      //Cycle through add/sub RS
            {
                if(add_sub_reservation_station[i].busy == false)
                {
                    //Update ROB entry
                    ROB.ROB[ROB.issue].opcode = instruction_queue.front().opcode;
                    ROB.ROB[ROB.issue].reg = instruction_queue.front().rd;

                    //Update RS
                    add_sub_reservation_station[i].opcode = instruction_queue.front().opcode;   //Update RS opcode
                    add_sub_reservation_station[i].dest = ROB.issue;     //Update RS destination to ROB entry
                    add_sub_reservation_station[i].busy = true;     //Mark RS unit as busy

                    if(RAT[instruction_queue.front().rs] == -1)     //If RAT points to RF populate value_1 with RF value
                        add_sub_reservation_station[i].value_1 = RF[instruction_queue.front().rs];
                    else        //If RAT doesn't point to RF, point tag_1 to ROB
                        add_sub_reservation_station[i].tag_1 = RAT[instruction_queue.front().rs];

                    if(RAT[instruction_queue.front().rt] == -1)     //If RAT points to RF populate value_2 with RF value
                        add_sub_reservation_station[i].value_2 = RF[instruction_queue.front().rt];
                    else        //If RAT doesn't point to RF, point tag_2 to ROB
                        add_sub_reservation_station[i].tag_2 = RAT[instruction_queue.front().rt];

                    //Update RAT to point to ROB
                    RAT[instruction_queue.front().rd] = ROB.issue;

                    //Update ROB issue pointer (if next available ROB slot is open)
                    if(ROB.issue < 5)       //If ROB isn't at max position
                    {
                        if(ROB.ROB[(ROB.issue+1)].opcode == (-1))
                        {
                            ROB.issue++;
                            instruction_queue.pop();
                            return;
                        }
                    }

                    if(ROB.issue == 5 && ROB.ROB[0].opcode == (-1))     //If ROB is at max position
                    {
                        ROB.issue = 0;
                        instruction_queue.pop();
                        return;
                    }
                }
            }
        }

        if(instruction_queue.front().opcode == 2 || instruction_queue.front().opcode == 3)      //If instruction is mul/div
        {
            for(int i = 0; i < 2; i++)      //Cycle through mul/div RS
            {
                if(mul_div_reservation_station[i].busy == false)
                {
                    //Update ROB entry
                    ROB.ROB[ROB.issue].opcode = instruction_queue.front().opcode;
                    ROB.ROB[ROB.issue].reg = instruction_queue.front().rd;

                    //Update RS
                    mul_div_reservation_station[i].opcode = instruction_queue.front().opcode;   //Update RS opcode
                    mul_div_reservation_station[i].dest = ROB.issue;     //Update RS destination to ROB entry
                    mul_div_reservation_station[i].busy = true;     //Mark RS unit as busy

                    if(RAT[instruction_queue.front().rs] == -1)     //If RAT points to RF populate value_1 with RF value
                        mul_div_reservation_station[i].value_1 = RF[instruction_queue.front().rs];
                    else        //If RAT doesn't point to RF, point tag_1 to ROB
                        mul_div_reservation_station[i].tag_1 = RAT[instruction_queue.front().rs];

                    if(RAT[instruction_queue.front().rt] == -1)     //If RAT points to RF populate value_2 with RF value
                        mul_div_reservation_station[i].value_2 = RF[instruction_queue.front().rt];
                    else        //If RAT doesn't point to RF, point tag_2 to ROB
                        mul_div_reservation_station[i].tag_2 = RAT[instruction_queue.front().rt];

                    //Update RAT to point to ROB
                    RAT[instruction_queue.front().rd] = ROB.issue;

                   //Update ROB issue pointer (if next available ROB slot is open)
                    if(ROB.issue < 5)       //If ROB isn't at max position
                    {
                        if(ROB.ROB[(ROB.issue+1)].opcode == (-1))
                        {
                            ROB.issue++;
                            instruction_queue.pop();
                            return;
                        }
                    }

                    if(ROB.issue == 5 && ROB.ROB[0].opcode == (-1))     //If ROB is at max position
                    {
                        ROB.issue = 0;
                        instruction_queue.pop();
                        return;
                    }
                }
            }
        }
    }
    return;
}

void dispatch_stage()
{
    for(int i = 0; i < 2; i++)
    {
        if(mul_div_reservation_station[i].busy == true && mul_div_reservation_station[i].dispatch == true
           && multiplier.busy == false)  //If mul/div RS is ready and multiplier unit available
        {
            if(mul_div_reservation_station[i].opcode == 2)      //If multiply
            {
                //Initialize multiplier unit with multiply instruction
                multiplier.busy = true;
                multiplier.latency_counter = 10;
                multiplier.opcode = 2;
                multiplier.rd = mul_div_reservation_station[i].dest;
                multiplier.rs = mul_div_reservation_station[i].value_1;
                multiplier.rt = mul_div_reservation_station[i].value_2;
                multiplier.result = multiplier.rs * multiplier.rt;      //Do operation

                //Free RS
                mul_div_reservation_station[i].busy = false;
                mul_div_reservation_station[i].dest = (-1);
                mul_div_reservation_station[i].dispatch = false;
                mul_div_reservation_station[i].opcode = (-1);
                mul_div_reservation_station[i].tag_1 = (-1);
                mul_div_reservation_station[i].tag_2 = (-1);
                mul_div_reservation_station[i].value_1 = (-1);
                mul_div_reservation_station[i].value_2 = (-1);

                break;     //Break out of multiplier unit loop
            }

            if(mul_div_reservation_station[i].opcode == 3)      //If divide
            {
                //Initialize multiplier unit with divide instruction
                multiplier.busy = true;
                multiplier.opcode = 3;
                multiplier.rd = mul_div_reservation_station[i].dest;
                multiplier.rs = mul_div_reservation_station[i].value_1;
                multiplier.rt = mul_div_reservation_station[i].value_2;
                if(multiplier.rt == 0)      //If divide by zero exception
                {
                    multiplier.div_by_zero = true;
                    multiplier.result = 0;
                    multiplier.latency_counter = 38;        //Exception taken care of at 38 cycles
                }
                else
                {
                    multiplier.result = multiplier.rs / multiplier.rt;      //Do operation
                    multiplier.latency_counter = 40;
                }

                 //Free RS
                mul_div_reservation_station[i].busy = false;
                mul_div_reservation_station[i].dest = (-1);
                mul_div_reservation_station[i].dispatch = false;
                mul_div_reservation_station[i].opcode = (-1);
                mul_div_reservation_station[i].tag_1 = (-1);
                mul_div_reservation_station[i].tag_2 = (-1);
                mul_div_reservation_station[i].value_1 = (-1);
                mul_div_reservation_station[i].value_2 = (-1);

                break;      //Break out of multiplier loop
            }
        }
    }

    for(int i = 0; i < 3; i++)
    {
        if(add_sub_reservation_station[i].busy == true && add_sub_reservation_station[i].dispatch == true
           && adder.busy == false) //If add/sub RS ready and adder unit available
        {
            //Initialize adder unit
            adder.busy = true;
            adder.latency_counter = 2;
            adder.rd = add_sub_reservation_station[i].dest;
            adder.rs = add_sub_reservation_station[i].value_1;
            adder.rt = add_sub_reservation_station[i].value_2;
            if(add_sub_reservation_station[i].opcode = 0)
            {
                adder.opcode = 0;
                adder.result = adder.rs + adder.rt;     //Do operation
            }
            else
            {
                adder.opcode = 1;
                adder.result = adder.rs - adder.rt;     //Do operation
            }

            //Free RS
            add_sub_reservation_station[i].busy = false;
            add_sub_reservation_station[i].dest = (-1);
            add_sub_reservation_station[i].dispatch = false;
            add_sub_reservation_station[i].opcode = (-1);
            add_sub_reservation_station[i].tag_1 = (-1);
            add_sub_reservation_station[i].tag_2 = (-1);
            add_sub_reservation_station[i].value_1 = (-1);
            add_sub_reservation_station[i].value_2 = (-1);

            break;      //Break out of adder loop
        }
    }
}

void rs_dispatch_check()
{
    for(int i = 0; i < 3; i++)      //Check add/sub RS for dispatch readiness
    {
        //If add/sub RS is occupied, not ready for dispatch, and does not contain any tags
        if(add_sub_reservation_station[i].busy == true && add_sub_reservation_station[i].dispatch == false
           && add_sub_reservation_station[i].tag_1 == (-1) && add_sub_reservation_station[i].tag_2 == (-1))
        {
            add_sub_reservation_station[i].dispatch = true;     //Make station dispatch-ready
        }
    }

    for(int i = 0; i < 2; i++)      //Check mul/div RS for dispatch readiness
    {
        //If mul/div RS is occupied, not ready for dispatch, and does not contain any tags
        if(mul_div_reservation_station[i].busy == true && mul_div_reservation_station[i].dispatch == false
           && mul_div_reservation_station[i].tag_1 == (-1) && mul_div_reservation_station[i].tag_2 == (-1))
        {
            mul_div_reservation_station[i].dispatch = true;     //Make station dispatch-ready
        }
    }
    return;
}

void arithmetic_latency_check()
{
    if(adder.latency_counter > 0)
    {
        adder.latency_counter--;
    }

    if(multiplier.latency_counter > 0)
    {
        multiplier.latency_counter--;
    }
}

void broadcast_stage()
{
    //Handle multiplier first to account for single common data bus (for no divide by zero exception)
    if(multiplier.busy == true && multiplier.latency_counter == 0 && multiplier.div_by_zero == false)
    {
        //Write result to ROB
        ROB.ROB[multiplier.rd].val = multiplier.result;
        ROB.ROB[multiplier.rd].done = true;

        for(int i = 0; i < 3; i++)      //Capture result in add/sub reservation stations
        {
            if(add_sub_reservation_station[i].tag_1 == multiplier.rd)
            {
                add_sub_reservation_station[i].value_1 = multiplier.result;
                add_sub_reservation_station[i].tag_1 = (-1);
            }

            if(add_sub_reservation_station[i].tag_2 == multiplier.rd)
            {
                add_sub_reservation_station[i].value_2 = multiplier.result;
                add_sub_reservation_station[i].tag_2 = (-1);
            }
        }

        for(int i = 0; i < 2; i++)      //Capture result in mul/div reservation stations
        {
            if(mul_div_reservation_station[i].tag_1 == multiplier.rd)
            {
                mul_div_reservation_station[i].value_1 = multiplier.result;
                mul_div_reservation_station[i].tag_1 = (-1);
            }

            if(mul_div_reservation_station[i].tag_2 == multiplier.rd)
            {
                mul_div_reservation_station[i].value_2 = multiplier.result;
                mul_div_reservation_station[i].tag_2 = (-1);
            }

        }

        //Free multiplier unit
        multiplier.busy = false;
        multiplier.opcode = (-1);
        multiplier.latency_counter = (-1);
        multiplier.div_by_zero = false;
        multiplier.rd = (-1);
        multiplier.result = (-1);
        multiplier.rs = (-1);
        multiplier.rt = (-1);

        return;     //Break out of broadcast method
    }

    //Handle multiplier unit if divide by zero exception
    if(multiplier.busy == true && multiplier.latency_counter == 0 && multiplier.div_by_zero == true)
    {
        //Write divide by zero exception to ROB
        ROB.ROB[multiplier.rd].div_zero_exception = true;
        ROB.ROB[multiplier.rd].done = true;

        //Free multiplier unit
        multiplier.busy = false;
        multiplier.opcode = (-1);
        multiplier.latency_counter = (-1);
        multiplier.div_by_zero = false;
        multiplier.rd = (-1);
        multiplier.result = (-1);
        multiplier.rs = (-1);
        multiplier.rt = (-1);

        return;     //Break out of broadcast method
    }

    //Handle adder unit if common data bus not being used by multiplier unit
    if(adder.busy == true && adder.latency_counter == 0)
    {
        //Write result to ROB
        ROB.ROB[adder.rd].val = adder.result;
        ROB.ROB[adder.rd].done = true;

        for(int i = 0; i < 3; i++)      //Capture result in add/sub RS
        {
            if(add_sub_reservation_station[i].tag_1 == adder.rd)
            {
                add_sub_reservation_station[i].value_1 = adder.result;
                add_sub_reservation_station[i].tag_1 = (-1);
            }

            if(add_sub_reservation_station[i].tag_2 == adder.rd)
            {
                add_sub_reservation_station[i].value_2 = adder.result;
                add_sub_reservation_station[i].tag_2 = (-1);
            }
        }

        for(int i = 0; i < 2; i++)      //Capture result in mul/div RS
        {
            if(mul_div_reservation_station[i].tag_1 == adder.rd)
            {
                mul_div_reservation_station[i].value_1 = adder.result;
                mul_div_reservation_station[i].tag_1 = (-1);
            }

            if(mul_div_reservation_station[i].tag_2 == adder.rd)
            {
                mul_div_reservation_station[i].value_2 = adder.result;
                mul_div_reservation_station[i].tag_2 = (-1);
            }
        }

        //Free adder unit
        adder.busy = false;
        adder.latency_counter = (-1);
        adder.div_by_zero = false;
        adder.opcode = (-1);
        adder.rd = (-1);
        adder.result = (-1);
        adder.rs = (-1);
        adder.rt = (-1);
    }
}

void commit_stage()
{
    //First handle divide by zero exception
    if(ROB.ROB[ROB.commit].done == true && ROB.ROB[ROB.commit].div_zero_exception == true
       && ROB.ROB[ROB.commit].commit == true)
    {
        //Flush ROB
        for(int i = 0; i < 6; i++)
        {
            ROB.ROB[i].div_zero_exception = false;
            ROB.ROB[i].done = false;
            ROB.ROB[i].opcode = (-1);
            ROB.ROB[i].reg = (-1);
            ROB.ROB[i].val = (-1);
            ROB.ROB[i].commit = false;
        }
        ROB.commit = (-1);
        ROB.issue = (-1);

        //Point RAT to RF
        for(int i = 0; i < 8; i++)
        {
            RAT[i] = (-1);
        }

        //Flush add/sub RS
        for(int i = 0; i < 3; i++)
        {
            add_sub_reservation_station[i].busy = false;
            add_sub_reservation_station[i].dispatch = false;
            add_sub_reservation_station[i].dest = (-1);
            add_sub_reservation_station[i].opcode = (-1);
            add_sub_reservation_station[i].tag_1 = (-1);
            add_sub_reservation_station[i].tag_2 = (-1);
            add_sub_reservation_station[i].value_1 = (-1);
            add_sub_reservation_station[i].value_2 = (-1);
        }

        //Flush mul/div RS
        for(int i = 0; i < 2; i++)
        {
            mul_div_reservation_station[i].busy = false;
            mul_div_reservation_station[i].dispatch = false;
            mul_div_reservation_station[i].dest = (-1);
            mul_div_reservation_station[i].opcode = (-1);
            mul_div_reservation_station[i].tag_1 = (-1);
            mul_div_reservation_station[i].tag_2 = (-1);
            mul_div_reservation_station[i].value_1 = (-1);
            mul_div_reservation_station[i].value_2 = (-1);
        }

        cout << "\n*****DIVIDE BY ZERO EXCEPTION DETECTED!!!!!*****" << endl;
        cout << "\n\n*****FINAL STATE OF PROCESSOR*****" << endl;

        data_flow_test();
        exit(EXIT_FAILURE);
    }

    //If no divide by zero exception, write ROB value to RF and update RAT if applicable
    if(ROB.ROB[ROB.commit].done == true && ROB.ROB[ROB.commit].commit == true)
    {
        RF[ROB.ROB[ROB.commit].reg] = ROB.ROB[ROB.commit].val;

        //Update RAT if applicable
        if(RAT[ROB.ROB[ROB.commit].reg] == ROB.commit)
        {
            RAT[ROB.ROB[ROB.commit].reg] = (-1);
        }

        //Free ROB
        ROB.ROB[ROB.commit].done = false;
        ROB.ROB[ROB.commit].commit = false;
        ROB.ROB[ROB.commit].opcode = (-1);
        ROB.ROB[ROB.commit].reg = (-1);
        ROB.ROB[ROB.commit].val = (-1);
        ROB.ROB[ROB.commit].div_zero_exception = false;

        //Update commit pointer
        if(ROB.commit < 5)
        {
            ROB.commit++;
            return;
        }

        if(ROB.commit == 5)
        {
            ROB.commit = 0;
            return;
        }
    }
}

void issue_pointer_check()      //If issue pointer was not updated in previous cycle (ROB full)
{
    if(ROB.ROB[ROB.issue].opcode != (-1))
    {
        if(ROB.issue < 5)       //If issue pointer is not at max position
        {
            if(ROB.ROB[(ROB.issue + 1)].opcode == (-1))
            {
                ROB.issue++;
                return;
            }
        }

        if(ROB.issue == 5)      //If issue pointer is at max position
        {
            if(ROB.ROB[0].opcode == (-1))
            {
                ROB.issue = 0;
                return;
            }
        }
        return;
    }
}

void rob_commit_check()
{
    if(ROB.ROB[ROB.commit].done == true)
    {
        ROB.ROB[ROB.commit].commit = true;
    }
}

int main()
{
    //Initialize simulation according to "Input.txt". Exit simulation if instructions exceeds maximum of 10
    if(populate() == false)
    {
        cout << "Unable to complete simulation. Adjust number of instructions to < 10." << endl;
        return 0;
    }

    /* Testing instruction queue upload
    for(int i = 0; i < number_of_instructions; i++)
    {
        cout
            << "Instruction "
            << i+1
            << "  Opcode: "
            << instruction_queue.front().opcode
            << "  Dest: "
            << instruction_queue.front().rd
            << "  Source_1: "
            << instruction_queue.front().rs
            << "  Source_2: "
            << instruction_queue.front().rt
            << endl;
        instruction_queue.pop();
    }
    */

    /*Testing RF upload
    for(int i = 0; i < 8; i++)
    {
        cout << "RF" << i << ": " << RF[i] << endl;
    }
    */

    /*Testing RAT initialization
    for(int i = 0; i < 8; i++)
    {
        cout << "RAT" << i << ": " << RAT[i] << endl;
    }
    */

    /*Testing RS initialization
    for(int i = 0; i < 3; i++)
    {
        cout
            << "RS" << i+1 << ": "
            << " Opcode: " << add_sub_reservation_station[i].opcode
            << " Dest: " << add_sub_reservation_station[i].dest
            << " Value_1: " << add_sub_reservation_station[i].value_1
            << " Value_2: " << add_sub_reservation_station[i].value_2
            << " Tag_1: " << add_sub_reservation_station[i].tag_1
            << " Tag_2: " << add_sub_reservation_station[i].tag_2
            << " Busy: " << add_sub_reservation_station[i].busy
            << " Dispatch: " << add_sub_reservation_station[i].dispatch
            << endl;
    }
    for(int i = 0; i < 2; i++)
    {
        cout
            << "RS" << i+4 << ": "
            << " Opcode: " << mul_div_reservation_station[i].opcode
            << " Dest: " << mul_div_reservation_station[i].dest
            << " Value_1: " << mul_div_reservation_station[i].value_1
            << " Value_2: " << mul_div_reservation_station[i].value_2
            << " Tag_1: " << mul_div_reservation_station[i].tag_1
            << " Tag_2: " << mul_div_reservation_station[i].tag_2
            << " Busy: " << mul_div_reservation_station[i].busy
            << " Dispatch: " << mul_div_reservation_station[i].dispatch
            << endl;
    }
    */

    /*Testing ROB initialization
    for(int i = 0; i < 6; i++)
    {
        cout
            << "ROB" << i+1 << ": "
            << " Opcode: " << ROB.ROB[i].opcode
            << " Reg: " << ROB.ROB[i].reg
            << " Value: " << ROB.ROB[i].val
            << " Done: " << ROB.ROB[i].done
            << " Exception: " << ROB.ROB[i].div_zero_exception
            << endl;
    }

    cout
        << "\tIssue Pointer: " << ROB.issue
        << " Commit Pointer: " << ROB.commit
        << endl;
    */

    /* Issue Stage Test
    for(int i = 0; i < number_of_cycles; i++)
    {
        cout << "*****CYCLE #" << i+1 << "*****" << endl;
        issue_stage();
        data_flow_test();
    }
    */

    /*Dispatch Stage Test
    for(int i = 0; i < number_of_cycles; i++)
    {
        cout << "*****CYCLE #" << i+1 << "*****" << endl;
        arithmetic_latency_check();
        rs_dispatch_check();
        issue_stage();
        dispatch_stage();
        data_flow_test();
    }
    */

    /*Broadcast Stage Test
    for(int i = 0; i < number_of_cycles; i++)
    {
        cout << "*****CYCLE #" << i+1 << "*****" << endl;
        arithmetic_latency_check();
        rs_dispatch_check();
        issue_stage();
        dispatch_stage();
        broadcast_stage();
        data_flow_test();
    }
    */

    /*Commit Stage Test
    for(int i = 0; i < number_of_cycles; i++)
    {
        cout << "*****CYCLE #" << i+1 << "*****" << endl;
        arithmetic_latency_check();
        rs_dispatch_check();
        issue_stage();
        dispatch_stage();
        broadcast_stage();
        commit_stage();
        data_flow_test();
    }
    */

    //FINAL RESULT!!!
    for(int i = 0; i < number_of_cycles; i++)
    {
        cout << "*****CYCLE #" << i+1 << "*****" << endl;
        arithmetic_latency_check();
        rs_dispatch_check();
        issue_pointer_check();
        rob_commit_check();
        issue_stage();
        dispatch_stage();
        broadcast_stage();
        commit_stage();
        data_flow_test();
    }
}
