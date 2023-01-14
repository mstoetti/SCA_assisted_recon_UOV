# script and code for trace capture@chipwhisperer

- install the chipwhisperer framework (and make sure that `arm-none-eabi-gcc` is available)
- link or copy this project to `<chipwhisperer_path>/hardware/victims/firmware/`
- to check everything, before starting the python script, build the image files by running `make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE CRYPTO_OPTIONS=NONE` from `<chipwhisperer_path>/hardware/victims/firmware/gen_reftraces/`
- run `python gen_reftraces.py <chipwhisperer_path>/hardware/victims/firmware/gen_reftraces/ [CWLITEARM|CW308_STM32F4]` from `<chipwhisperer_path>/hardware/victims/firmware/gen_reftraces/`

It should generate 256 `*.csv` files and a `randtrace.csv` containing the traces.