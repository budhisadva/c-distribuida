#!/bin/bash
mpicc abuson.c
mpirun --oversubscribe -np 6 a.out
