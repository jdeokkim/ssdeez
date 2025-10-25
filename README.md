<div align="center">

<a href="https://github.com/jdeokkim/ssdeez">
    <img src="docs/static/ssdeez.png" alt="SSDeez 🥜" width="169" height="169">
</a>

# SSDeez 🥜

[![CodeFactor](https://www.codefactor.io/repository/github/jdeokkim/ssdeez/badge)](https://www.codefactor.io/repository/github/jdeokkim/ssdeez)
[![Code Size](https://img.shields.io/github/languages/code-size/jdeokkim/ssdeez?color=brightgreen)](https://github.com/jdeokkim/ssdeez)
[![License](https://img.shields.io/github/license/jdeokkim/ssdeez)](https://github.com/jdeokkim/ssdeez/blob/main/LICENSE)

I present SSDeez, a lightweight NAND flash-based SSD simulator written in C99, for educational purposes.

</div>

## Features

> [!CAUTION]
> Please be aware that a lot of features are currently not implemented.

- Page
  - Data Area
    - [x] Program/Read Operations
  - Out-Of-Band (OOB) Area
    - [ ] Error Correction Code (ECC)
    - [x] Layer-to-Layer Endurance Variation
    - [x] Page States (Free, Valid, Bad, etc.)
    - [ ] Page-Level Statistics
      - [ ] Last Program Time
      - [ ] Last Read Time
      - [x] Total Program Count
      - [x] Total Read Count
    - [x] Physical Page Address
    - [x] "Probablistic" P/E Cycle Counts
    - [x] "Probablistic" Program/Read Latencies
    - [ ] Read Disturbance
- Block
  - [x] Block States (Free, Active, Bad, etc.)
  - [ ] Block-Level Statistics
    - [ ] Last Erase Time
    - [x] Page State Bitmap
    - [x] Total Erase Count
  - [x] Erase Operations
  - [x] "Probablistic" Erase Latencies
  - [x] Sequential Page Programming
- Plane
  - [ ] Plane-Level Statistics
    - [x] Block State Bitmap
    - [ ] Least Worn Block
- Die
  - [x] Factory Bad Block Injection
    - [x] "Spatial Correlation" Model
- Chip (Package)
  - [ ] [ONFI 1.0 Simulation](https://onfi.org)
- Channel
- SSD

~~TODO: More Features~~

<!-- TODO: ... -->

## Prerequisites

<!-- TODO: ... -->

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
- [Y. Kai, E. F. Haratsch, O. Mutlu, and K. Mai, "Threshold Voltage Distribution in MLC NAND Flash Memory: Characterization, Analysis, and Modeling," Design, Automation & Test in Europe (DATE) Conference & Exhibition, 2013.](https://users.ece.cmu.edu/~omutlu/pub/flash-memory-voltage-characterization_date13.pdf)
- [Y. Kim, B. Tauras, A. Gupta and B. Urgaonkar, "FlashSim: A Simulator for NAND Flash-Based Solid-State Drives," International Conference on Advances in System Simulation (SIMUL), Sep. 2009.](https://ieeexplore.ieee.org/document/5283998)

## License

MIT License

> Copyright (c) 2025 Jaedeok Kim <jdeokkim@protonmail.com>
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
