# Script and code for generating attack traces@chipwhisperer

- install the chipwhisperer framework (and make sure that `arm-none-eabi-gcc` and `clang` is available)
- link or copy this directory to `<chipwhisperer_path>/hardware/victims/firmware/`
- to check everything, before starting the python script, build the image files by running `make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE CRYPTO_OPTIONS=NONE` from `<chipwhisperer_path>/hardware/victims/firmware/gen_attacktraces/`
- run `python gen_attacktraces.py <chipwhisperer_path>/hardware/victims/firmware/gen_attacktraces/ [CWLITEARM|CW308_STM32F4] 100` for collecting 100 traces from `<chipwhisperer_path>/hardware/victims/firmware/gen_attacktraces/`

It should generate files containing the traces. The number depends on the chosen input argument.

The signature generation of UOV requires the evaluation of so-called vinegar variables under a quadratic map with known coefficients. This includes the multiplication of these vinegar variables with a considerable amount of known field elements. We trace the power consumption of these multiplications.
In the attack step, the vinegar variables are randomly generated during the signing operation and not known to an attacker. For each potential signature generation, we trace the multiplication of the vinegar variables with the known field elements. 
After acquiring the power trace, we can compare it to the reference traces and find the one with the highest correlation, which is probably the secret vinegar value.
