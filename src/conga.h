#pragma once

#include "config.h"

typedef struct _Conga Conga;

void    conga_startup  (void);
void    conga_shutdown (void);
Conga * conga_new      (const Config *c);
void    conga_run      (Conga *g);
void    conga_free     (Conga *g);
