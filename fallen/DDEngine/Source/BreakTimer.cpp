// BreakTimer.cpp
//
// break timer for timing portions of code

#include <MFStdLib.h>
#if !defined(TARGET_DC)
#include <cmath>
#endif
#include "BreakTimer.h"

// Windows timer access functions

static inline ULONG GetFineTimerFreq()
{
	LARGE_INTEGER	freq;

	QueryPerformanceFrequency(&freq);

	return ULONG(freq.u.LowPart);
}

static inline ULONG GetFineTimerValue()
{
	LARGE_INTEGER	time;

	QueryPerformanceCounter(&time);

	return ULONG(time.u.LowPart);
}

// basic timing

static ULONG	stopwatch_start;

void StartStopwatch()
{
	stopwatch_start = GetFineTimerValue();
}

float StopStopwatch()
{
	ULONG	time = GetFineTimerValue() - stopwatch_start;

	float	secs = float(time) / float(GetFineTimerFreq());

	return secs;
}

#ifdef BREAKTIMER

const size_t MAX_BREAK = 64;

static size_t	next_break;
static size_t	max_break;
static ULONG	time_base;
static bool		enabled = false;

static ULONG	total_dfacets;

struct BreakPoint
{
	const char*	name;		// name of breakpoint
	ULONG		time;		// last measured breaktime
	float		min_sec;	// minimum seconds since last breakpoint
	float		max_sec;	// maximum seconds since last breakpoint
	float		tot_sec;	// total seconds measured between this and last breakpoint
	float		tot_secsq;	// total seconds measured between this and last breakpoint, squared
	ULONG		tot_cnt;	// total number of measurements
}	breakpoint[MAX_BREAK + 1];

// BreakStart
//
// start break timing

void BreakStart()
{
	time_base = GetFineTimerFreq();
	for (size_t ii = 0; ii <= MAX_BREAK; ii++)
	{
		breakpoint[ii].name = NULL;
		breakpoint[ii].time = 0;
		breakpoint[ii].min_sec = 1000000;
		breakpoint[ii].max_sec = 0;
		breakpoint[ii].tot_sec = 0;
		breakpoint[ii].tot_cnt = 0;
	}
	breakpoint[0].name = "Frame Start";
	breakpoint[0].time = GetFineTimerValue();
	enabled = true;
	next_break = 1;
	max_break = 0;
	total_dfacets = 0;
}

void BreakTime(const char* name)
{
	if (enabled)
	{
		ASSERT(next_break < MAX_BREAK);
		breakpoint[next_break].name = name;
		breakpoint[next_break++].time = GetFineTimerValue();
	}
}

void BreakFacets(ULONG dfacets)
{
	total_dfacets += dfacets;
}

void BreakFrame()
{
	if (enabled)
	{
		breakpoint[next_break].name = "Frame End";
		breakpoint[next_break].time = GetFineTimerValue();

		if (!max_break)		max_break = next_break;
		else				ASSERT(max_break == next_break);

		// get break times
		for (size_t ii = 1; ii <= next_break; ii++)
		{
			ULONG	dtime = breakpoint[ii].time - breakpoint[ii-1].time;
			float	dsec = float(dtime) / float(time_base);

			if (dsec < breakpoint[ii].min_sec)	breakpoint[ii].min_sec = dsec;
			if (dsec > breakpoint[ii].max_sec)	breakpoint[ii].max_sec = dsec;
			breakpoint[ii].tot_sec += dsec;
			breakpoint[ii].tot_secsq += dsec * dsec;
			breakpoint[ii].tot_cnt++;
		}

		// get frame time
		ULONG	dtime = breakpoint[next_break].time - breakpoint[0].time;
		float	dsec = float(dtime) / float(time_base);

		if (dsec < breakpoint[0].min_sec)	breakpoint[0].min_sec = dsec;
		if (dsec > breakpoint[0].max_sec)	breakpoint[0].max_sec = dsec;
		breakpoint[0].tot_sec += dsec;
		breakpoint[0].tot_secsq += dsec * dsec;
		breakpoint[0].tot_cnt++;

		// restart
		breakpoint[0].time = GetFineTimerValue();
		next_break = 1;
	}
}

void BreakEnd(const char* filename)
{
	if (!enabled)	return;

	FILE*	fd = MF_Fopen(filename, "w");

	if (fd)
	{
		float	fps = float(breakpoint[0].tot_cnt) / breakpoint[0].tot_sec;
		float	facets = float(total_dfacets) / float(breakpoint[0].tot_cnt);

		fprintf(fd, "%d frames in %d seconds = %d fps\n\n", breakpoint[0].tot_cnt, int(floor(breakpoint[0].tot_sec + 0.5)), int(floor(fps + 0.5)));
		fprintf(fd, "Average %d facets\n\n", int(floor(facets + 0.5)));

		float	eX = breakpoint[0].tot_sec / breakpoint[0].tot_cnt;
		float	eXsq = breakpoint[0].tot_secsq / breakpoint[0].tot_cnt;
		float	variance = eXsq - eX*eX;	// var = E[X^2] - E[X]^2
		float	stddev = float(sqrt(variance));

		ULONG	min = ULONG(floor(breakpoint[0].min_sec * 1000000 + 0.5));
		ULONG	max = ULONG(floor(breakpoint[0].max_sec * 1000000 + 0.5));
		ULONG	av  = ULONG(floor(eX * 1000000 + 0.5));
		ULONG	sd  = ULONG(floor(stddev * 1000000 + 0.5));

		if (0)
		{
			fprintf(fd, "Whole frame:\n");
			fprintf(fd, "  Range: %d - %d us\n  Avg: %d us, Dev: %d us\n\n", min, max, av, sd);
		}

		for (size_t ii = 1; ii <= max_break; ii++)
		{
			float	eX = breakpoint[ii].tot_sec / breakpoint[ii].tot_cnt;
			float	eXsq = breakpoint[ii].tot_secsq / breakpoint[ii].tot_cnt;
			float	variance = eXsq - eX*eX;	// var = E[X^2] - E[X]^2
			float	stddev = float(sqrt(variance));

			ULONG	min = ULONG(floor(breakpoint[ii].min_sec * 1000000 + 0.5));
			ULONG	max = ULONG(floor(breakpoint[ii].max_sec * 1000000 + 0.5));
			ULONG	av  = ULONG(floor(eX * 1000000 + 0.5));
			ULONG	sd  = ULONG(floor(stddev * 1000000 + 0.5));
//			ULONG	apc = ULONG(floor(breakpoint[ii].tot_sec * 100 / breakpoint[0].tot_sec + 0.5));
			ULONG	apc = ULONG(floor(eX * 1000000 / 333 + 0.5));

			if (0)
			{
				fprintf(fd, "\"%s\" to \"%s\":\n", breakpoint[ii-1].name, breakpoint[ii].name);
				fprintf(fd, "  Range: %d - %d us\n  Avg: %d us (%d%%), Dev: %d us\n\n", min, max, av, apc, sd);
			}
			else
			{
				fprintf(fd, "%s", breakpoint[ii].name);
				for (size_t jj = strlen(breakpoint[ii].name); jj < 32; jj++)
				{
					fprintf(fd, " ");
				}
				fprintf(fd, "%d tms  ", apc);
				if (apc < 10)	fprintf(fd, " ");
				for (jj = 0; jj < apc; jj++)
				{
					fprintf(fd, "X");
				}
				fprintf(fd, "\n");
			}
		}


		MF_Fclose(fd);
	}

	enabled = false;
}

#endif	// BREAKTIMER

