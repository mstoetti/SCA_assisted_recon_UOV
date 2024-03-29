# Separating Oil and Vinegar with a Single Trace

This repository contains auxiliary material for the paper: ["Separating Oil and Vinegar with a Single Trace : Side-Channel Assisted Reconciliation Attack on UOV"](https://eprint.iacr.org/2023/335).

Authors:
- [Thomas Aulbach](https://www.uni-regensburg.de/informatics-data-science/qpc/team/thomas-aulbach/index.html)
- [Fabio Campos](https://www.sopmac.org/)
- [Juliane Krämer](https://www.uni-regensburg.de/informatics-data-science/qpc/team/prof-dr-juliane-kraemer/index.html)
- [Simona Samardjiska](https://samardjiska.org)
- [Marc Stöttinger](https://www.hs-rm.de/de/hochschule/person`<en/stoettinger-marc) 

## Overview
This repository allows to run a single trace attack on UOV using the ChipWhisperer Setup with an STM32F3 Target Board. For the attack to be successful, we need 256 reference traces (possibly from a profiling device) and one attack trace (from the target device). The attached folders allow you to either generate the traces by yourself or to work with those we provide, in case you do not have the CW Setup at hand or just want to execute the attack quickly. Please consider the following folders:

- **gen_attacktraces**: Generate your own attack traces. You need the CW Setup with an STM32F3 board. Note that if one takes around 100 attacktraces to get an estimate for the success probability, this might take a few hours to complete.
- **gen_reftraces**: Generate your own reference traces. You need the CW Setup with an STM32F3 board. Note, that this takes several hours up to one day to complete.
- **prepared_attacktraces**: Contains 100 attack traces along with the corresponding signatures.
- **prepared_reftraces**: Contains 256 reference traces and a random trace file.


## Demonstration of the Attack
For a detailed description of the attack, please read the paper.

The script **DPA_Recon.py** contains the attack.

- the script requires **python** >= 3.8.x, **sagemath** >= 9.0, and **numpy** >= 1.17.4.

- run `python DPA_Recon.py 0 100` to run the attack with the provided 100 attack traces and 256 reference traces. After recovering the oil vector(s) from the traces, the script will ask to select which one of the recovered oil vector(s) should be used for the Kipnis Shamir attack and Reconciliation step.

- run `python DPA_Recon.py 1 [NUM_OF_TRACES]`, if you already have collected reference traces in **gen_reftraces** and attack traces in **gen_attacktraces** on your own and want to try the attack on them. After recovering the oil vector(s) from the traces, the script will ask to select which one of the recovered oil vector(s) should be used for the Kipnis Shamir attack and Reconciliation step.

The script **simulate_noisy_HW_measure.py** contains a simulated attack, that recoveres the vinegar variables from the Hamming weights of certain products in the signing process. The noise level of the simulated Hamming weight measurements can be adjusted.

# Licenses

Code in this repository that does not indicate otherwise is placed in the public domain.

For the third party code see their licenses:

- [UOV](https://github.com/pqov/pqov-paper): [https://github.com/pqov/pqov-paper](https://github.com/pqov/pqov-paper)
- [ChipWhisperer](https://github.com/newaetech/chipwhisperer): [https://github.com/newaetech/chipwhisperer/blob/develop/LICENSE.txt](https://github.com/newaetech/chipwhisperer/blob/develop/LICENSE.txt)
