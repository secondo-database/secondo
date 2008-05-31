#!/bin/shell
cpp -C RegionInterpolator.h | sed /^#/d >tmp   
pdview tmp
rm tmp
