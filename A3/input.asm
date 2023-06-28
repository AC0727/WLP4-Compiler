threeRegisterOperands:
add   $1,  $2,  $3
sub   $4,  $5,  $6
slt   $7,  $8,  $9
sltu  $10, $11, $12
twoRegisterOperands:
mult  $13, $14
multu $15, $16
div   $17, $18
divu  $19, $20
oneRegisterOperand:
mfhi  $21
mflo  $22
lis   $23
jr    $24
jalr  $25
branchInstructions:
beq   $26, $27,  1
bne   $28, $29, -1
memoryInstructions:
lw    $30,  4($31)
sw    $31, -4($30)
lastButNotLeast:
.word lastButNotLeast



