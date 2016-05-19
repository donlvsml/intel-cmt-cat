/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2016 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cpu.h"
#include "common.h"

static cpu_set_t m_cpuset;

int
parse_cpu(const char *cpu)
{
	char *cpu_str = NULL;
	int ret = 0;

	if (NULL == cpu)
		return -EINVAL;

	while (isblank(*cpu))
		cpu++;

	/* Add enclosing delimiters "(" and ")", required by parse_set(...) */
	int len = strlen(cpu) + strlen("()") + 1;

	cpu_str = malloc(len);

	if (NULL != cpu_str)
		snprintf(cpu_str, len, "(%s)", cpu);
	else
		return -EINVAL;

	/* Parse CPU set string*/
	CPU_ZERO(&m_cpuset);
	if (-1 == parse_cpu_set(cpu_str, &m_cpuset))
		ret = -EINVAL;

	if (NULL != cpu_str)
		free(cpu_str);

	return ret;
}

int
set_affinity(pid_t pid)
{
	int ret = 0;

	/* Is affinity configured ? */
	if (0 == CPU_COUNT(&m_cpuset)) {
		if (g_verbose) {
			printf("CPU affinity, CPU list empty, "
				"nothing to do...\n");
		}

		return 0;
	}

	/* Set affinity */
#ifdef __linux__
	ret = sched_setaffinity(pid, sizeof(m_cpuset), &m_cpuset);
#endif
#ifdef __FreeBSD__
	/* Current thread */
	if (0 == pid)
		ret = cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1,
				sizeof(m_cpuset), &m_cpuset);
	/* Process via PID */
	else
		ret = cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_PID, pid,
						sizeof(m_cpuset), &m_cpuset);
#endif

	return ret;
}

void
print_cmd_line_cpu_config(void)
{
	char cpustr[CPU_SETSIZE * 3] = { 0 };

	if (0 != CPU_COUNT(&m_cpuset)) {
		cpuset_to_str(cpustr, sizeof(cpustr), &m_cpuset);
		printf("Core Affinity: CPUs: %s\n", cpustr);
	}
}