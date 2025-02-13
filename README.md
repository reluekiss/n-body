n-body in c and raylib

to add collision box add -DBOX and to add space curvature grid lines add -DGRID
compile and run
```console
$ cc -o main main.c -lraylib -lm -O2
$ ./main <#particles> <arrangement> <mass>
```
there exist defaults
the available arrangements are circle and random
