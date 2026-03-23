# IronPawn

A chess engine written in C that supports WebAssembly. Play against it at [ironpawn.vercel.app](https://ironpawn.vercel.app). You play as white.

---

## Architecture Overview

The engine compiles to a native binary (for UCI use) or to a `.wasm` module via Emscripten. This WebAssembly module exposes two functions: `wasm_init()` and `wasm_process_uci_command()`. A Next.js frontend loads the WASM module and communicates with it using a subset of the UCI protocol.

---

## Board Representation: Bitboards
A bitboard is a 64-bit unsigned integer (`uint64_t`, aliased as `BITBOARD`), which represents the state of the chess board relative to a piece's type and color.
Therefore, there is one bitboard per type per color.

Bitboards are beneficial since they provide O(1) access to important board state, whereas the traditional **mailbox** approach would require scanning the board for the same state, which
is O(64).

### Example: Bitboard vs Mailbox
**Problem**: Generate all white pawn pushes.

* **Mailbox**:
 - Loops all 64 squares, finds each pawn one by one and computes where it can go (i.e., if its blocked, etc.).
 - This is O(64) + O(branching logic).

* **Bitboards**:
 - Uses two instructions:
 ```c
 BITBOARD pawn_pushes = white_pawns << 8;   // advance each white pawn at once
 pawn_pushes &= empty_squares;              // remove blocked ones at once
 ```
 - This is O(1).

*Note: when it comes to extracting each move for move simulation, the cost is effectively the same for both approaches.
The bitboard needs to be scanned for set bits, and the move array (for mailbox) needs to be iterated.*
 - **The benefit of bitboards comes in the steps before simulation, like of that in the example above**.

### Sliding Pieces Need Some Magic
#### The Problem
Knights and kings have fixed move sets. Given a square these pieces can be on, there will be only one set of (pseudolegal) positions in which
they may move if they aren't blocked by a piece of the same color.

Pawns are slightly different, in that their move set changes depending on board occupancy, but a pawn moves in such a simplistic
way (barring en passant) that their move set logic is simple (see above, which can be expanded to double pushes and captures).

Bishops, Rooks, and Queens are different due to their complexity. Their move set heavily depends on board occupation.
For example: a rook on `e4` with a piece on `e6` cannot reach `e7` or `e8`, but move that piece to `e7` and suddenly `e6` is fully reachable.
Moves change depending on what's blocking them, so you cannot index moves on square position alone.

#### A Naive Solution
Walk each square along the rays of the piece until you hit a piece or the edge. This works but is slow since it happens millions of times a second.

#### The Idea
For a given square, there are only a finite number of ways pieces can be arranged along its rays. Precompute the legal moves for every possible arrangement
and store them in a table. At runtime, just use the table, which is an O(1) query.

The problem, now, is how to turn the current state of the board into a table index cheaply.

#### Magic Numbers
As mentioned above, there are a finite number of ways pieces can be arranged along the rays of a sliding piece.
This is a useful property since if we just used the `all_pieces` bitboard for an index, the table (represented by an array)
would be massive and sparse (64-bit => `2^64` possible values).

A **blocker mask** is precomputed per square. For a rook on `d4` it marks every square along its rays, *excluding edge squares*.
Edges are omitted because whether or not they are occupied never changes the outcome (the rook stops there regardless).

This leaves ~10 relevant squares for a rook, meaning only `2^10 = 1024` possible blocker configurations instead of `2^64`.

A **blocker** is the actual pieces sitting on those marked squares right now:
```c
blocker = blocker_mask[square] & all_pieces;
```

**Problem**: the blocker is still a 64-bit number with bits scattered across it. There is no way to use this directly as an index,
since doing so would require a table with `2^64` (~18 quintillion entries) => not possible.

There needs to be a way to compress this 64-bit integer into a much smaller range of consecutively set bits, such that it is usable as an index.

This is where a **magic number** comes into play. A magic number is just a constant that every distinct blocker pattern maps to a *unique*
index in the range 0-1023 (for rooks, bishops have a smaller range). No two different patterns collide for a given square. There is no formula to find such a number.
Instead, it is found through brute force, and hardcoded into a table with other magic numbers, one for each square.

For a magic number to be valid, it must hold the following properties:
 - All blockers for a square, when multiplied by this number, produce a unique output.
 - The set bits of the output should be packed as high as possible, maximising the shift and minimising the table size.

After a blocker has been multiplied by the magic number, it is then bit-shifted to the right by a certain amount: the shift is `64 - number_of_relevant_squares_for_that_square`.
```c
index = (blocker * MAGIC[square]) >> SHIFT[square];
moves = table[sq][index]
```

Therefore, a table of magic numbers (and their shift values) exist for rooks and bishops.

Queens reuse both tables: diagonal moves use the bishop table, straight moves use the rook table.

---

## Move Representation

Moves are packed into a `uint32_t` (`move_info_t`):

```
bits  0–5:   from_pos  (6 bits, 0–63)
bits  6–11:  to_pos    (6 bits, 0–63)
bits 12–15:  flags     (promotion, etc.)
```

Macros `GET_FROM_POS` and `GET_TO_POS` extract the fields via masking and shifting. This keeps `MoveArray` (a fixed-size stack-allocated array of moves) compact and avoids heap allocation during search.

---

## Search: Minimax with Alpha-Beta Pruning

`search.c` implements a standard negamax-style minimax search. White is the maximizing player; black is minimizing.

**Alpha-beta pruning** maintains two variables: `alpha`, `beta`. When a branch is proven to be worse than an already-found alternative, it is cut off without evaluation. This improves the performance substantially over standard minimax.

The top-level `search()` function generates pseudo-legal moves, simulates each one, verifies legality (i.e., the moving side's king is not left in check), then calls `__minimax()` recursively at `depth - 1`. Moves are undone by reversing the piece placement and restoring any captured piece.

The default search depth is **6 half-moves (plies)**.

### Evaluation

Leaf nodes are scored by `__eval()`, which combines:

- **Material**: standard piece values (pawn=100, knight/bishop=300, rook=500, queen=900, king=9,999,900).
- **Piece-square tables**: 8×8 tables per piece type per color that add bonuses for positionally favorable squares (e.g., knights prefer the center, pawns are rewarded for advancement, rooks are rewarded on the 7th rank).

The evaluation is from white's perspective: positive scores favor white, negative scores favor black.

Checkmate is scored as ±9,999,900 adjusted by remaining depth, so the engine prefers faster mates.

---

## UCI Protocol

The engine exposes a subset of UCI sufficient to drive the Next.js frontend:

| Command | Behavior |
|---|---|
| `uci` | Returns engine name/author and `uciok` |
| `position startpos` | Resets to starting position |
| `position fen <fen>` | Sets up an arbitrary position |
| `go [depth N] [movetime N] [wtime N] [btime N]` | Searches and returns `bestmove <move>` |

*Note: `movetime`, `wtime` and `btime` at this time are not used and don't impact move generation*

`go` also returns `gameover checkmate` or `gameover stalemate` when appropriate, which the frontend uses to end the game.

The WASM build exposes `wasm_process_uci_command(const char*)` which accepts a UCI string and returns the engine's response string.

---

## Known Limitations

- **No castling**: king and rook move independently; castling rights are not tracked.
- **No en passant**: pawn capture rules do not include en passant.
- **No repetition detection**: the engine does not detect threefold repetition or the fifty-move rule.
- **Promotion is always queen**: promotion moves auto-queen; underpromotion is not supported.
- **Partial UCI**: only the commands needed by the GUI are implemented; the full UCI spec is not supported.

---

## Building

### Native binary

```bash
make
./ironpawn
```

### WebAssembly (requires Emscripten)

```bash
make wasm
```

---

## Project Structure

| File | Responsibility |
|---|---|
| `ironpawn.c` | Native entry point, debug/magic-finding modes |
| `wasm_main.c` | WASM entry point |
| `bitboard.c/h` | Board init, bit ops, precomputed tables, magic finder |
| `engine.c/h` | Move generation, make/undo move, check detection |
| `search.c/h` | Minimax, alpha-beta, evaluation, position tables |
| `uci.c/h` | UCI command parsing and dispatch |
| `magic_info.c/h` | Hardcoded magic numbers and shifts |
| `utils.c/h` | `String` and `Vec` types |
