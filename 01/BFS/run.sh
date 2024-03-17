#!/bin/bash
mpicc bfs.c
mpirun --oversubscribe -np 5 a.out
