#ifndef OPTIMIZER_H
#define OPTIMIZER_H

/* Phase 5: Code Optimization.
   Runs copy-propagation + dead-temporary elimination over a TAC line
   array produced by codegen_generate(). Returns a newly allocated
   array of newly allocated strings (free with codegen_free_lines()).
   *out_count receives the number of lines in the optimized program.
   *out_removed receives how many redundant instructions were removed. */
char **optimize_tac(char **lines, int count, int *out_count, int *out_removed);

#endif
