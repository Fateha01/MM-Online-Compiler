// Tiny interpreter for the TAC grammar emitted by codegen.c / optimizer.c.
//
// Instruction forms (one per line):
//   label:
//   goto LABEL
//   ifFalse <place> goto LABEL
//   print <place>
//   <dest> = <place>
//   <dest> = <unop> <place>            (unop: ! neg)
//   <dest> = <place> <binop> <place>   (binop: + - * / % < > <= >= == != && ||)
//
// <place> is a literal (int, float, true/false, "string"), an identifier,
// or a compiler temp (t0, t1, ...). Tokenized with quote-awareness so string
// literals containing spaces don't break parsing.

const MAX_STEPS = 200000;

const TOKEN_RE = /"[^"]*"|\S+/g;
function tokenize(rhs) {
  return rhs.match(TOKEN_RE) || [];
}

function literalOrLookup(tok, env) {
  if (tok === "true") return true;
  if (tok === "false") return false;
  if (/^"[^"]*"$/.test(tok)) return tok.slice(1, -1);
  if (/^-?\d+$/.test(tok)) return parseInt(tok, 10);
  if (/^-?\d+\.\d+$/.test(tok)) return parseFloat(tok);
  if (Object.prototype.hasOwnProperty.call(env, tok)) return env[tok];
  return undefined; // unknown identifier — shouldn't happen post semantic-check
}

function applyBinop(op, a, b) {
  switch (op) {
    case "+": return a + b;
    case "-": return a - b;
    case "*": return a * b;
    case "/": return a / b;
    case "%": return a % b;
    case "<": return a < b;
    case ">": return a > b;
    case "<=": return a <= b;
    case ">=": return a >= b;
    case "==": return a === b;
    case "!=": return a !== b;
    case "&&": return Boolean(a) && Boolean(b);
    case "||": return Boolean(a) || Boolean(b);
    default: throw new Error(`unknown binary op "${op}"`);
  }
}

function applyUnop(op, a) {
  if (op === "neg") return -a;
  if (op === "!") return !a;
  throw new Error(`unknown unary op "${op}"`);
}

function formatValue(v) {
  if (typeof v === "boolean") return v ? "true" : "false";
  if (typeof v === "number") return String(v);
  return String(v);
}

/**
 * @param {string[]} lines  optimized (or raw) TAC lines, comments already stripped
 * @returns {{ output: string[], error: string|null, steps: number }}
 */
function runTAC(lines) {
  const clean = lines
    .map((l) => l.trim())
    .filter((l) => l.length > 0 && !l.startsWith("("));

  const labels = {};
  clean.forEach((l, i) => {
    if (/^[A-Za-z_]\w*:$/.test(l)) labels[l.slice(0, -1)] = i;
  });

  const env = {};
  const output = [];
  let pc = 0;
  let steps = 0;

  try {
    while (pc < clean.length) {
      if (++steps > MAX_STEPS) {
        return { output, error: `Execution aborted: exceeded ${MAX_STEPS} steps (possible infinite loop).`, steps };
      }
      const line = clean[pc];

      if (/^[A-Za-z_]\w*:$/.test(line)) { pc++; continue; }

      let m;
      if ((m = line.match(/^goto\s+(\S+)$/))) {
        const target = labels[m[1]];
        if (target === undefined) throw new Error(`unknown label "${m[1]}"`);
        pc = target;
        continue;
      }

      if ((m = line.match(/^ifFalse\s+(\S+)\s+goto\s+(\S+)$/))) {
        const cond = literalOrLookup(m[1], env);
        const target = labels[m[2]];
        if (target === undefined) throw new Error(`unknown label "${m[2]}"`);
        pc = Boolean(cond) ? pc + 1 : target;
        continue;
      }

      if ((m = line.match(/^print\s+(.+)$/))) {
        const val = literalOrLookup(m[1].trim(), env);
        output.push(formatValue(val));
        pc++;
        continue;
      }

      if ((m = line.match(/^([A-Za-z_]\w*)\s*=\s*(.+)$/))) {
        const dest = m[1];
        const rhsTokens = tokenize(m[2]);
        let value;
        if (rhsTokens.length === 1) {
          value = literalOrLookup(rhsTokens[0], env);
        } else if (rhsTokens.length === 2) {
          value = applyUnop(rhsTokens[0], literalOrLookup(rhsTokens[1], env));
        } else if (rhsTokens.length === 3) {
          value = applyBinop(rhsTokens[1], literalOrLookup(rhsTokens[0], env), literalOrLookup(rhsTokens[2], env));
        } else {
          throw new Error(`cannot parse instruction: "${line}"`);
        }
        env[dest] = value;
        pc++;
        continue;
      }

      throw new Error(`cannot parse instruction: "${line}"`);
    }
  } catch (err) {
    return { output, error: err.message, steps };
  }

  return { output, error: null, steps };
}

module.exports = { runTAC };
