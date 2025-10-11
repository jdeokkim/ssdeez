<div align="center">

<a href="https://github.com/jdeokkim/ssdeez">
    <img src="docs/static/ssdeez.png" alt="SSDeez 🥜" width="169" height="169">
</a>

# SSDeez 🥜

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
    - [x] Page States (Free, Valid, Bad, etc.)
    - [ ] Page-Level Statistics
      - [ ] Last Program Time
      - [ ] Last Read Time
      - [x] Total Program Count
      - [ ] Total Read Count
    - [x] Program/Read Latencies
    - [x] Randomized P/E Cycle Counts
      - [x] Gaussian Sampling ([Marsaglia's Polar](https://www.jstor.org/stable/2027592) + [xoshiro256++](https://prng.di.unimi.it))
      - [x] Layer-to-Layer Endurance Variation
    - [ ] Read Disturbance
- Block
  - [x] Block States (Free, Valid, Bad, etc.)
  - [ ] Block-Level Statistics
      - [ ] Last Erase Time
      - [x] Total Erase Count
  - [x] Erase Latencies
- Plane
- Die (Chip)
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

GNU General Public License, version 3

> Copyright (c) 2025 Jaedeok Kim (jdeokkim@protonmail.com)
>
> This program is free software: you can redistribute it and/or modify
> it under the terms of the GNU General Public License as published by
> the Free Software Foundation, either version 3 of the License, or
> (at your option) any later version.
>
> This program is distributed in the hope that it will be useful,
> but WITHOUT ANY WARRANTY; without even the implied warranty of
> MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
> GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License
> along with this program. If not, see <https://www.gnu.org/licenses/>.
