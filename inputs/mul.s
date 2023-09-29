    .text
main:
    addi $t0, $zero, 7
    addi $t1, $zero, 9
    mult $t0, $t1     
    mflo $t2           
    addiu $v0, $zero, 10    
    syscall