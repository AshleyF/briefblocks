# Brief Turtle Language

# Bytecode VM

The VM is a stack machine. Zero-operand instructions consume and produce values on a shared stack. An additional stack contains return addresses for nesting calls.

A program counter (`p`) into memory is maintained. Bytecodes are read through this pointer and interpreted as either single-byte instructions or two-bytes calls. If the leading bit is set, then this along with the next byte form an address to call (sans-leading bit).

    void run() {
      int i;
      do {
        i = mem(p++);
        if ((i & 0x40) == 0) { // instruction? (7th bit set is a call)
          instructions[i](); // execute
        } else { // address to call
          if (mem(p + 1) != 0) rpush(p + 1); // push return address if not followed by return (TCO)
          p = ((i << 8) & 0x3f00) | mem(p); // jump (address is lower 6 bits)
        }
      } while (p >= 0); // note: -1 initially pushed to return stack
    }

To execute code at a particular address, the data and return stacks are reset, the program counter (`p`) begins at the address being executed, and a `-1` is pushed to the return stack to cause termination. Then the above `run()` is called.

    void exec(int address) {
      r = rstack - 1; // reset return stack
      s = dstack - 1; // reset data stack
      p = address;
      rpush(-1); // causing run() to fall through upon completion
      run();
    }

This assumes that the instruction stream is terminated by a `ret`urn instruction; the implementation of which merely pops the return stack: `p = rpop()`.

# Instructions

The following are the instructions understood by the VM:

| Instruction | Bytecode |     |
| ----------- | -------- | --- |
| `ret`    | `00` | `p = rpop()` |
| `lit`    | `01` | `push(0)` |
| `dig0`   | `02` | `push(pop() * 10 + 0)` |
| `dig1`   | `03` | `push(pop() * 10 + 1)` |
| `dig2`   | `04` | `push(pop() * 10 + 2)` |
| `dig3`   | `05` | `push(pop() * 10 + 3)` |
| `dig4`   | `06` | `push(pop() * 10 + 4)` |
| `dig5`   | `07` | `push(pop() * 10 + 5)` |
| `dig6`   | `08` | `push(pop() * 10 + 6)` |
| `dig7`   | `09` | `push(pop() * 10 + 7)` |
| `dig8`   | `0A` | `push(pop() * 10 + 8)` |
| `dig9`   | `0B` | `push(pop() * 10 + 9)` |
| `mod`    | `0C` | `push(pop() % pop())` |
| `mul`    | `0D` | `push(pop() * pop())` |
| `div`    | `0E` | `push(pop() / pop())` |
| `add`    | `0F` | `push(pop() + pop())` |
| `sub`    | `10` | `push(pop() - pop())` |
| `neg`    | `11` | `push(-pop())` |
| `drop`   | `12` | `pop()` |
| `dup`    | `13` | `int a = pop(); push(a); push(a)` |
| `swap`   | `14` | `int a = pop(); int b = pop(); push(a); push(b)` |
| `repeat` | `15` | `int a = pop(); int t = pop(); for (int i = 0; i < t; i++) exec(a)` |
| `forward` | `16`| `double d = pop(); x += d * cos(theta / 180.0 * PI); y += d * sin(theta / 180.0 * PI)` |
| `turn`   | `17` | `theta += pop()` |

## Subroutines

As stated before, there is no "call" instruction. Instead addresses to call are encoded with the leading bit set. This optimizes for subroutine threading where, at the higher levels of abstraction, routines are very commonly simply a sequence of calls.  
The `ret` instruction returns from calls.

## Literals

To get data onto the stack initially, we use `lit` along with the `dig0`, `dig1`, ..., `dig9` instructions. The `lit` instruction pushes a zero value. From there, decimal digits are added by multiplying the current stack value by 10 and adding a digit. For example, the sequence `lit dig1 dig2 dig3` results in 123 on the stack - `lit` pushes a zero, `dig1` multiplies by 10 (still 0) and adds 1 (now 1), `dig2` multiplies by 10 (now 10) and adds 2 (now 12), and finally `dig3` multiplies by 10 (now 120) and adds 3 (finally 123).

This requires four bytes; not the most efficient encoding of the value 123. It has the benifit of remaining zero-operand where other stack machine instruction sets have special literal instructions taking the value as an operand. The uniformity of the instruction set is appealing.

## Arithematic

The binary arithematic operators (`mod`, `mul`, `div`, `add`, `sub`) each pop two values from the stack and push back one. The VM can be used as an RPN calculator at this point. The unary `neg` instruction simply replaces the top stack value with a negated version. Otherwise there is no way to create a negative literal.

## Stack Manipulation

At times, operands may not be in the correct order on the stack or may need to be ignored. For this, `drop` discards, `dup` duplicates and `swap` transposes the top two values. Other common stack manipulators such as "over", "roll", "pick", etc. are not included out of simplicity.

## Repeat

The only looping construct is `repeat`. This executes a given address a given number of times. Other constructs such as counted loops and arbitrary bounds are not supported out of simplicity.

## Turtle Graphics

Internally, a position (`x`/`y`) and heading (`theta`) are maintained. This is the "turtle." Ultimately, all that can be done with the turtle is to move `forward` and to `turn`. Other secondary concepts such as `left` (negative turn), `right` (positive turn) and `back` (negative `forward`) are provided in the higher level language but are not instructions in the VM.

## Missing

It should be noted that there is no conditional (`if`/`then`/`else`) construct, no arbitrary `jump`s, no `load`/`store` instructions, etc. Much is missing out of simplicity.

# Secondaries

Many routines are "baked" into memory to be called as needed.

| Routine | Instructions |
| ----------- | -------- |
| `left`    | `neg turn ret` |
| `right`    | `turn ret` |
| `back`    | `neg forward ret` |
| `back`    | `neg forward ret` |

And very many literals are available as two-byte calls to routines using the otherwise inefficient literal instructions. Values include all single digits, multiples of ten up to 360, and all other values that are evenly divisible into 360 - that is, `12`, `15`, `18`, `24`, `36`, `45` and `72`. This is for convenience in dealing with degrees in turtle graphics.

| Routine | Instructions |
| ----------- | -------- |
| `0`     | `lit ret` |
| `1`     | `lit dig1 ret` |
| `2`     | `lit dig2 ret` |
| `3`     | `lit dig3 ret` |
| `4`     | `lit dig4 ret` |
| `5`     | `lit dig5 ret` |
| `6`     | `lit dig6 ret` |
| `7`     | `lit dig7 ret` |
| `8`     | `lit dig8 ret` |
| `9`     | `lit dig9 ret` |
| `10`    | `lit dig1 dig0 ret` |
| `12`    | `lit dig1 dig2 ret` |
| `15`    | `lit dig1 dig5 ret` |
| `18`    | `lit dig1 dig8 ret` |
| `20`    | `lit dig2 dig0 ret` |
| `24`    | `lit dig2 dig4 ret` |
| `30`    | `lit dig3 dig0 ret` |
| `36`    | `lit dig3 dig6 ret` |
| `40`    | `lit dig4 dig0 ret` |
| `45`    | `lit dig4 dig5 ret` |
| `50`    | `lit dig5 dig0 ret` |
| `60`    | `lit dig6 dig0 ret` |
| `70`    | `lit dig7 dig0 ret` |
| `72`    | `lit dig7 dig2 ret` |
| `80`    | `lit dig8 dig0 ret` |
| `90`    | `lit dig9 dig0 ret` |
| `100`   | `lit dig1 dig0 dig0 ret` |
| `110`   | `lit dig1 dig1 dig0 ret` |
| `120`   | `lit dig1 dig2 dig0 ret` |
| `130`   | `lit dig1 dig3 dig0 ret` |
| `140`   | `lit dig1 dig4 dig0 ret` |
| `150`   | `lit dig1 dig5 dig0 ret` |
| `160`   | `lit dig1 dig6 dig0 ret` |
| `170`   | `lit dig1 dig7 dig0 ret` |
| `180`   | `lit dig1 dig8 dig0 ret` |
| `190`   | `lit dig1 dig9 dig0 ret` |
| `200`   | `lit dig2 dig0 dig0 ret` |
| `210`   | `lit dig2 dig1 dig0 ret` |
| `220`   | `lit dig2 dig2 dig0 ret` |
| `230`   | `lit dig2 dig3 dig0 ret` |
| `240`   | `lit dig2 dig4 dig0 ret` |
| `250`   | `lit dig2 dig5 dig0 ret` |
| `260`   | `lit dig2 dig6 dig0 ret` |
| `270`   | `lit dig2 dig7 dig0 ret` |
| `280`   | `lit dig2 dig8 dig0 ret` |
| `290`   | `lit dig2 dig9 dig0 ret` |
| `300`   | `lit dig3 dig0 dig0 ret` |
| `310`   | `lit dig3 dig1 dig0 ret` |
| `320`   | `lit dig3 dig2 dig0 ret` |
| `330`   | `lit dig3 dig3 dig0 ret` |
| `340`   | `lit dig3 dig4 dig0 ret` |
| `350`   | `lit dig3 dig5 dig0 ret` |
| `360`   | `lit dig3 dig6 dig0 ret` |

# Compression

Because function and parameter tiles contain a very tiny 1024-bit EEPROM, we need to compress the instruction stream as much as possible! Another way to think of the instruction stream is as a simple stream of integers with maximum value 2^16. Values below 128 are instructions. Other values are addresses to call.

Certain instructions will be more common than other and certain routines will be called more often than others. We pre-determine these relative frequencies and use Huffman coding to achieve a substantial level of compression.

## Sample Programs

`20 forward 10 right 36 repeat` draws a circle. The use may store this on a custom tile and call it `circle`. Then `circle 20 right 18 repeat` makes an interesting pattern of circles. They may store this too on a custom tile. The catch is that the second custom tile cannot reference the first. Instead, the new definition along with definitions on which it depends must be stored on the second custom tile.

The literal `20`, `10`, `36` and `18` each take two bytes (calls to a known literals). `forward` and `right` are a one-byte instructions (`right` compiles to `turn` actually. `left`, on the other hand, would be a two-byte call to a secondary [`neg turn ret`]). `repeat` is special - it compiles to contain a literal offset to the address to be executed repeatedly (one byte) [TODO: or a marker byte?].

The total byte count is: 17

## Huffman Tree

TODO