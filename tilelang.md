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

## Literals

Literal values from 0 to 360 are *very* commonly used. Simple single-byte literals are of the form `LIT8 123`. A `LIT9` instruction allows for 256 to 511 as two-byte forms (a nine-bit values with high bit assumed set). A `LIT16` instruction allows for 0-65535 as a three-byte form.

Internally, `Int16`s are used on the stack.

Literals are always positive but a `NEG` instruction allows for negation.

## Example

`10 FORWARD 10 RIGHT 36 REPEAT` draws a circle. The actual instruction sequence is `LIT 10 FORWARD LIT 10 RIGHT LIT 36 REPEAT 8`. Notice that `REPEAT` takes a parameter from the stack (`36`) and _also_ an operand (`8`) specifying how far back to jump. This is not specified by the user, but implied by the beginning of the current sequence.

`100 FORWARD 90 RIGHT 4 REPEAT` draws a square. The actual instructions are `LIT 100 FORWARD LIT 90 RIGHT LIT 4 REPEAT 8`.

If these are then saved to a new tiles, called `CIRCLE` and `SQUARE`, they may be used in further sequences such as `CIRCLE SQUARE 20 RIGHT 18 REPEAT`. The instruction sequence will simply be the concatenation of `CIRCLE` with `SQUARE` followed by the remainder. THe `REPEAT` instructions within will be relative and will jump to the start of the original definition, while the final one will jump to the start of the whole program: 10 FORWARD 10 RIGHT 36 REPEAT 100 FORWARD 90 RIGHT 4 REPEAT LIT 20 RIGHT LIT 18 REPEAT 17`. The total program is now 19 bytes. The maximum that can be stored on a single tile is 256.
