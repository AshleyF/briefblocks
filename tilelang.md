# Tile Language

## Motions

Starting with very simple Turtle Graphics commands:

* Forward
* Left
* Right

More general:

* Move (positive forward, negative backward)
* Turn (positive right, negative left)

## Constructs

* Repeat

Repeat is a jump back to the start address of the current sequence. Once saved and embedded into other sequences, it is a delimited loop.

The "syntax" of the physical tiles is `x y z 10 repeat` - meaning do x, y and z ten times. This is simplified for kids. There are no loop delimiting words. It's assumed that the start of the loop is the start of the program sequence. To construct arbitrarily nested sequences, save sub-sequences to a new tile. Also, it is assumed that the immediately preceeding literal is the loop count - no computed counts are allowed.

Normal Brief syntax would be more like `10 do x y z repeat` with the count (potentially computed) _before_ the `do` and with `do` delimiting the start. Through macro expansion, this becomes the instructions: `lit8 10 pushr x y z next 3`. The `next` instruction is a counted loop, taking a count from the return stack (hence, `lit8 10 pushr`) and an operand from the instruction stream (`3`) indicating the relative start of the loop. It then counts down to zero (maintained on the return stack) and drops the top of the return stack upon completion.

To do the same with physical tiles, the `repeat` word is special; essentially a macro. It does the following:

1. Determines whether the count is a `lit16` (3 bytes back) or a `lit8` (2 bytes back)
2. Shifts everything right 4 or 5 bytes, moving the literal to the front, followed by a `pushr`.
3. Compiles in a `next` followed by the length of the loop (back to the start - just after the `pushr`)

## Literals

Literal values from 0 to 360 are *very* commonly used. Simple single-byte literals are of the form `lit8 123`. A `lit9` instruction allows for 256 to 511 as two-byte forms (a nine-bit values with high bit assumed set). A `lit16` instruction allows for 0-65535 as a three-byte form.

Internally, `Int16`s are used on the stack.

Literals are always positive but a `NEG` instruction allows for negation.

## Example

`10 forward 10 right 36 repeat` draws a circle. The actual instruction sequence is `lit8 10 forward lit8 10 right lit8 36 repeat 8`. Notice that `repeat` takes a parameter from the stack (`36`) and _also_ an operand (`8`) specifying how far back to jump. This is not specified by the user, but implied by the beginning of the current sequence.

`100 forward 90 right 4 repeat` draws a square. The actual instructions are `lit8 100 forward lit8 90 right lit8 4 repeat 8`.

If these are then saved to a new tiles, called `circle` and `square`, they may be used in further sequences such as `circle square 20 right 18 repeat`. The instruction sequence will simply be the concatenation of `circle` with `square` followed by the remainder. THe `repeat` instructions within will be relative and will jump to the start of the original definition, while the final one will jump to the start of the whole program: 10 forward 10 right 36 repeat 100 forward 90 right 4 repeat lit8 20 right lit8 18 repeat 17`. The total program is now 19 bytes. The maximum that can be stored on a single tile is 256.
