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
| `repeat` | `15` | `int t = pop(); for (int i = 0; i < t; i++) exec(address)` |
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

The only looping construct is `repeat`. This executes the beginning address a given number of times. The address is assumed to be the start of a dictionary frame. The `repeat` instruction itself is replaced by a `ret` temporarily. Other constructs such as counted loops and arbitrary bounds are not supported out of simplicity.

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

Certain instructions will be more common than other and certain routines will be called more often than others. We pre-determine these relative frequencies and use [Huffman coding](https://en.wikipedia.org/wiki/Huffman_coding) to achieve a substantial level of compression.

Upon reading a program from EEPROM, it is unpacked into the bytecode described above and then executed.

## Sample Programs

`20 forward 10 right 36 repeat` draws a circle. The use may store this on a custom tile and call it `circle`. Then `circle 20 right 18 repeat` makes an interesting pattern of circles. They may store this too on a custom tile. The catch is that the second custom tile cannot reference the first. Instead, the new definition along with definitions on which it depends must be stored on the second custom tile.

The literal `20`, `10`, `36` and `18` each take two bytes (calls to a known literals). `forward` and `right` are a one-byte instructions (`right` compiles to `turn` actually. `left`, on the other hand, would be a two-byte call to a secondary [`neg turn ret`]). `repeat` is a single byte.

The total consumption is 15 bytes (byte count is: 120 bits).

## [Huffman-like Tree](https://en.wikipedia.org/wiki/Huffman_coding)

The `ret` instruction is only used internally after expansion, during `repeat` and in the built-in secondary dictionary. It is not contained in compressed programs. Additionally, the raw `turn` instruction is replaced with the alias `right` and the secondary `left`. Out of simplicity, `back` is not included (if necessary it may be encoded as `neg forward`).

### Movements

The most common instructions by far are the movement commands:

| Movement | Encoding |
| -------- | ---- |
| `forward` | `00` |
| `left`    | `01` |
| `right`   | `10` |
| `repeat`  | `11` |

### Literals

Literals, as the parameters to these, are as common as movement instructions. Some literal values are more common than others. "Special" literals have their own Huffman tree, while others are encoded as fixed-width 4-, 8- or 16-bit values.

| Literals  | Encoding |
| --------- | ---- |
| "Special" | `00` |
| 4-bit     | `01` |
| 8-bit     | `10` |
| 16-bit    | `11` |

"Special" values include all single digits, multiples of ten up to 360, and all other values that are evenly divisible into 360 - that is, `12`, `15`, `18`, `24`, `36`, `45` and `72`. Additionally all of these multiplied by 2 which are under 360 (these are useful for making stars - e.g. 144 degree turns for a 5-pointed star). Additionally, +/- 1 of each of 360 divided by 3, 4, 5, 6, and 8 (7 is not evenly divisible). These are useful for spiraling patterns - e.g. slightly incomplete triangles with 89 degree turns.

If we only include values that cannot be encoded in 4-bits (> 15 that is), then it very conveniently includes the following 64 values: `16`, `18`, `20`, `24`, `28`, `30`, `32`, `36`, `40`, `44`, `45`, `46`, `48`, `50`, `56`, `59`, `60`, `61`, `64`, `70`, `71`, `72`, `73`, `80`, `89`, `90`, `91`, `96`, `100`, `110`, `112`, `119`, `120`, `121`, `128`, `130`, `140`, `144`, `150`, `160`, `170`, `180`, `190`, `192`, `200`, `210`, `220`, `224`, `230`, `240`, `250`, `256`, `260`, `270`, `280`, `288`, `290`, `300`, `310`, `320`, `330`, `340`, `350`, `360`,

The reason I say, "conveniently" 64 values, is that these can then be incoded in 6-bits as a lookup into the above list.

For example, the value 10 would be encoded as `011010`. That is `01` to indicate a 4-bit value followed by the simple binary encoding `1010`.

An example of a "special" value, 360 could be encoded as a 16-bit value (`11` indicating 16-bits, followed by `0000000101101000`), but much more efficiently as `11` indicating a special value followed by `111111`; the 63rd special value.

### Miscellaneous

The remaining miscellaneous arithematic and stack manipulation instructions are encoded as 3-bit values:

|     | Encoding |
| --------- | ---- |
| `mul`  | `000` |
| `div`  | `001` |
| `add`  | `010` |
| `sub`  | `011` |
| `neg`  | `100` |
| `drop` | `101` |
| `dup`  | `110` |
| `swap` | `111` |

### Dispatch

Besides these, we want to leave room in the scheme for future extension with backward compatibility. The movement and literal classes have equal frequency, while miscellaneous things are much less common and future expansion is least common (or at least unknown). This gives a true Huffman tree of:

|              | Encoding |
| ------------ | ---- |
| Movement     | `0` |
| Literal      | `10` |
| Miscellaneous | `110` |
| Extension     | `111` |

These are the prefixes to each class.

### Example

Finally, back to our original (120-bit program): `20 forward 10 right 36 repeat 20 right 18 repeat`.

`20` is a special value. `10` indicating a literal, `00` indicating it's "special", followed by `000010` (index 2 into the table). This is just 10 bits; a savings of 6 bits.

`forward` encodes as `0` indicating a movement, followed by `00` for `forward`. This is just 3 bits; a savings of 5 more bits.

`10` can be encoded as a 4-bit literal. `10` followed by `01` indicating 4-bit, followed by `1010` (decimal 10). This is 6 bits; a savings of 10.

`right` is another movement, `0` followed by `10`. A savings of 5.

`36` and `18` are special and encode similarly to `20` (elaborated above). That is, `1000000111` and `1000000001` respectively.

`repeat` is a movement and encodes similarly to `forward` and `right` (above). That is, `011`.

All together, `20 forward 10 right 36 repeat 20 right 18 repeat` becomes `1000000010 000 10011010 010 1000000111 011 1000000010 010 1000000001 011`.

Notice that even smashed together, this bit stream can be decoded unambiguously: `100000001000010011010010100000011101110000000100101000000001011`.

This 63-bit encoding represents a savings of 57 bits; or 47%. A 6-bit average encoding per token in the language should be expected.