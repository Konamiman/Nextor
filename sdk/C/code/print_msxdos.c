void print(char* s) __naked
{
    __asm

    ;HL = *s

loop:
    ld  a,(hl)
    or  a
    jr  z,end
    ld  e,a
    ld  c,#2
    push    hl
    call    #5
    pop hl
    inc hl
    jr  loop
end:
    ret
    __endasm;    
}
