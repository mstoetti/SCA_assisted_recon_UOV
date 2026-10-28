# Script and code for generating reference traces@chipwhisperer

- install the chipwhisperer framework (and make sure that `arm-none-eabi-gcc` and `clang` is available)
- link or copy this project to `<chipwhisperer_path>/hardware/victims/firmware/`
- to check everything, before starting the python script, build the image files by running `make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE CRYPTO_OPTIONS=NONE` from `<chipwhisperer_path>/hardware/victims/firmware/gen_reftraces/`
- run `python gen_reftraces.py <chipwhisperer_path>/hardware/victims/firmware/gen_reftraces/ [CWLITEARM|CW308_STM32F4]` from `<chipwhisperer_path>/hardware/victims/firmware/gen_reftraces/`

It should generate 256 `*.raw` files and a `randtrace.raw` file containing the traces.

The signature generation of UOV requires the evaluation of so-called vinegar variables under a quadratic map with known coefficients. This includes the multiplication of these vinegar variables with a considerable amount of known field elements. We trace the power consumption of these multiplications.
In the profiling step, we manually set the vinegar variables all to a certain value x in Fq and record the power trace of the multiplications. This is repeated for every element in Fq.
