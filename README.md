Build:

    make

Run

    ./c2ibniz <file.ibc>

To pipe the output into IBNIZ:

    ./c2ibniz <file.ibc> | xargs ibniz -c

(You can press TAB in IBNIZ to open the code editor after launching it this way)

#File Spec

A valid IBC file consists of the C language restricted to minimal arithmetic operations. Instead of the main function, the file must define one or more of the following function signatures:

    int ibniz_video_tyx(int t, int y, int x)
    int ibniz_video_t(int t)
    int ibniz_audio(int t)

In the case both video functions are defined, `ibniz_video_t` takes priority.

The user is allowed to define one global array of unsigned ints named `DATA[]`. This is the only global/heap based storage allowed by C2IBNIZ.

The only variable type allowed is signed `int`. Since IBNIZ works on 16.16 fixed point numbers, two functions are allowed to specify literals in the code.

   frac(double) \\convert a double to 16.16
   fixed16(int,int) \\treat the first arg as the upper 16 bits, the second as the lower 16 bits
                    \\(Recall: Upper sixteen represent whole number, Lower 16 represent fractional)

Note that these are only allowed for literals in the code (especially since `double` variables are forbidden).

Additionally, three math functions are provided: `sin(int)`, `sqrt(int)`, and `atan(int,int)` for sine, square root, and inverse tangent.

All arithmetic operations follow the IBNIZ semantics rather than C semantics when they differ, which comes down to the 16.16 fixed width format and the simple boundary conditions (like overflow or dividing by 0).

Branches and loops are allowed.

The user may define additional subroutines and call them. Non-tail recursion (or co-recursion) is not supported.

When in doubt, see the sample files.

#Output

code for IBNIZ, the Ideally Bare Numeric Impression giZmo. 

(All debug information is printed on std err, so `2>/dev/null` will get you just the IBNIZ code.)
