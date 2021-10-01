#include "r0e.h"
#include <stdio.h>
#include <string.h>

typedef void (*line_cb)(char *, int, void *);

void file_foreach_line(char *fname, line_cb cb, void *arg) {
  FILE *f = fopen(fname, "r");
  if (!f) {
    return;
  }
  size_t length = 0;
  char *content = NULL;
  int line_count = 0;
  do {
    int success = getline(&content, &length, f);
    if (success == -1) {
      break;
    }
    cb(content, line_count, arg);
    line_count++;
  } while (1);
  fclose(f);
}

typedef struct {
  const char *name;
  size_t addr;
} kallsyms_symbol;

void kallsyms_parse_symbol(char *line, int linenr, void *addr) {
  char *saddr = strtok(line, " ");
  char *type = strtok(NULL, " ");
  char *name = strtok(NULL, " \n\r");
  kallsyms_symbol *sym = (kallsyms_symbol *)addr;
  if (!strcmp(name, sym->name)) {
    sym->addr = strtoull(saddr, NULL, 16);
  }
}

size_t kallsyms_lookup(const char *symbol) {
  kallsyms_symbol symaddr;
  symaddr.name = symbol;
  symaddr.addr = -1;
  file_foreach_line("/proc/kallsyms", kallsyms_parse_symbol, &symaddr);
  return symaddr.addr;
}

volatile void *addr = 0;

size_t read_mem() { 
    return *(size_t *)addr; 
}

int main() {
  if (r0e_init()) {
    printf("Could not initialize r0e\n");
    return 1;
  }
  addr = (void *)kallsyms_lookup("page_offset_base");
  printf("[r0e] Get direct-physical-map offset from 'page_offset_base'\n");
  if (addr != 0) {
    printf("[r0e] Reading kernel location %p\n", addr);
    size_t value = r0e_call(read_mem);
    printf("[r0e] Direct-physical-map offset: 0x%zx\n", value);
  } else {
    printf("[r0e] Could not get offset position, did you start as root?\n");
  }
  r0e_cleanup();
  return 0;
}
