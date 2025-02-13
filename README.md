n-body in c and raylib

to add collision box add -DBOX and to add space curvature grid lines add -DGRID
compile and run
```console
$ cc -o 2d 2d.c -lraylib -lm -O2
$ ./2d <#particles> <arrangement> <mass>
```
there exist defaults
the available arrangements are circle and random
