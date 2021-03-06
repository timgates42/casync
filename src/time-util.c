/* SPDX-License-Identifier: LGPL-2.1+ */

#include "util.h"
#include "time-util.h"

char *format_timespan(char *buf, size_t l, uint64_t t, uint64_t accuracy) {
        static const struct {
                const char *suffix;
                uint64_t nsec;
        } table[] = {
                { "y",     NSEC_PER_YEAR   },
                { "month", NSEC_PER_MONTH  },
                { "w",     NSEC_PER_WEEK   },
                { "d",     NSEC_PER_DAY    },
                { "h",     NSEC_PER_HOUR   },
                { "min",   NSEC_PER_MINUTE },
                { "s",     NSEC_PER_SEC    },
                { "ms",    NSEC_PER_MSEC   },
                { "us",    NSEC_PER_USEC   },
                { "ns",    1               },
        };

        size_t i;
        char *p = buf;
        bool something = false;

        assert(buf);
        assert(l > 0);

        if (t == NSEC_INFINITY) {
                strncpy(p, "infinity", l-1);
                p[l-1] = 0;
                return p;
        }

        if (t <= 0) {
                strncpy(p, "0", l-1);
                p[l-1] = 0;
                return p;
        }

        /* The result of this function can be parsed with parse_sec */

        for (i = 0; i < ELEMENTSOF(table); i++) {
                int k = 0;
                size_t n;
                bool done = false;
                uint64_t a, b;

                if (t <= 0)
                        break;

                if (t < accuracy && something)
                        break;

                if (t < table[i].nsec)
                        continue;

                if (l <= 1)
                        break;

                a = t / table[i].nsec;
                b = t % table[i].nsec;

                /* Let's see if we should shows this in dot notation */
                if (t < NSEC_PER_MINUTE && b > 0) {
                        uint64_t cc;
                        signed char j;

                        j = 0;
                        for (cc = table[i].nsec; cc > 1; cc /= 10)
                                j++;

                        for (cc = accuracy; cc > 1; cc /= 10) {
                                b /= 10;
                                j--;
                        }

                        if (j > 0) {
                                k = snprintf(p, l,
                                             "%s%" PRIu64 ".%0*" PRIu64 "%s",
                                             p > buf ? " " : "",
                                             a,
                                             j,
                                             b,
                                             table[i].suffix);

                                t = 0;
                                done = true;
                        }
                }

                /* No? Then let's show it normally */
                if (!done) {
                        k = snprintf(p, l,
                                     "%s%" PRIu64 "%s",
                                     p > buf ? " " : "",
                                     a,
                                     table[i].suffix);

                        t = b;
                }

                n = MIN((size_t) k, l);

                l -= n;
                p += n;

                something = true;
        }

        *p = 0;

        return buf;
}
