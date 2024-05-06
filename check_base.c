#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "types/fbase.h"

struct args_state {
    size_t modes;
    size_t points;
    double threshold;
    double (*base)(size_t,double);
    unsigned int modes_set:1;
    unsigned int points_set:1;
    unsigned int threshold_set:1;
    unsigned int help_set:1;
};

static int parse_base(const char *arg, struct args_state *state) {
    if (state->base != NULL) {
        dprintf(2, "Base is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (base)\n");
        return -1;
    }
    if (strcmp(arg, "fourier") == 0) {
        state->base = fourier;
    } else if (strcmp(arg, "heaviside") == 0) {
        state->base = heaviside;
    } else if (strcmp(arg, "legendre") == 0) {
        state->base = legendre;
    } else {
        dprintf(2, "Provided base is not supported (try \"fourier\", \"legendre\" or \"heaviside\")\n");
        return -1;
    }
    return 0;
}

static int parse_modes(const char *arg, struct args_state *state) {
    if (state->modes_set) {
        dprintf(2, "Number of modes is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (modes)\n");
        return -1;
    }
    char *end = NULL;
    state->modes = strtoull(arg, &end, 0);
    if (*end != '\0') {
        dprintf(2, "Cannot parse number of modes\n");
        return -1;
    }
    state->modes_set = 1;
    return 0;
}

static int parse_points(const char *arg, struct args_state *state) {
    if (state->points_set) {
        dprintf(2, "Number of points is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (points)\n");
        return -1;
    }
    char *end = NULL;
    state->points = strtoull(arg, &end, 0);
    if (*end != '\0') {
        dprintf(2, "Cannot parse number of points\n");
        return -1;
    }
    state->points_set = 1;
    return 0;
}

static int parse_threshold(const char *arg, struct args_state *state) {
    if (state->threshold_set) {
        dprintf(2, "Threshold is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (threshold)\n");
        return -1;
    }
    char *end = NULL;
    state->threshold = strtod(arg, &end);
    if (*end != '\0') {
        dprintf(2, "Cannot parse threshold\n");
        return -1;
    }
    state->threshold_set = 1;
    return 0;
}

static int parse_help(const char *arg, struct args_state *state) {
    if (state->help_set) {
        dprintf(2, "Help is already set\n");
        return -1;
    }
    if (arg != NULL) {
        dprintf(2, "Unexpected parameter (help)\n");
        return -1;
    }
    state->help_set = 1;
    return 0;
}

struct option {
    const char *arg_name;
    const char *parameter_name;
    const char *description;
    const char *deflt;
    int (*parse)(const char *arg, struct args_state *state);
};

static struct option options[] = {
    {
        .arg_name = "base",
        .parameter_name = "orth_fun_base",
        .description = "<orth_fun_base> must be either \"fourier\", \"legendre\" or \"heaviside\"",
        .parse = parse_base,
        .deflt = "fourier",
    },
    {
        .arg_name = "points",
        .parameter_name = "points",
        .description = "<points> is the number of points to integrate (warning: computation time is linear on it)",
        .parse = parse_points,
        .deflt = "100",
    },
    {
        .arg_name = "modes",
        .parameter_name = "modes",
        .description = "<modes> is the number of modes to test (warning: computation time is quadratic on it)",
        .parse = parse_modes,
        .deflt = "4",
    },
    {
        .arg_name = "threshold",
        .parameter_name = "th",
        .description = "<th> is the threshold to detect 0.0 or 1.0 values",
        .parse = parse_threshold,
        .deflt = "0.01",
    },
    {
        .arg_name = "help",
        .parameter_name = NULL,
        .description = "Prints this help",
        .parse = parse_help,
        .deflt = NULL,
    },
    { 0 }
};


static void show_help(const char *cmd) {
    dprintf(2, "%s ", cmd);
    struct option *opt;
    opt = options;
    while (opt->arg_name != NULL) {
        _Bool mandatory = (opt->parameter_name != NULL) && (opt->deflt == NULL);
        if (!mandatory) {
            dprintf(2, "[");
        }
        dprintf(2, "--%s", opt->arg_name);
        if (opt->parameter_name != NULL) {
            dprintf(2, " <%s>", opt->parameter_name);
        }
        if (!mandatory) {
            dprintf(2, "]");
        }
        dprintf(2, " ");
        ++opt;
    }
    dprintf(2, "\n");
    opt = options;
    while (opt->arg_name != NULL) {
        dprintf(2, "  --%s", opt->arg_name);
        if (opt->parameter_name != NULL) {
            dprintf(2, " <%s>", opt->parameter_name);
        }
        dprintf(2, ": %s", opt->description);
        if (opt->deflt != NULL) {
            dprintf(2, " (deflt is \"%s\")", opt->deflt);
        }
        dprintf(2, "\n");
        ++opt;
    }
    return;
}

static int parse_args(struct args_state *args, int argc, char **argv) {
    int i = 1;
    while (i < argc) {
        if ((argv[i][0] == '\0') || (argv[i][1] == '\0')) {
            dprintf(2, "%s: invalid argument\n", argv[i]);
            return -1;
        }
        if ((argv[i][0] != '-') || (argv[i][1] != '-')) {
            dprintf(2, "%s: invalid argument\n", argv[i]);
            return -1;
        }
        struct option *opt = options;
        while (opt->arg_name != NULL) {
            if (strcmp(argv[i] + 2, opt->arg_name) == 0) {
                break;
            }
            ++opt;
        }
        if (opt->arg_name == NULL) {
            dprintf(2, "%s: invalid argument\n", argv[i]);
            return -1;
        }
        const char *param = NULL;
        if (opt->parameter_name != NULL) {
            ++i;
            if (i >= argc) {
                dprintf(2, "%s: missing parameter\n", argv[i-1]);
                return -1;
            }
            param = argv[i];
        }
        int r = opt->parse(param, args);
        if (r != 0) {
            return -1;
        }
        ++i;
    }
    return 0;
}

static int set_deflts(struct args_state *args) {
    if (args->base == NULL) {
        args->base = fourier;
    }
    if (args->modes_set == 0) {
        args->modes = 4;
        args->modes_set = 1;
    }
    if (args->points_set == 0) {
        args->points = 100;
        args->points_set = 1;
    }
    if (args->threshold_set == 0) {
        args->threshold = 0.01;
        args->threshold_set = 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    struct args_state args = { 0 };
    int r;
    r = parse_args(&args, argc, argv);
    if (r != 0) {
        args.help_set = 1;
    }
    r = set_deflts(&args);
    if (r != 0) {
        args.help_set = 1;
    }

    if (args.help_set) {
        show_help(argv[0]);
        return -1;
    }
    if (args.points == 0) {
        dprintf(2, "Expecting at least 1 point\n");
        return -1;
    }

    double dt = 1.0 / ((double)args.points);
    unsigned int failures = 0;

    for (size_t i = 0; i < args.modes; ++i) {
        for (size_t j = i; j < args.modes; ++j) {
            double integrate = 0.0;
            double t = 0.0;
            for (size_t k = 0; k < args.points; ++k) {
                integrate += args.base(i, t) * args.base(j, t);
                t += dt;
            }
            integrate *= dt;
            double centered = integrate;
            if (i == j) {
                centered -= 1.0;
            }
            if ((centered + args.threshold < 0.0) || (centered - args.threshold > 0.0)) {
                ++failures;
                dprintf(2, "<%zu|%zu> returned %g\n", i, j, integrate);
            }
        }
    }
    dprintf(2, "%u failures reported\n", failures);
    return (failures == 0) ? 0 : -1;
}
