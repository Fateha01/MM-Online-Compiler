#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "optimizer.h"

static int is_temp_token(const char *s) {
    if (s[0] != 't' || !isdigit((unsigned char)s[1])) return 0;
    for (const char *p = s + 1; *p; p++) if (!isdigit((unsigned char)*p)) return 0;
    return 1;
}

/* counts whole-word occurrences of `tok` across all lines except line `skip` */
static int count_uses(char **lines, int count, int skip, const char *tok) {
    int n = 0;
    for (int i = 0; i < count; i++) {
        if (i == skip) continue;
        char tmp[256];
        strncpy(tmp, lines[i], sizeof tmp - 1);
        tmp[sizeof tmp - 1] = 0;
        char *w = strtok(tmp, " ");
        while (w) {
            size_t l = strlen(w);
            if (l && w[l - 1] == ':') {
                char lbl[64];
                if (l - 1 < sizeof lbl) { strncpy(lbl, w, l - 1); lbl[l - 1] = 0; if (!strcmp(lbl, tok)) n++; }
            } else if (!strcmp(w, tok)) n++;
            w = strtok(NULL, " ");
        }
    }
    return n;
}

char **optimize_tac(char **lines, int count, int *out_count, int *out_removed) {
    int *keep = calloc(count, sizeof(int));
    char **replace = calloc(count, sizeof(char *));
    for (int i = 0; i < count; i++) keep[i] = 1;

    for (int i = 0; i < count; i++) {
        char *eq = strstr(lines[i], " = ");
        if (!eq) continue;
        size_t llen = eq - lines[i];
        if (llen == 0 || llen >= 64) continue;
        char lhs[64];
        strncpy(lhs, lines[i], llen); lhs[llen] = 0;
        const char *rhs = eq + 3;
        if (strchr(rhs, ' ')) continue;      /* only pure "A = B" copies qualify */
        if (!is_temp_token(rhs)) continue;   /* B must be a compiler temp */

        for (int j = 0; j < count; j++) {
            if (j == i) continue;
            char *eq2 = strstr(lines[j], " = ");
            if (!eq2) continue;
            size_t l2 = eq2 - lines[j];
            if (l2 != strlen(rhs) || strncmp(lines[j], rhs, l2) != 0) continue;
            /* lines[j] is the definition of temp `rhs`; safe to fold+drop
               only if that temp is used nowhere else */
            if (count_uses(lines, count, j, rhs) == 1) {
                char buf[256];
                snprintf(buf, sizeof buf, "%s = %s", lhs, eq2 + 3);
                replace[i] = strdup(buf);
                keep[j] = 0;
            }
            break;
        }
    }

    char **out = malloc(count * sizeof(char *));
    int oc = 0, removed = 0;
    for (int i = 0; i < count; i++) {
        if (!keep[i]) { removed++; continue; }
        out[oc++] = strdup(replace[i] ? replace[i] : lines[i]);
    }
    for (int i = 0; i < count; i++) free(replace[i]);
    free(replace); free(keep);

    *out_count = oc;
    *out_removed = removed;
    return out;
}
