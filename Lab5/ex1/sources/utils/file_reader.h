#pragma once

#include <stdio.h>
#include "../headers/pipe_args.h"

PipeArgs** preprocessLineOfFile(FILE *stream);
void freePipeArgs(PipeArgs** args);