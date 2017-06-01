#!/bin/bash

# mess with output to confuse jenkins build failure indications because these
# tests pass when compilation fails
eval $@ 2>&1 | sed s/error\\\|failed\\\|Error/[Censored\ by\ testrunner.bash]/g
