<div align="center">

<a href="https://github.com/jdeokkim/ssdeez">
    <img src="docs/static/ssdeez.png" alt="SSDeez 🥜" width="169" height="169">
</a>

# SSDeez 🥜

[![CodeFactor](https://www.codefactor.io/repository/github/jdeokkim/ssdeez/badge)](https://www.codefactor.io/repository/github/jdeokkim/ssdeez)
[![Code Size](https://img.shields.io/github/languages/code-size/jdeokkim/ssdeez?color=brightgreen)](https://github.com/jdeokkim/ssdeez)
[![License](https://img.shields.io/github/license/jdeokkim/ssdeez)](https://github.com/jdeokkim/ssdeez/blob/main/LICENSE)

I present SSDeez 🥜, A tiny NAND flash-based SSD simulator written in C.

</div>

## Features

> [!CAUTION]
> Please be aware that a lot of features are currently not implemented.

<!-- TODO: Cell-to-Cell Inference, Read Disturbance, Retention Loss, etc. -->

- [ ] Error Correction Code (ECC)
- [x] Factory Bad Block Injection ("Spatial Correlation" Model)
- [ ] Layer-to-Layer Endurance Variation
- [ ] [ONFI (Open NAND Flash Interface) 1.0 Support](https://onfi.org)
  - [ ] Block Erase (`0x60` $\rightarrow$ `0xD0`)
  - [ ] Change Read Column (`0x05` $\rightarrow$ `0xE0`)
  - [ ] Change Write Column (`0x85`)
  - [x] Get Features (`0xEE`)
  - [ ] Page Program (`0x80` $\rightarrow$ `0x10`)
  - [ ] Read (`0x00` $\rightarrow$ `0x30`)
  - [x] Read ID (`0x70`)
  - [ ] Read Parameter Page (`0xEC`)
  - [ ] Read Status (`0x90`)
  - [x] Reset (`0xFF`)
  - [ ] Set Features (`0xEF`)
- [x] "Probablistic" Program/Read/Erase Latencies
- [x] Sequential Page Programming

~~TODO: More Features~~

<!-- TODO: ... -->

## Prerequisites

- C99 Compiler
  - Clang 4.0.1+
  - GCC 4.9.3+
- POSIX-conformant Make
  - GNU Make 4.1+
  - NetBSD Make 20160220+
  - [Public Domain POSIX Make](https://github.com/rmyorston/pdpmake) 2.0.0+

## Build Instructions

<!-- TODO: ... -->

### Unit Tests

```console
$ git clone https://github.com/jdeokkim/ssdeez && cd ssdeez
$ make && make -C tests
$ ./tests/bin/ssdeez-tests
```

## References

- [A. Tavakkol, J. Gómez-Luna, M. Sadrosadati, S. Ghose, and O. Mutlu, "MQSim: A Framework for Enabling Realistic Studies of Modern Multi-Queue SSD Devices," USENIX Conference on File and Storage Technologies (FAST), Feb. 2018.](https://www.usenix.org/system/files/conference/fast18/fast18-tavakkol.pdf)
- [M. Raquibuzzaman, A. Milenkovic and B. Ray, "Intrablock Wear Leveling to Counter Layer-to-Layer Endurance Variation of 3-D NAND Flash Memory," IEEE Transactions on Electron Devices, Jan. 2023.](https://ieeexplore.ieee.org/document/9966490)
- [Open NAND Flash Interface Working Group, "Open NAND Flash Interface Specification, Revision 1.0," onfi.org, Dec. 2006.](https://web.archive.org/web/20250214142844/https://onfi.org/files/onfi_1_0_gold.pdf)
- [T. Gleixner, "MTD NAND Driver Programming Interface," The Linux Kernel Documentation, 2025.](https://docs.kernel.org/driver-api/mtdnand.html)
- [Y. Kai, E. F. Haratsch, O. Mutlu, and K. Mai, "Threshold Voltage Distribution in MLC NAND Flash Memory: Characterization, Analysis, and Modeling," Design, Automation & Test in Europe (DATE) Conference & Exhibition, 2013.](https://users.ece.cmu.edu/~omutlu/pub/flash-memory-voltage-characterization_date13.pdf)
- [Y. Kim, B. Tauras, A. Gupta and B. Urgaonkar, "FlashSim: A Simulator for NAND Flash-Based Solid-State Drives," International Conference on Advances in System Simulation (SIMUL), Sep. 2009.](https://ieeexplore.ieee.org/document/5283998)

## License

MIT License

> Copyright (c) 2025 Jaedeok Kim (jdeokkim@protonmail.com)
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE. 
