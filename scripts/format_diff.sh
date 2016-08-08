#!/bin/bash

git show HEAD | ~/clang-format-diff.py -style=file -p1 -i -iregex '.*\.(metal|glsl|cpp|mm|h|m|c)'