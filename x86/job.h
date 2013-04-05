
/*
 * PM2: Parallel Multithreaded Machine
 * Copyright (C) 2001 "the PM2 team" (see AUTHORS file)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

extern void init_queue (TSPqueue *q) ;

extern int empty_queue (TSPqueue q) ;

extern void add_job (TSPqueue *q, Job_t j)  ;

extern int get_job (TSPqueue *q, Job_t *j) ;

extern void no_more_jobs (TSPqueue *q) ;
