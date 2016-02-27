#!/bin/bash
pushd ../build
make
EXIT=$?
popd

if [ $EXIT -eq 0 ]; then
  ../build/seastash
fi
