# script and code for trace capture@chipwhisperer

- install chipwhisperer framework (and make sure that `arm-none-eabi-gcc` is available)
- link or copy this project to `<chipwhisperer_path>/hardware/victims/firmware`
- to check everything, before starting the python script, build the image files by running `make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE CRYPTO_OPTIONS=NONE` from `<chipwhisperer_path>/hardware/victims/firmware/UOV_SPA_check`
- run `python SPA_check.py <chipwhisperer_path>/hardware/victims/firmware/UOV_SPA_check CWLITEARM 100` for collecting 100 traces from `<chipwhisperer_path>/hardware/victims/firmware/UOV_SPA_check`

It should generate files containing the results.
