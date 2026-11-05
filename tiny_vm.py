#!/usr/bin/env python3
"""Stack-based Tiny VM for project 3's three-address code."""
from __future__ import annotations

import sys
from dataclasses import dataclass


@dataclass
class Instruction:
    pc: int
    op: str
    arg: str | None = None


def read_code(path: str) -> list[Instruction]:
    result: list[Instruction] = []
    with open(path, encoding="utf-8") as stream:
        for line in stream:
            line = line.strip()
            if not line or line.startswith("#"): continue
            pc_text, rest = line.split(":", 1)
            parts = rest.strip().split(maxsplit=1)
            result.append(Instruction(int(pc_text), parts[0], parts[1] if len(parts) > 1 else None))
    return result


class VM:
    def __init__(self, code: list[Instruction], trace: bool = False):
        self.code, self.trace = code, trace
        self.stack: list[int] = []
        self.variables: dict[str, int] = {}
        self.pc = 0
        self.output: list[int] = []

    def pop(self) -> int:
        if not self.stack: raise RuntimeError("stack underflow")
        return self.stack.pop()

    def run(self) -> list[int]:
        steps = 0
        while 0 <= self.pc < len(self.code):
            ins = self.code[self.pc]; steps += 1
            if steps > 100000: raise RuntimeError("instruction limit exceeded; possible infinite loop")
            if self.trace: print(f"pc={self.pc:04d} {ins.op:<6} {ins.arg or '':<8} stack={self.stack} vars={self.variables}")
            op, arg = ins.op, ins.arg
            if op == "DECL": self.variables[arg] = 0
            elif op == "CONST": self.stack.append(int(arg))
            elif op == "LOAD":
                if arg not in self.variables: raise RuntimeError(f"unknown variable {arg}")
                self.stack.append(self.variables[arg])
            elif op == "STORE": self.variables[arg] = self.pop()
            elif op in {"ADD", "SUB", "MUL", "DIV", "GT", "LT", "EQ"}:
                b, a = self.pop(), self.pop()
                if op == "ADD": self.stack.append(a + b)
                elif op == "SUB": self.stack.append(a - b)
                elif op == "MUL": self.stack.append(a * b)
                elif op == "DIV":
                    if b == 0: raise RuntimeError("division by zero")
                    self.stack.append(int(a / b))
                elif op == "GT": self.stack.append(int(a > b))
                elif op == "LT": self.stack.append(int(a < b))
                elif op == "EQ": self.stack.append(int(a == b))
            elif op == "JZ":
                if self.pop() == 0: self.pc = int(arg); continue
            elif op == "JMP": self.pc = int(arg); continue
            elif op == "PRINT": self.output.append(self.pop()); print(self.output[-1])
            elif op == "LABEL": pass
            elif op == "HALT": break
            else: raise RuntimeError(f"unknown opcode {op}")
            self.pc += 1
        return self.output


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: python3 tiny_vm.py program.tac [--trace]", file=sys.stderr); raise SystemExit(2)
    try: VM(read_code(sys.argv[1]), "--trace" in sys.argv[2:]).run()
    except (OSError, ValueError, RuntimeError) as exc:
        print(f"VM error: {exc}", file=sys.stderr); raise SystemExit(1)


if __name__ == "__main__": main()
