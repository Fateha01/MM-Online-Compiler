const API = ""; // same-origin; set e.g. "http://localhost:4000" if serving client separately

const LANGS = {
  c:      { label: "C",      ext: ".mmc" },
  cpp:    { label: "C++",    ext: ".mmcpp" },
  java:   { label: "Java",   ext: ".mmjava" },
  python: { label: "Python", ext: ".mmpy" },
};

// Canonical 6-phase pipeline (must match main.c phase titles).
const PHASES = [
  { id: 1, title: "Lexical Analysis" },
  { id: 2, title: "Syntax Analysis" },
  { id: 3, title: "Semantic Analysis" },
  { id: 4, title: "Intermediate Code Generator" },
  { id: 5, title: "Code Optimization" },
  { id: 6, title: "Target Code Generation" },
];

const STARTER = {
  c: `int x = 5;\nint sum = 0;\nfor (int i = 0; i < x; i = i + 1) {\n    sum = sum + i;\n}\nprint sum;\n`,
  cpp: `int x = 5;\nint sum = 0;\nfor (int i = 0; i < x; i = i + 1) {\n    sum = sum + i;\n}\ncout << sum;\n`,
  java: `class Main {\n    public static void main(String[] args) {\n        int x = 5;\n        int sum = 0;\n        for (int i = 0; i < x; i = i + 1) {\n            sum = sum + i;\n        }\n        System.out.println(sum);\n    }\n}\n`,
  python: `x: int = 5\nsum: int = 0\nfor i in range(0, x):\n    sum = sum + i\nprint(sum)\n`,
};

let currentLang = "c";

const el = (id) => document.getElementById(id);
const codeEl = el("code");
const gutterEl = el("gutter");
const fileLabelEl = el("fileLabel");
const exampleSelectEl = el("exampleSelect");
const compileBtn = el("compileBtn");
const phaseListEl = el("phaseList");
const tracePulseEl = el("tracePulse");
const errorBannerEl = el("errorBanner");
const metaLangEl = el("metaLang");
const metaStatusEl = el("metaStatus");
const metaExitEl = el("metaExit");
const consoleBodyEl = el("consoleBody");
const consoleExitEl = el("consoleExit");

function setLang(lang) {
  currentLang = lang;
  document.querySelectorAll(".langbtn").forEach((b) => b.classList.toggle("is-active", b.dataset.lang === lang));
  fileLabelEl.textContent = "source" + LANGS[lang].ext;
  metaLangEl.textContent = LANGS[lang].label.toUpperCase();
  if (!codeEl.value.trim()) codeEl.value = STARTER[lang];
  syncGutter();
  loadExampleList(lang);
}

function syncGutter() {
  const lines = codeEl.value.split("\n").length;
  let out = "";
  for (let i = 1; i <= lines; i++) out += i + "\n";
  gutterEl.textContent = out;
}

codeEl.addEventListener("input", syncGutter);
codeEl.addEventListener("scroll", () => { gutterEl.scrollTop = codeEl.scrollTop; });
codeEl.addEventListener("keydown", (e) => {
  if (e.key === "Tab") {
    e.preventDefault();
    const s = codeEl.selectionStart, en = codeEl.selectionEnd;
    codeEl.value = codeEl.value.slice(0, s) + "    " + codeEl.value.slice(en);
    codeEl.selectionStart = codeEl.selectionEnd = s + 4;
    syncGutter();
  }
});

document.querySelectorAll(".langbtn").forEach((b) => {
  b.addEventListener("click", () => {
    codeEl.value = ""; // clear so the new language's starter loads
    setLang(b.dataset.lang);
  });
});

async function loadExampleList(lang) {
  exampleSelectEl.innerHTML = '<option value="">load example&hellip;</option>';
  try {
    const res = await fetch(`${API}/api/examples/${lang}`);
    if (!res.ok) return;
    const names = await res.json();
    for (const n of names) {
      const opt = document.createElement("option");
      opt.value = n;
      opt.textContent = n;
      exampleSelectEl.appendChild(opt);
    }
  } catch (_) { /* backend not reachable yet — ignore */ }
}

exampleSelectEl.addEventListener("change", async () => {
  const name = exampleSelectEl.value;
  if (!name) return;
  try {
    const res = await fetch(`${API}/api/example/${currentLang}/${name}`);
    const data = await res.json();
    if (data.code !== undefined) {
      codeEl.value = data.code;
      syncGutter();
    }
  } catch (_) {}
});

function renderPipeline(resultPhases, ok) {
  phaseListEl.innerHTML = "";
  phaseListEl.appendChild(tracePulseEl); // keep the pulse element in the DOM

  const gotIds = new Set((resultPhases || []).map((p) => p.id));
  const failId = !ok && resultPhases && resultPhases.length ? resultPhases[resultPhases.length - 1].id : null;

  for (const phase of PHASES) {
    const found = (resultPhases || []).find((p) => p.id === phase.id);
    const li = document.createElement("li");
    li.className = "phase";

    let status = "notreached";
    if (found) status = phase.id === failId ? "fail" : "pass";
    li.classList.add(status === "pass" ? "is-pass" : status === "fail" ? "is-fail" : "is-notreached");

    const head = document.createElement("button");
    head.className = "phase__head";
    head.disabled = status === "notreached";
    head.innerHTML = `
      <span class="phase__num">0${phase.id}</span>
      <span class="phase__title">${phase.title}</span>
      <span class="phase__tag">${status === "pass" ? "PASS" : status === "fail" ? "FAILED" : "NOT REACHED"}</span>
      <span class="phase__chevron">&#9656;</span>
    `;

    const body = document.createElement("div");
    body.className = "phase__body";
    const pre = document.createElement("pre");
    pre.className = "phase__pre";
    pre.textContent = found ? found.content : "";
    body.appendChild(pre);

    if (status !== "notreached") {
      head.addEventListener("click", () => {
        const wasOpen = li.classList.contains("is-open");
        document.querySelectorAll(".phase.is-open").forEach((p) => p.classList.remove("is-open"));
        if (!wasOpen) li.classList.add("is-open");
      });
    }

    li.appendChild(document.createElement("span")).className = "phase__led";
    li.appendChild(head);
    li.appendChild(body);
    phaseListEl.appendChild(li);
  }
}

function renderConsole({ lines, errorText, exitLabel, kind }) {
  consoleBodyEl.classList.toggle("is-error", !!errorText);
  consoleBodyEl.innerHTML = "";

  if (errorText) {
    const span = document.createElement("span");
    span.className = "line";
    span.textContent = errorText;
    consoleBodyEl.appendChild(span);
  } else if (lines && lines.length) {
    for (const l of lines) {
      const span = document.createElement("span");
      span.className = "line";
      span.textContent = l;
      consoleBodyEl.appendChild(span);
    }
  } else if (lines) {
    const span = document.createElement("span");
    span.textContent = "(program produced no output)";
    consoleBodyEl.appendChild(span);
  } else {
    consoleBodyEl.textContent = "// click COMPILE to run source.mmc through the pipeline";
  }

  const cursor = document.createElement("span");
  cursor.className = "console__cursor";
  consoleBodyEl.appendChild(cursor);

  consoleExitEl.textContent = exitLabel || "not run yet";
  consoleExitEl.className = "console__exit" + (kind ? ` is-${kind}` : "");
}

function setStatus(text, kind) {
  metaStatusEl.textContent = text;
  metaStatusEl.className = "stampfield__value" + (kind ? ` is-${kind}` : "");
}

async function compile() {
  const code = codeEl.value;
  errorBannerEl.hidden = true;
  compileBtn.disabled = true;
  tracePulseEl.classList.add("is-active");
  setStatus("COMPILING", "busy");
  metaExitEl.textContent = "\u2014";
  renderPipeline([], true); // reset stations to not-reached while running
  renderConsole({ lines: null, errorText: null, exitLabel: "running\u2026", kind: "busy" });

  try {
    const res = await fetch(`${API}/api/compile`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ language: currentLang, code }),
    });
    const data = await res.json();

    if (!res.ok && data.error) {
      setStatus("ERROR", "fail");
      errorBannerEl.textContent = data.error;
      errorBannerEl.hidden = false;
      renderPipeline([], false);
      renderConsole({ lines: null, errorText: data.error, exitLabel: "error", kind: "fail" });
      return;
    }

    renderPipeline(data.phases, data.ok);
    metaExitEl.textContent = String(data.exitCode);

    if (data.ok) {
      setStatus("PASS", "pass");
      const errText = data.runtimeError ? `runtime error: ${data.runtimeError}` : null;
      renderConsole({
        lines: data.runtimeOutput || [],
        errorText: errText,
        exitLabel: `exit ${data.exitCode}`,
        kind: errText ? "fail" : "pass",
      });
    } else {
      setStatus("FAIL", "fail");
      const msg = data.errorSummary || "Compilation failed.";
      errorBannerEl.textContent = msg;
      errorBannerEl.hidden = false;
      renderConsole({ lines: null, errorText: msg, exitLabel: `exit ${data.exitCode}`, kind: "fail" });
    }
  } catch (err) {
    setStatus("OFFLINE", "fail");
    const msg = "Could not reach the backend. Is the server running (npm start in web/server)?";
    errorBannerEl.textContent = msg;
    errorBannerEl.hidden = false;
    renderPipeline([], false);
    renderConsole({ lines: null, errorText: msg, exitLabel: "offline", kind: "fail" });
  } finally {
    compileBtn.disabled = false;
    tracePulseEl.classList.remove("is-active");
  }
}

compileBtn.addEventListener("click", compile);

// init
renderPipeline([], true);
renderConsole({ lines: null, errorText: null, exitLabel: "not run yet", kind: null });
setLang("c");
