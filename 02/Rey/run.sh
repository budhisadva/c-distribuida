#!/bin/bash
mpicc rey.c
mpirun --oversubscribe -np 5 a.out
