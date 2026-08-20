// MM_compiler web backend
// Spawns the compiled language frontends (mm_c, mm_cpp, mm_java, mm_python)
// on user-submitted source, and returns the 6-phase output as structured JSON.

const express = require("express");
const cors = require("cors");
const { spawn } = require("child_process");
const fs = require("fs");
const os = require("os");
const path = require("path");
const crypto = require("crypto");
const { runTAC } = require("./tacInterpreter");

const app = express();
app.use(cors());
app.use(express.json({ limit: "256kb" }));

const PORT = process.env.PORT || 4000;
const IS_WIN = process.platform === "win32";
const ROOT = path.resolve(__dirname, "..", ".."); // MM_compiler/

// language -> { dir, binName, ext, sample }
const LANGS = {
  c: { dir: "c", bin: "mm_c", ext: ".mmc", label: "C" },
  cpp: { dir: "cpp", bin: "mm_cpp", ext: ".mmcpp", label: "C++" },
  java: { dir: "java", bin: "mm_java", ext: ".mmjava", label: "Java" },
  python: { dir: "python", bin: "mm_python", ext: ".mmpy", label: "Python" },
};

const MAX_SOURCE_CHARS = 20000;
const RUN_TIMEOUT_MS = 8000;

function binaryPath(langKey) {
  const l = LANGS[langKey];
  const name = IS_WIN ? `${l.bin}.exe` : l.bin;
  // Deployment-only static copies live in web/bin/ so the original
  // c/, cpp/, java/, python/ folders (used for local `make`/viva/grading)
  // never need to be touched or overwritten.
  return path.join(ROOT, "web", "bin", name);
}

// Split combined stdout+stderr text into the six labelled phase blocks.
function parsePhases(text) {
  const markerRe = /^=== (Phase \d+): (.+?) ===\s*$/gm;
  const hits = [];
  let m;
  while ((m = markerRe.exec(text)) !== null) {
    hits.push({ index: m.index, headerEnd: markerRe.lastIndex, num: m[1], title: m[2].trim() });
  }
  const phases = [];
  for (let i = 0; i < hits.length; i++) {
    const start = hits[i].headerEnd;
    const end = i + 1 < hits.length ? hits[i + 1].index : text.length;
    phases.push({
      id: Number(hits[i].num.replace(/\D/g, "")),
      title: hits[i].title,
      content: text.slice(start, end).trim(),
    });
  }
  return phases;
}

function firstErrorLine(stderrText) {
  const m = stderrText.match(/^(Lexical|Syntax|Semantic)\s*Error.*$/m);
  return m ? m[0].trim() : null;
}

app.get("/api/languages", (_req, res) => {
  const out = Object.entries(LANGS).map(([key, v]) => ({ key, label: v.label, ext: v.ext }));
  res.json(out);
});

app.get("/api/example/:lang/:name", (req, res) => {
  const l = LANGS[req.params.lang];
  if (!l) return res.status(400).json({ error: "unknown language" });
  const safeName = path.basename(req.params.name);
  const file = path.join(ROOT, l.dir, "examples", safeName);
  if (!file.startsWith(path.join(ROOT, l.dir, "examples")) || !fs.existsSync(file)) {
    return res.status(404).json({ error: "example not found" });
  }
  res.json({ code: fs.readFileSync(file, "utf8") });
});

app.get("/api/examples/:lang", (req, res) => {
  const l = LANGS[req.params.lang];
  if (!l) return res.status(400).json({ error: "unknown language" });
  const dir = path.join(ROOT, l.dir, "examples");
  if (!fs.existsSync(dir)) return res.json([]);
  res.json(fs.readdirSync(dir).filter((f) => f.endsWith(l.ext)));
});

app.post("/api/compile", (req, res) => {
  const { language, code } = req.body || {};
  const lang = LANGS[language];
  if (!lang) {
    return res.status(400).json({ error: `Unknown language "${language}". Use one of: ${Object.keys(LANGS).join(", ")}` });
  }
  if (typeof code !== "string" || code.trim().length === 0) {
    return res.status(400).json({ error: "No source code provided." });
  }
  if (code.length > MAX_SOURCE_CHARS) {
    return res.status(400).json({ error: `Source too large (max ${MAX_SOURCE_CHARS} chars).` });
  }

  const bin = binaryPath(language);
  if (!fs.existsSync(bin)) {
    return res.status(500).json({
      error: `Compiler binary not built: ${bin}. It should live in web/bin/ — see web/README.md.`,
    });
  }

  const tmpFile = path.join(os.tmpdir(), `mmc_${crypto.randomBytes(6).toString("hex")}${lang.ext}`);
  fs.writeFileSync(tmpFile, code, "utf8");

  const child = spawn(bin, [tmpFile], { cwd: path.join(ROOT, "web", "bin") });
  let stdout = "";
  let stderr = "";
  let timedOut = false;

  const killer = setTimeout(() => {
    timedOut = true;
    child.kill("SIGKILL");
  }, RUN_TIMEOUT_MS);

  child.stdout.on("data", (d) => (stdout += d.toString()));
  child.stderr.on("data", (d) => (stderr += d.toString()));

  child.on("close", (exitCode) => {
    clearTimeout(killer);
    try { fs.unlinkSync(tmpFile); } catch (_) {}

    if (timedOut) {
      return res.status(504).json({ error: "Compilation timed out." });
    }

    const phases = parsePhases(stdout + "\n" + stderr);

    let runtimeOutput = null;
    let runtimeError = null;
    if (exitCode === 0) {
      const optimized = phases.find((p) => p.id === 5);
      const ir = phases.find((p) => p.id === 4);
      const tacText = (optimized || ir) ? (optimized || ir).content : "";
      const tacLines = tacText.split("\n").filter((l) => l.trim().length > 0);
      const result = runTAC(tacLines);
      runtimeOutput = result.output;
      runtimeError = result.error;
    }

    res.json({
      ok: exitCode === 0,
      exitCode,
      language,
      phases,
      errorSummary: exitCode === 0 ? null : (firstErrorLine(stderr) || "Compilation failed."),
      runtimeOutput,
      runtimeError,
      raw: { stdout, stderr },
    });
  });

  child.on("error", (err) => {
    clearTimeout(killer);
    try { fs.unlinkSync(tmpFile); } catch (_) {}
    res.status(500).json({ error: `Failed to run compiler: ${err.message}` });
  });
});

app.use(express.static(path.join(__dirname, "..", "client")));

// Local dev: run as a normal long-lived server.
// Vercel: this file is imported by @vercel/node, which wraps `module.exports`
// as the request handler — it must NOT call app.listen() in that case.
if (require.main === module) {
  app.listen(PORT, () => {
    console.log(`MM_compiler web backend on http://localhost:${PORT}`);
    console.log(`Frontend served from ${path.join(__dirname, "..", "client")}`);
  });
}

module.exports = app;
