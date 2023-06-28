.import readWord
.import printHex
sw $31, -4($30)
lis $31
.word 4
sub $30, $30, $31

lis $2
.word readWord
lis $5
.word printHex
lis $11
.word 0x01
lis $4
.word 4

; skip header
jalr $2
jalr $2
jalr $2
lis $12
.word 12
sub $6, $3, $12 ; code size in bytes

loop:
beq $6, $0, end
jalr $2
add $1, $3, $0 ; move to 1
jalr $5 ; print
sub $6, $6, $4 ;; decrease counter
beq $0, $0, loop

end:
lis $31
.word 4
add $30, $30, $31
lw $31, -4($30)
jr $31
