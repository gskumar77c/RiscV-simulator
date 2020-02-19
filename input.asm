addi x10,x0,5
jal x1 fact
jal x0 exit

fact:
    addi x2 x2 -8
    sw x1 0(x2)
    sw x10 4(x2)
    addi x5 x0 2
    bge x10 x5 next_fact
    addi x10 x0 1
    jalr x0 0(x1)

    next_fact:
  addi x10 x10 -1
  jal x1 fact

    addi x2 x2 8
    lw x5 4(x2)
    lw x1 0(x2)
    mul x10 x10 x5
    jalr x0 0(x1)

exit: 
