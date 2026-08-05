#ifndef TARGETCODE_H
#define TARGETCODE_H

/* Phase 6: Target Code Generation.
   Translates optimized TAC lines into simple pseudo-assembly for a
   hypothetical register machine and prints it to stdout. */
void generate_target_code(char **lines, int count);

#endif
