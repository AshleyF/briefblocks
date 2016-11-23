# Brief Turtle Language

# Bytecode VM

The VM is a stack machine. Zero-operand instructions consume and produce values on a shared stack. An additional stack contains return addresses for nesting calls.

A program counter (`p`) into memory is maintained. Bytecodes are read through this pointer and interpreted as either single-byte instructions or two-bytes calls. If the leading bit is set, then this along with the next byte form an address to call (sans-leading bit).

    void run() {
      int16_t i;
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

    void exec(int16_t address) {
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
| `dup`    | `13` | `int16_t a = pop(); push(a); push(a)` |
| `swap`   | `14` | `int16_t a = pop(); int16_t b = pop(); push(a); push(b)` |
| `repeat` | `15` | `` |
| `forward` | `16`| `double d = pop(); x += d * cos(theta / 180.0 * PI); y += d * sin(theta / 180.0 * PI)` |
| `turn`   | `17` | `theta += pop()` |

Another way to think of the instruction stream is as a simple stream of integers with maximum value 2^16. Values below 128 are instructions. Over values are addresses to call.