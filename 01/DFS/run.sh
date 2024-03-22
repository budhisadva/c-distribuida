#!/bin/bash
mpicc dfs.c
mpirun --oversubscribe -np 3 a.out
