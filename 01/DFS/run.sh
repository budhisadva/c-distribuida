#!/bin/bash
mpicc dfs.c
mpirun --oversubscribe -np 5 a.out
