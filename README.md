Brief Description
-----------------

An implementation of zero-runtime-overhead destructors in C, relying on constant-folding optimisations.

Compiler success matrix
-----------------------

| compiler                   | status, under O3 equivalent     |
| -------------------------- | ------------------------------- |
| gcc                        | works since ver. 4.6.4          |
| clang                      | works since ver. 3.0.0          |
| icc                        | does not work as of ver. 18     |
| msvc                       | does not work as of ver. 2017   |

