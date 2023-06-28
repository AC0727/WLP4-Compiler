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
lis $12
.word 12

; skip header
jalr $2
add $7, $3, $0 ; starting memory address
add $8, $7, $0 ; copy of $7
jalr $2
jalr $2
add $9, $3, $0 ; $9 stores end of merl file address
jalr $2
sub $6, $3, $12 ; code size in bytes
add $15, $6, $0 ; copy over $6

lis $12
.word 8

loop:
beq $6, $0, relocate
jalr $2
add $1, $3, $0 ; move to 1
jalr $5 ; print
sw $1, 0($8)
add $8, $8, $4 ; increase index by 1
sub $6, $6, $4 ;; decrease counter
beq $0, $0, loop


relocate:
lis $12
.word 12
add $10, $15, $12 ; i

relocateLoop:
slt $13, $10, $9
bne $13, $11, execute
jalr $2 ; read in row in relocation table
beq $3, $11, entry
jr $31
    entry:
    jalr $2
    add $13, $7, $3 ; startAddress + rel
    sub $13, $13, $12 ; startAddress + rel - 12
    lw $14, 0($13) ; entry there
    add $14, $14, $7
    sub $14, $14, $12
    sw $14, 0($13)
lis $12
.word 8
add $10, $10, $12
lis $12
.word 12
beq $0, $0, relocateLoop


execute:
sw $7, -8($30)
sw $8, -12($30)
sw $5, -16($30)
sub $30, $30, $12

lis $31
.word 0x10000
add $30, $7, $31

jalr $7 ; error here

lis $30
.word 0x00fffff0
lis $12
.word 12

add $30, $30, $12
lw $5, -16($30)
lw $8, -12($30)
lw $7, -8($30)

lis $4
.word 4

printTwo:
beq $7, $8, end  ; out of index, move to end
lw $1, 0($7)
jalr $5
add $7, $7, $4 ; increase index
beq $0, $0, printTwo

end:
lis $31
.word 4
add $30, $30, $31
lw $31, -4($30)
jr $31
