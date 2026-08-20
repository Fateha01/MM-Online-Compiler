# Deploying MM_compiler to Vercel

## What changed for Vercel

- `vercel.json` (repo root) — tells Vercel: `web/client/` is a static site,
  `web/server/server.js` is a serverless function, and the compiler
  binaries + example files must be bundled into that function.
- `web/server/server.js` — now exports the Express `app` instead of always
  calling `app.listen()`. Locally (`npm start`) it still runs exactly the
  same as before. It also now runs the compiler binary from `web/bin/`
  instead of `c/`, `cpp/`, `java/`, `python/`.
- `web/bin/mm_c`, `web/bin/mm_cpp`, `web/bin/mm_java`, `web/bin/mm_python`
  — **new**, standalone **statically linked** copies of the 4 compiler
  binaries (no glibc dependency, so they run correctly on Vercel's Linux
  runtime even though it differs from your local machine). Your original
  `c/`, `cpp/`, `java/`, `python/` folders — and the binaries inside them
  used for `make`/viva/grading — are **not touched or modified**.

## Deploy steps

1. **Root Directory** in Vercel project settings must be the repo root
   (the folder containing `vercel.json`, `c/`, `cpp/`, `java/`, `python/`,
   `web/`) — **not** `web`. Leave it blank / ".".
2. Framework Preset: **Other** (not Next.js / CRA — this isn't a JS
   framework project).
3. Add these files/folders to your repo (everything else is untouched):
   - `vercel.json`
   - `web/server/server.js` (replace)
   - `web/bin/mm_c`, `web/bin/mm_cpp`, `web/bin/mm_java`, `web/bin/mm_python` (new)
   - Binaries **must stay executable** — if you use git, run
     `git update-index --chmod=+x web/bin/mm_c web/bin/mm_cpp web/bin/mm_java web/bin/mm_python`
     before committing, otherwise the +x bit can get lost on push.
4. Redeploy. Visit the deployment URL — should load the same blueprint UI.

## Known limits on Vercel (Hobby plan)

- Serverless function timeout: 10s (our `RUN_TIMEOUT_MS` is 8s — fine).
- Cold starts add a little delay on the first request after idling.
- `/tmp` is the only writable path — already what `server.js` uses for
  temp source files, so no change needed there.

## If it still 404s

- Double check **Root Directory** in Vercel → Settings → General.
- Check the Vercel deployment's **Build Logs** — confirm all 4 binaries
  in `web/bin/` show up under "Included files" for the `server.js` function.
- Check **Function Logs** for the actual runtime error if a compile
  request fails (e.g. permission denied → the +x bit was lost on push).
