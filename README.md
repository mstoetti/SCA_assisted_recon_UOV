# Separating Oil and Vinegar with a Single Trace

This repository contains auxiliary material for the paper: **"Separating Oil and Vinegar with a Single Trace : 
Side-Channel Assisted Reconciliation Attack on UOV"**.

# Practical Evaluation

# Single Trace Attack on UOV
This repository allows to run a single trace attack on UOV using the ChipWhisperer Setup with an STM32F3 Target Board. For the attack to be successful, we need 256 reference traces (possibly from a profiling device) and one attack trace (from the target device). The attached folders allow you to either generate the traces by yourself or to work this those we provide, in case you do not have the CW Setup at hand or just want to execute the attack quickly. Please consider the following folders

- gen_attacktraces: Generate your own attack traces. You need the CW Setup with an STM32F3 board.
- gen_reftraces: Generate your own reference traces. You need the CW Setup with an STM32F3 board. Note, that this takes around 8 hours to complete.
- prepared_attacktraces: Contains 100 attack traces along with the corresponding signatures.
- prepared_reftraces: Contains 256 reference traces.


# Demonstration of the Attack
For a detailed description of the attack, please read the paper <paper>.

The script 'DPA_recon.py' contains the attack.

- run `python DPA_recon.py 0' to run the attack with the provided 100 attack traces and 256 reference traces
    for each of the 100 attack traces, the script will either output the secret key (the secret oil space) corresponding to the public key of the attack trace or a message that the attack trace was not 

- if you already have collected some traces on your own and want to try the attack on them, run 'python DPA_recon.py 1'

# Licenses

Code in this repository that does not indicate otherwise is placed in the public domain.

For the third party code see their licenses:

- [UOV](https://github.com/pqov/pqov-paper): [https://github.com/pqov/pqov-paper](https://github.com/pqov/pqov-paper)
- [ChipWhisperer](https://github.com/newaetech/chipwhisperer): [https://github.com/newaetech/chipwhisperer/blob/develop/LICENSE.txt](https://github.com/newaetech/chipwhisperer/blob/develop/LICENSE.txt)
