#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "centrality.h"

unsigned short
total_contribute(struct contribute *contributors)
{
  unsigned short total = 0;

  if (!contributors)
    return 0;
  struct contribute *contributor = contributors;
  while (contributor->next) {
    contributor=contributor->next;
    total = total + contributor->contribute;
  }
  return total;
}
