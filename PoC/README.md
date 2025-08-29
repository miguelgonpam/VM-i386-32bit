# Poc (Proof of Concept)
<p align="justify">The i386 emulator idea was conceived to be have a teaching approach, so Computer Science students can learn how assembly level works. This project had two main teaching approaches in mind: 
<br>- Understanding how assembly works and what every instruction does. 
<br>- Understanding how any C program is translated into assembly (via loading and linking), and that assembly code is executed, focusing mainly on the vulnerabilities that "bad code" creates, and how some mitigation techniques can avoid that.
</p>

<p align="justify">
The first approach can be satisfied writing some assembly programs and watching how the registers and memory change step by step, but the second one is more difficult to achieve. This proofs of concept were designed to help with that task.
</p>

<p align="justify">
Modern mitigation techniques such as <a href="https://en.wikipedia.org/wiki/Address_space_layout_randomization">ASLR</a> or <a href="https://en.wikipedia.org/wiki/NX_bit">NX</a> were not implemented, so students can learn how the absence of this techniques could be catastrophic.
Compiler-dependent mitigation techniques such as <a href="https://en.wikipedia.org/wiki/Buffer_overflow_protection">SSP (Stack Smashing Protector)</a> are expected to be turned off, so the attack can be replicated.
Understanding how this techniques work and how the lack of them affects any program is a step every Cibersecurity Student has to make.
</p>
This directory contains some Proofs of Concept of some "poor writen" C programs, and how they can be exploited. 


