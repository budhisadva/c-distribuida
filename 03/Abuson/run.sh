#!/bin/bash
mpicc abuson.c
mpirun --oversubscribe -np 5 a.out
