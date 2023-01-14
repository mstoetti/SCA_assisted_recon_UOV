# script and code for trace capture@chipwhisperer

- install the chipwhisperer framework (and make sure that `arm-none-eabi-gcc` is available)
- link or copy this directory to `<chipwhisperer_path>/hardware/victims/firmware/`
- to check everything, before starting the python script, build the image files by running `make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE CRYPTO_OPTIONS=NONE` from `<chipwhisperer_path>/hardware/victims/firmware/gen_attacktraces/`
- run `python gen_attacktraces.py <chipwhisperer_path>/hardware/victims/firmware/gen_attacktraces/ [CWLITEARM|CW308_STM32F4] 100` for collecting 100 traces from `<chipwhisperer_path>/hardware/victims/firmware/gen_attacktraces/`

It should generate files containing the results.
