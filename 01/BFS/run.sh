#!/bin/bash
mpicc bfs.c
mpirun --oversubscribe -np 6 a.out
