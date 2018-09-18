Brief Description
-----------------

An implementation of zero-runtime-overhead destructors in C, relying on constant-folding optimisations.

For an in-depth account see article on [CNX Software](https://www.cnx-software.com/2018/09/17/zero-overhead-destructors-in-c/).

Compiler success matrix
-----------------------

| compiler                   | status, under O3 equivalent     |
| -------------------------- | ------------------------------- |
| gcc                        | works since ver. 4.6.4          |
| clang                      | works since ver. 3.0.0          |
| icc                        | does not work as of ver. 18     |
| msvc                       | does not work as of ver. 2017   |


How does a constant-folded destructor invocation look?
------------------------------------------------------

A constat-folded destructor invocation should appear like a direct funciton call when the destructor is not inlined. For example:

```c
int main(int argc, char** argv) {
	begin_scope(0);

	Foo f;
	construct(0, f, Foo_ctor); // empty ctor, corresponding dtor named Foo_dtor, non-inlined

	end_scope(0);
	return EXIT_SUCCESS;
}
```

Target amd64:
```
main:
        subq    $24, %rsp
        movq    %rsp, %rdi
        call    Foo_dtor
        xorl    %eax, %eax
        addq    $24, %rsp
        ret
```
Target arm7l:
```
main:
        str     lr, [sp, #-4]!
        sub     sp, sp, #12
        add     r0, sp, #4
        bl      Foo_dtor
        mov     r0, #0
        add     sp, sp, #12
        ldmfd   sp!, {pc}
```
Target mips32:
```
main:
        addiu   $sp,$sp,-40
        sw      $31,36($sp)
        jal     Foo_dtor
        addiu   $4,$sp,24
        lw      $31,36($sp)
        move    $2,$0
        j       $31
        addiu   $sp,$sp,40
```

Conversely, when the compiler did not constant-fold the destructor invocation, the code should have an *indirect* function call -- via a register that was previosuly loaded with the address of the destructor, normally from a source on stack.

```c
int main(int argc, char** argv) {
    begin_scope(0);

    Foo f;
    construct(0, f, Foo_ctor);

    __asm volatile ("": : : "memory"); // flush compiler's knowledge of constants that went to memory

    end_scope(0); // we might get an assert check as well if we haven't disabled those
    return EXIT_SUCCESS;
}
```

Target amd64:
```
main:
        subq    $1048, %rsp
        leaq    1024(%rsp), %rax
        movq    $Foo_dtor, 8(%rsp)
        movq    %rax, (%rsp)
        movq    8(%rsp), %rax
        testq   %rax, %rax
        je      .L27
        movq    (%rsp), %rdi
        call    *%rax
        xorl    %eax, %eax
        addq    $1048, %rsp
        ret
.L27:
        movl    $__PRETTY_FUNCTION__.1992, %ecx
        movl    $16, %edx
        movl    $.LC0, %esi
        movl    $.LC3, %edi
        call    __assert_fail
```
